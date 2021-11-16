#include "json.h"

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>
#include <klib/util.h>

#include "trans.h"
#include "util.h"

std::string serialize(const std::string &login_name,
                      const std::string &password) {
  return fmt::format(FMT_COMPILE(R"({{"username":"{}","password":"{}"}})"),
                     login_name, password);
}

JsonBase::JsonBase(std::string json) : json_(std::move(json)) {
  json_.reserve(std::size(json_) + simdjson::SIMDJSON_PADDING);
  doc_ = parser_.iterate(json_);

  auto status = doc_["status"];
  http_code_ =
      static_cast<std::int32_t>(status["httpCode"].get_int64().value());
  error_code_ =
      static_cast<std::int32_t>(status["errorCode"].get_int64().value());

  if (login_expired()) {
    klib::warn(status["msg"].get_string().value());
  } else if (!ok()) {
    klib::error(status["msg"].get_string().value());
  }
}

UserInfo::UserInfo(std::string json) : JsonBase(std::move(json)) {
  if (!login_expired()) {
    nick_name_ = doc_["data"]["nickName"].get_string().value();
  }
}

LoginInfo::LoginInfo(std::string json) : JsonBase(std::move(json)) {
  nick_name_ = doc_["data"]["nickName"].get_string().value();
}

BookInfo::BookInfo(std::string json) : JsonBase(std::move(json)) {
  auto data = doc_["data"];

  book_name_ = kepub::trans_str(data["novelName"].get_string().value(), false);
  author_ = kepub::trans_str(data["authorName"].get_string().value(), false);
  cover_url_ = data["novelCover"].get_string().value();

  std::string intro_str(data["expand"]["intro"].get_string().value());

  for (const auto &line : klib::split_str(intro_str, "\n")) {
    kepub::push_back(intro_, kepub::trans_str(line, false), false);
  }
}

VolumeChapter::VolumeChapter(std::string json) : JsonBase(std::move(json)) {
  for (auto volume : doc_["data"]["volumeList"].get_array()) {
    auto volume_name =
        kepub::trans_str(volume["title"].get_string().value(), false);

    std::vector<std::tuple<std::string, std::string, std::string>> chapters;
    for (auto chapter : volume["chapterList"].get_array()) {
      auto chapter_id = std::to_string(chapter["chapId"].get_int64());
      auto chapter_title =
          kepub::trans_str(chapter["title"].get_string().value(), false);

      auto need_fire_money = chapter["needFireMoney"].get_int64().value();
      if (need_fire_money > 0) {
        klib::warn("No authorized access, volume: {}, title: {}", volume_name,
                   chapter_title);
      } else {
        chapters.emplace_back(chapter_id, chapter_title, "");
      }
    }
    volume_chapter_.emplace_back(volume_name, chapters);
  }
}

Content::Content(std::string json) : JsonBase(std::move(json)) {
  content_ = doc_["data"]["expand"]["content"].get_string().value();
}
