#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include <iostream>
#include "trans.h"
#include "util.h"
namespace {

const std::string authorization =
    "Basic YW5kcm9pZHVzZXI6MWEjJDUxLXl0Njk7KkFjdkBxeHE=";
// TODO Updated version
const std::string user_agent = "boluobao/4.3.20(android;22)/HomePage";
const std::string device_token = "AAC3B586-D131-32DE-942C-F5CCED55B45E";

std::string sf_security() {
  std::string uuid = boost::to_upper_copy(klib::uuid());
  auto timestamp = std::time(nullptr);
  std::string sign = boost::to_upper_copy(
      klib::md5(uuid + std::to_string(timestamp) + device_token + "xw3#a12-x"));

  return fmt::format(
      FMT_COMPILE("nonce={}&timestamp={}&devicetoken={}&sign={}"), uuid,
      timestamp, device_token, sign);
}

auto parse_json(const std::string &json) {
  static boost::json::error_code error_code;
  static boost::json::monotonic_resource mr;
  auto jv = boost::json::parse(json, error_code, &mr);

  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  return jv;
}

klib::Response http_get(const std::string &url,
                        const std::map<std::string, std::string> &params) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(
      url, params,
      {{"Authorization", authorization}, {"SFSecurity", sf_security()}});

  if (auto code = response.status_code();
      code != klib::Response::StatusCode::Ok) {
    auto status = parse_json(response.text()).at("status");
    klib::error("HTTP GET fail, httpCode: {}, errorCode: {}, msg: {}", code,
                status.at("errorCode").as_int64(),
                status.at("msg").as_string().c_str());
  }

  return response;
}

klib::Response http_post(const std::string &url, const std::string &json) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.post(url, json,
                               {{"Content-Type", "application/json"},
                                {"Authorization", authorization},
                                {"SFSecurity", sf_security()}});

  if (auto code = response.status_code();
      code != klib::Response::StatusCode::Ok) {
    auto status = parse_json(response.text()).at("status");
    klib::error("HTTP POST fail, httpCode: {}, errorCode: {}, msg: {}", code,
                status.at("errorCode").as_int64(),
                status.at("msg").as_string().c_str());
  }

  return response;
}

void login(const std::string &login_name, const std::string &password) {
  boost::json::object obj;
  obj["username"] = login_name;
  obj["password"] = password;

  auto response =
      http_post("https://api.sfacg.com/sessions", boost::json::serialize(obj));

  spdlog::info("登陆成功");
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &book_id) {
  auto response = http_get("https://api.sfacg.com/novels/" + book_id,
                           {{"expand", "intro"}});
  auto data = parse_json(response.text()).at("data");

  std::string book_name =
      kepub::trans_str(data.at("novelName").as_string().c_str());
  std::string author =
      kepub::trans_str(data.at("authorName").as_string().c_str());
  std::string description_str =
      data.at("expand").at("intro").as_string().c_str();
  std::string cover_url = data.at("novelCover").as_string().c_str();

  // TODO use klib
  std::vector<std::string> temp;
  boost::split(temp, description_str, boost::is_any_of("\n"),
               boost::token_compress_on);
  std::vector<std::string> description;
  for (const auto &line : temp) {
    kepub::push_back(description, kepub::trans_str(line), false);
  }

  spdlog::info("书名: {}", book_name);
  spdlog::info("作者: {}", author);
  spdlog::info("封面: {}", cover_url);

  return {book_name, author, description};
}

std::vector<
    std::pair<std::string, std::vector<std::pair<std::int64_t, std::string>>>>
get_volume_chapter(const std::string &book_id) {
  auto response = http_get(
      fmt::format(FMT_COMPILE("https://api.sfacg.com/novels/{}/dirs"), book_id),
      {});
  auto jv = parse_json(response.text());

  std::vector<
      std::pair<std::string, std::vector<std::pair<std::int64_t, std::string>>>>
      volume_chapter;

  auto volume_list = jv.at("data").at("volumeList").as_array();
  for (const auto &volume : volume_list) {
    std::string volume_name =
        kepub::trans_str(volume.at("title").as_string().c_str());

    std::vector<std::pair<std::int64_t, std::string>> chapters;
    auto chapter_list = volume.at("chapterList").as_array();
    for (const auto &chapter : chapter_list) {
      auto chapter_id = chapter.at("chapId").as_int64();
      auto chapter_title =
          kepub::trans_str(chapter.at("title").as_string().c_str());
      chapters.emplace_back(chapter_id, chapter_title);
    }

    spdlog::info("获取章节: {} ok", volume_name);
    volume_chapter.emplace_back(volume_name, chapters);
  }

  return volume_chapter;
}

std::vector<std::string> get_content(std::int64_t chapter_id) {
  auto response =
      http_get("https://api.sfacg.com/Chaps/" + std::to_string(chapter_id),
               {{"expand", "content"}});
  auto jv = parse_json(response.text());

  std::string content_str =
      jv.at("data").at("expand").at("content").as_string().c_str();

  // TODO use klib
  std::vector<std::string> temp;
  boost::split(temp, content_str, boost::is_any_of("\n"),
               boost::token_compress_on);
  std::vector<std::string> content;
  for (const auto &line : temp) {
    kepub::push_back(content, kepub::trans_str(line), false);
  }

  return content;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  auto login_name = kepub::get_login_name();
  auto password = kepub::get_password();

  login(login_name, password);
  auto [book_name, author, description] = get_book_info(book_id);
  auto volume_chapter = get_volume_chapter(book_id);

  auto p = std::make_unique<klib::ChangeWorkingDir>("temp");
  for (const auto &[volume_name, chapters] : volume_chapter) {
    klib::ChangeWorkingDir change_working_dir(volume_name);

    for (const auto &[chapter_id, chapter_title] : chapters) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100ms);

      auto pid = fork();
      if (pid < 0) {
        klib::error("Fork error");
      } else if (pid == 0) {
        klib::write_file(chapter_title + ".txt", false,
                         boost::join(get_content(chapter_id), "\n"));
        spdlog::info("{} ok", chapter_title);
        std::exit(EXIT_SUCCESS);
      }
    }
  }

  klib::wait_for_child_process();
  p.reset();

  std::ofstream book_ofs(book_name + ".txt");
  book_ofs << author << "\n\n";
  for (const auto &line : description) {
    book_ofs << line << "\n";
  }
  book_ofs << "\n";

  p = std::make_unique<klib::ChangeWorkingDir>("temp");
  for (const auto &[volume_name, chapters] : volume_chapter) {
    klib::ChangeWorkingDir change_working_dir(volume_name);
    book_ofs << "[VOLUME] " << volume_name << "\n\n";

    for (const auto &[chapter_id, chapter_title] : chapters) {
      book_ofs << "[WEB] " << chapter_title << "\n\n";
      book_ofs << klib::read_file(chapter_title + ".txt", false) << "\n\n";
    }
  }

  p.reset();
  std::filesystem::remove_all("temp");
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
