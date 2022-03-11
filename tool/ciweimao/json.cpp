#include "json.h"

#include <klib/log.h>
#include <klib/util.h>
#include <simdjson.h>
#include <boost/json.hpp>

#include "util.h"

enum Code { Ok = 100000, LoginExpired = 200100 };

#define JSON_BASE(json)                                         \
  simdjson::ondemand::parser parser;                            \
  (json).reserve(std::size(json) + simdjson::SIMDJSON_PADDING); \
  auto doc = parser.iterate(json);                              \
  auto code = doc["code"].get_int64().value();                  \
  if (code == LoginExpired) {                                   \
    klib::warn(doc["tip"].get_string().value());                \
  } else if (code != Ok) {                                      \
    klib::error(doc["tip"].get_string().value());               \
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
  JSON_BASE(json)

  UserInfo result;
  result.login_expired_ = (code == LoginExpired);

  if (!result.login_expired_) {
    result.nick_name_ =
        doc["data"]["reader_info"]["reader_name"].get_string().value();
  }

  return result;
}

LoginInfo json_to_login_info(std::string json) {
  JSON_BASE(json)
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

  JSON_BASE(json)
  auto book_info = doc["data"]["book_info"];

  result.name_ = book_info["book_name"].get_string().value();
  result.author_ = book_info["author_name"].get_string().value();
  result.cover_path_ = book_info["cover"].get_string().value();

  std::string intro_str(book_info["description"].get_string().value());

  for (const auto &line : klib::split_str(intro_str, "\n")) {
    kepub::push_back(result.introduction_, line);
  }

  return result;
}

std::vector<kepub::Volume> get_volume_info(std::string json) {
  std::vector<kepub::Volume> result;

  JSON_BASE(json)
  for (auto volume : doc["data"]["division_list"].get_array()) {
    std::string volume_id(volume["division_id"].get_string().value());
    std::string volume_name(volume["division_name"].get_string().value());

    result.push_back({volume_id, volume_name, {}});
  }

  return result;
}

std::vector<kepub::Chapter> get_chapter_info(std::string json) {
  std::vector<kepub::Chapter> result;

  JSON_BASE(json)
  for (auto chapter : doc["data"]["chapter_list"].get_array()) {
    std::string chapter_id(chapter["chapter_id"].get_string().value());
    std::string chapter_title(chapter["chapter_title"].get_string().value());

    auto is_valid = chapter["is_valid"].get_int64().value();
    if (is_valid != 1) {
      klib::warn("The chapter is not valid, title: {}", chapter_title);
      continue;
    }

    auto auth_access = chapter["auth_access"].get_int64().value();
    if (auth_access != 1) {
      klib::warn("No authorized access, title: {}", chapter_title);
    } else {
      result.push_back({chapter_id, chapter_title, {}});
    }
  }

  return result;
}

std::string get_chapter_command(std::string json) {
  JSON_BASE(json)
  std::string result(doc["data"]["command"].get_string().value());

  return result;
}

std::string json_to_chapter_text(std::string json) {
  JSON_BASE(json)
  std::string result(
      doc["data"]["chapter_info"]["txt_content"].get_string().value());

  return result;
}
