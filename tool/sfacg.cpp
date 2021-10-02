#include <ctime>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>
#include <klib/html.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <pugixml.hpp>

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

klib::Response http_get(const std::string &url) {
  static klib::Request request;
  request.set_no_proxy();
  request.use_cookies(false);
  request.set_browser_user_agent();
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);

  if (auto code = response.status_code();
      code != klib::Response::StatusCode::Ok) {
    klib::error("HTTP GET fail, httpCode: {}", code);
  }

  return response;
}

klib::Response http_get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params,
    bool check = true) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(
      url, params,
      {{"Authorization", authorization}, {"SFSecurity", sf_security()}});

  if (check) {
    if (auto code = response.status_code();
        code != klib::Response::StatusCode::Ok) {
      auto status = parse_json(response.text()).at("status");
      klib::error("HTTP GET fail, httpCode: {}, errorCode: {}, msg: {}", code,
                  status.at("errorCode").as_int64(),
                  status.at("msg").as_string().c_str());
    }
  }

  return response;
}

klib::Response http_post(const std::string &url, const std::string &json) {
  klib::Request request;
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

bool show_user_info() {
  auto response = http_get("https://api.sfacg.com/user", {}, false);
  auto jv = parse_json(response.text());

  if (response.status_code() == klib::Response::StatusCode::Unauthorized) {
    klib::warn(jv.at("status").at("msg").as_string().c_str());
    return false;
  } else if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error(jv.at("status").at("msg").as_string().c_str());
  } else {
    std::string nick_name = jv.at("data").at("nickName").as_string().c_str();
    spdlog::info("Use existing cookies, nick name: {}", nick_name);

    return true;
  }
}

void login(const std::string &login_name, const std::string &password) {
  boost::json::object obj;
  obj["username"] = login_name;
  obj["password"] = password;
  http_post("https://api.sfacg.com/sessions", boost::json::serialize(obj));

  auto response = http_get("https://api.sfacg.com/user", {}, false);
  auto jv = parse_json(response.text());
  std::string nick_name = jv.at("data").at("nickName").as_string().c_str();
  spdlog::info("Login successful, nick name: {}", nick_name);
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

  std::vector<std::string> description;
  for (const auto &line : klib::split_str(description_str, "\n")) {
    kepub::push_back(description, kepub::trans_str(line), false);
  }

  spdlog::info("Book name: {}", book_name);
  spdlog::info("Author: {}", author);
  spdlog::info("Cover url: {}", cover_url);

  std::string cover_name = "cover.jpg";
  response = http_get(cover_url);
  response.save_to_file(cover_name, true);
  spdlog::info("Cover downloaded successfully: {}", cover_name);

  return {book_name, author, description};
}

std::vector<
    std::pair<std::string,
              std::vector<std::tuple<std::string, std::string, std::string>>>>
get_volume_chapter(const std::string &book_id) {
  auto response = http_get(
      fmt::format(FMT_COMPILE("https://api.sfacg.com/novels/{}/dirs"), book_id),
      {});
  auto jv = parse_json(response.text());

  std::vector<
      std::pair<std::string,
                std::vector<std::tuple<std::string, std::string, std::string>>>>
      volume_chapter;

  auto volume_list = jv.at("data").at("volumeList").as_array();
  for (const auto &volume : volume_list) {
    std::string volume_name =
        kepub::trans_str(volume.at("title").as_string().c_str());

    std::vector<std::tuple<std::string, std::string, std::string>> chapters;
    auto chapter_list = volume.at("chapterList").as_array();
    for (const auto &chapter : chapter_list) {
      std::string chapter_id = std::to_string(chapter.at("chapId").as_int64());
      auto chapter_title =
          kepub::trans_str(chapter.at("title").as_string().c_str());

      chapters.emplace_back(chapter_id, chapter_title, "");
    }
    volume_chapter.emplace_back(volume_name, chapters);

    spdlog::info("Successfully obtained sub-volume: {} ok", volume_name);
  }

  return volume_chapter;
}

