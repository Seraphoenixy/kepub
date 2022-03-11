#include "json.h"

#include <klib/log.h>
#include <klib/unicode.h>
#include <klib/util.h>
#include <simdjson.h>
#include <boost/json.hpp>

#include "util.h"

#define JSON_BASE(json)                                                   \
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

void json_base(std::string json){JSON_BASE(json)}

UserInfo json_to_user_info(std::string json) {
  JSON_BASE(json)

  UserInfo result;
  result.login_expired_ = (http_code == 401 && error_code == 502);

  if (!result.login_expired_) {
    result.nick_name_ = doc["data"]["nickName"].get_string().value();
  }

  return result;
}

LoginInfo json_to_login_info(std::string json) {
  JSON_BASE(json)
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

  JSON_BASE(json)
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
  JSON_BASE(json)

  for (auto volume : doc["data"]["volumeList"].get_array()) {
    auto volume_id = std::to_string(volume["volumeId"].get_int64());
    std::string volume_name(volume["title"].get_string().value());
    klib::trim(volume_name);

    std::vector<kepub::Chapter> chapters;
    for (auto chapter : volume["chapterList"].get_array()) {
      auto chapter_id = std::to_string(chapter["chapId"].get_int64());
      std::string chapter_title(chapter["title"].get_string().value());
      klib::trim(chapter_title);

      auto need_fire_money = chapter["needFireMoney"].get_int64().value();
      if (need_fire_money > 0) {
        klib::warn("No authorized access, title: {}", chapter_title);
      } else {
        chapters.push_back({chapter_id, chapter_title, {}});
      }
    }
    result.push_back({volume_id, volume_name, chapters});
  }

  return result;
}

std::string json_to_chapter_text(std::string json) {
  JSON_BASE(json)
  std::string result(doc["data"]["expand"]["content"].get_string().value());

  return result;
}
