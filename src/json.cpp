#include "json.h"

#include <charconv>
#include <cstdint>

#include <klib/log.h>
#include <klib/unicode.h>
#include <klib/util.h>
#include <simdjson.h>
#include <boost/json.hpp>

#include "util.h"

namespace kepub {

namespace masiro {

enum Code { Ok = 1, Error = -1 };

void json_base(std::string json) {
  simdjson::ondemand::parser parser;
  json.reserve(std::size(json) + simdjson::SIMDJSON_PADDING);

  auto doc = parser.iterate(json);
  std::int32_t code = doc["code"].get_int64();
  if (code == Error) {
    klib::error(doc["msg"].get_string().value());
  }
}

}  // namespace masiro

namespace ciweimao {

enum Code { Ok = 100000, LoginExpired = 200100 };

// FIXME
// auto code = doc["code"].get_int64().value();
#define JSON_BASE_CIWEIMAO(json)                                        \
  simdjson::ondemand::parser parser;                                    \
  (json).reserve(std::size(json) + simdjson::SIMDJSON_PADDING);         \
  auto doc = parser.iterate(json);                                      \
  auto code = std::stoi(std::string(doc["code"].get_string().value())); \
  if (code == LoginExpired) {                                           \
    klib::warn(doc["tip"].get_string().value());                        \
  } else if (code != Ok) {                                              \
    klib::error(doc["tip"].get_string().value());                       \
  }

std::string serialize(const std::string &account,
                      const std::string &login_token) {
  boost::json::object obj;
  obj["account"] = account;
  obj["login_token"] = login_token;

  return boost::json::serialize(obj);
}

Token get_token(std::string json) {
  simdjson::ondemand::parser parser;
  json.reserve(std::size(json) + simdjson::SIMDJSON_PADDING);
  auto doc = parser.iterate(json);

  Token result;

  result.account_ = doc["account"].get_string().value();
  result.login_token_ = doc["login_token"].get_string().value();

  return result;
}

UserInfo json_to_user_info(std::string json) {
  JSON_BASE_CIWEIMAO(json)

  UserInfo result;
  result.login_expired_ = (code == LoginExpired);

  if (!result.login_expired_) {
    result.nick_name_ =
        doc["data"]["reader_info"]["reader_name"].get_string().value();
  }

  return result;
}

LoginInfo json_to_login_info(std::string json) {
  JSON_BASE_CIWEIMAO(json)
  if (code == LoginExpired) {
    klib::error("Failed to login");
  }

  LoginInfo result;
  auto data = doc["data"];
  result.token_.login_token_ = data["login_token"].get_string().value();

  auto reader_info = data["reader_info"];
  result.token_.account_ = reader_info["account"].get_string().value();

  UserInfo user_info;
  user_info.nick_name_ =
      std::string(reader_info["reader_name"].get_string().value());
  result.user_info_ = user_info;

  return result;
}

kepub::BookInfo json_to_book_info(std::string json) {
  kepub::BookInfo result;

  JSON_BASE_CIWEIMAO(json)
  auto book_info = doc["data"]["book_info"];

  result.name_ = book_info["book_name"].get_string().value();
  klib::trim(result.name_);

  result.author_ = book_info["author_name"].get_string().value();
  klib::trim(result.author_);

  result.cover_path_ = book_info["cover"].get_string().value();

  std::string intro_str(book_info["description"].get_string().value());

  for (const auto &line : klib::split_str(intro_str, "\n")) {
    kepub::push_back(result.introduction_, line);
  }

  return result;
}

std::vector<kepub::Volume> get_volume_info(std::string json) {
  std::vector<kepub::Volume> result;

  JSON_BASE_CIWEIMAO(json)
  for (auto volume : doc["data"]["division_list"].get_array()) {
    std::string volume_id(volume["division_id"].get_string().value());
    std::string volume_name(volume["division_name"].get_string().value());
    klib::trim(volume_name);

    std::uint64_t id;
    auto ptr = std::data(volume_id);
    std::from_chars(ptr, ptr + std::size(volume_id), id);

    result.emplace_back(id, volume_name);
  }

  return result;
}

std::vector<kepub::Chapter> get_chapter_info(std::string json) {
  std::vector<kepub::Chapter> result;

  JSON_BASE_CIWEIMAO(json)
  for (auto chapter : doc["data"]["chapter_list"].get_array()) {
    std::string chapter_id(chapter["chapter_id"].get_string().value());
    std::string chapter_title(chapter["chapter_title"].get_string().value());
    klib::trim(chapter_title);

    // FIXME
    // auto is_valid = chapter["is_valid"].get_int64().value();
    std::string is_valid(chapter["is_valid"].get_string().value());
    if (is_valid != "1") {
      klib::warn("The chapter is not valid, title: {}", chapter_title);
      continue;
    }

    // FIXME
    // auto auth_access = chapter["auth_access"].get_int64().value();
    std::string auth_access(chapter["auth_access"].get_string().value());
    if (auth_access != "1") {
      klib::warn("No authorized access, title: {}", chapter_title);
    } else {
      std::uint64_t id;
      auto ptr = std::data(chapter_id);
      std::from_chars(ptr, ptr + std::size(chapter_id), id);

      result.emplace_back(id, chapter_title);
    }
  }

  return result;
}

std::string get_chapter_command(std::string json) {
  JSON_BASE_CIWEIMAO(json)
  std::string result(doc["data"]["command"].get_string().value());

  return result;
}

std::string json_to_chapter_text(std::string json) {
  JSON_BASE_CIWEIMAO(json)
  std::string result(
      doc["data"]["chapter_info"]["txt_content"].get_string().value());

  return result;
}

#undef JSON_BASE_CIWEIMAO

}  // namespace ciweimao

namespace sfacg {

#define JSON_BASE_SFACG(json)                                             \
  simdjson::ondemand::parser parser;                                      \
  (json).reserve(std::size(json) + simdjson::SIMDJSON_PADDING);           \
  auto doc = parser.iterate(json);                                        \
  auto status = doc["status"];                                            \
  auto http_code =                                                        \
      static_cast<std::int32_t>(status["httpCode"].get_int64().value());  \
  auto error_code =                                                       \
      static_cast<std::int32_t>(status["errorCode"].get_int64().value()); \
  if (http_code == 401 && error_code == 502) {                            \
    klib::warn(status["msg"].get_string().value());                       \
  } else if (!(http_code == 200 && error_code == 200)) {                  \
    klib::error(status["msg"].get_string().value());                      \
  }

std::string serialize(const std::string &login_name,
                      const std::string &password) {
  boost::json::object obj;
  obj["userName"] = login_name;
  obj["passWord"] = password;

  return boost::json::serialize(obj);
}

void json_base(std::string json){JSON_BASE_SFACG(json)}

UserInfo json_to_user_info(std::string json) {
  JSON_BASE_SFACG(json)

  UserInfo result;
  result.login_expired_ = (http_code == 401 && error_code == 502);

  if (!result.login_expired_) {
    result.nick_name_ = doc["data"]["nickName"].get_string().value();
  }

  return result;
}

LoginInfo json_to_login_info(std::string json) {
  JSON_BASE_SFACG(json)
  if (http_code == 401 && error_code == 502) {
    klib::error("Failed to login");
  }

  LoginInfo result;
  UserInfo user_info;
  user_info.nick_name_ =
      std::string(doc["data"]["nickName"].get_string().value());
  result.user_info_ = user_info;

  return result;
}

kepub::BookInfo json_to_book_info(std::string json) {
  kepub::BookInfo result;

  JSON_BASE_SFACG(json)
  auto data = doc["data"];

  result.name_ = data["novelName"].get_string().value();
  klib::trim(result.name_);

  result.author_ = data["authorName"].get_string().value();
  klib::trim(result.author_);

  result.cover_path_ = data["novelCover"].get_string().value();
  result.point_ = data["point"].get_double().value();

  std::string intro_str(data["expand"]["intro"].get_string().value());

  for (const auto &line : klib::split_str(intro_str, "\n")) {
    kepub::push_back(result.introduction_, line);
  }

  return result;
}

std::vector<kepub::Volume> json_to_volumes(std::string json) {
  std::vector<kepub::Volume> result;
  JSON_BASE_SFACG(json)

  for (auto volume : doc["data"]["volumeList"].get_array()) {
    std::uint64_t volume_id = volume["volumeId"].get_int64();
    std::string volume_name(volume["title"].get_string().value());
    klib::trim(volume_name);

    std::vector<kepub::Chapter> chapters;
    for (auto chapter : volume["chapterList"].get_array()) {
      std::uint64_t chapter_id = chapter["chapId"].get_int64();
      std::string chapter_title(chapter["title"].get_string().value());
      klib::trim(chapter_title);

      auto need_fire_money = chapter["needFireMoney"].get_int64().value();
      if (need_fire_money > 0) {
        klib::warn("No authorized access, title: {}", chapter_title);
      } else {
        chapters.emplace_back(chapter_id, chapter_title);
      }
    }
    result.emplace_back(volume_id, volume_name, chapters);
  }

  return result;
}

std::string json_to_chapter_text(std::string json) {
  JSON_BASE_SFACG(json)
  std::string result(doc["data"]["expand"]["content"].get_string().value());

  return result;
}

#undef JSON_BASE_SFACG

}  // namespace sfacg

}  // namespace kepub