std::vector<std::string> get_content_from_web(const std::string &chapter_id) {
  auto response = http_get(
      fmt::format(FMT_COMPILE("https://book.sfacg.com/vip/c/{}/"), chapter_id));

  auto xml = klib::html_tidy(response.text(), true);
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  auto node =
      doc.select_node(
             "/html/body/div[@class='container']/div[@class='article-box "
             "skin-white']/"
             "div[@class='article-outer "
             "width-middle']/div[@class='article-wrap']/"
             "div[@class='article']/div[@class='article-content font16']")
          .node();
  std::string content_str = kepub::trans_str(node.text().as_string());

  if (content_str.ends_with("...")) {
    boost::erase_tail(content_str, 3);
  }

  std::vector<std::string> content;
  auto separate = std::string(3, ' ');

  if (content_str.find(separate) == std::string::npos) {
    boost::replace_all(content_str, "。", "。" + separate);
    boost::replace_all(content_str, "”“", "”" + separate + "“");
  }

  for (const auto &line : klib::split_str(content_str, separate)) {
    kepub::push_back(content, line, false);
  }

  return content;
}

std::vector<std::string> get_content(const std::string &chapter_id,
                                     const std::string &chapter_title,
                                     bool download_without_authorization) {
  auto response = http_get("https://api.sfacg.com/Chaps/" + chapter_id,
                           {{"expand", "content"}}, false);

  if (auto code = response.status_code();
      code == klib::Response::StatusCode::Ok) {
    auto jv = parse_json(response.text());
    auto content_str =
        jv.at("data").at("expand").at("content").as_string().c_str();

    std::vector<std::string> content;
    for (const auto &line : klib::split_str(content_str, "\n")) {
      kepub::push_back(content, kepub::trans_str(line), false);
    }

    for (auto &line : content) {
      if (line.starts_with("[img")) {
        auto begin = line.find("https");
        if (begin == std::string::npos) {
          klib::error("no image url");
        }

        auto end = line.find("[/img]");
        if (end == std::string::npos) {
          klib::error("no image url");
        }

        auto image_url = line.substr(begin, end - begin);
        boost::replace_all(image_url, "：", ":");

        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_int_distribution uid;

        auto image = http_get(image_url);
        auto image_name = std::to_string(uid(gen)) + ".jpg";
        image.save_to_file(image_name, true);

        line = "TODO [IMAGE] " + image_name;
      }
    }

    return content;
  } else if (code == klib::Response::Forbidden) {
    if (download_without_authorization) {
      return get_content_from_web(chapter_id);
    } else {
      klib::warn("No authorized access, id: {}, title: {}", chapter_id,
                 chapter_title);
      return {};
    }
  } else {
    auto status = parse_json(response.text()).at("status");
    klib::error(
        "When get chapter {}(id: {}), HTTP GET fail, httpCode: {}, errorCode: "
        "{}, msg: {}",
        chapter_title, chapter_id, code, status.at("errorCode").as_int64(),
        status.at("msg").as_string().c_str());
  }
}

}  // namespace

int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  if (!show_user_info()) {
    auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    login(login_name, password);
  }

  auto [book_name, author, description] = get_book_info(book_id);
  auto volume_chapter = get_volume_chapter(book_id);

  for (auto &[volume_name, chapters] : volume_chapter) {
    for (auto &[chapter_id, chapter_title, content] : chapters) {
      content =
          boost::join(get_content(chapter_id, chapter_title,
                                  options.download_without_authorization_),
                      "\n");

      if (!std::empty(content)) {
        spdlog::info("Successfully obtained chapter: {}", chapter_title);
      } else {
        if (options.download_without_authorization_) {
          klib::error("No content");
        }
      }
    }
  }

  kepub::generate_txt(book_name, author, description, volume_chapter);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
