#include "json.h"

#include <utility>

#include <klib/log.h>
#include <klib/util.h>
#include <boost/json.hpp>

#include "util.h"

std::string serialize(const std::string &account,
                      const std::string &login_token) {
  boost::json::object obj;
  obj["account"] = account;
  obj["login_token"] = login_token;

  return boost::json::serialize(obj);
}

Token::Token(std::string json) {
  json.reserve(std::size(json) + simdjson::SIMDJSON_PADDING);
  simdjson::ondemand::parser parser;
  auto doc = parser.iterate(json);

  account_ = doc["account"].get_string().value();
  login_token_ = doc["login_token"].get_string().value();
}

JsonBase::JsonBase(std::string json) : json_(std::move(json)) {
  json_.reserve(std::size(json_) + simdjson::SIMDJSON_PADDING);
  doc_ = parser_.iterate(json_);

  code_ = doc_["code"].get_int64().value();

  if (login_expired()) {
    klib::warn(doc_["tip"].get_string().value());
  } else if (!ok()) {
    klib::error(doc_["tip"].get_string().value());
  }
}

UserInfo::UserInfo(std::string json) : JsonBase(std::move(json)) {
  if (!login_expired()) {
    nick_name_ =
        doc_["data"]["reader_info"]["reader_name"].get_string().value();
  }
}

LoginInfo::LoginInfo(std::string json) : JsonBase(std::move(json)) {
  if (login_expired()) {
    klib::error("Failed to login");
  }

  auto data = doc_["data"];
  login_token_ = data["login_token"].get_string().value();

  auto reader_info = data["reader_info"];
  account_ = reader_info["account"].get_string().value();
  nick_name_ = reader_info["reader_name"].get_string().value();
}

BookInfo::BookInfo(std::string json) : JsonBase(std::move(json)) {
  auto book_info = doc_["data"]["book_info"];

  book_name_ = book_info["book_name"].get_string().value();
  author_ = book_info["author_name"].get_string().value();
  cover_url_ = book_info["cover"].get_string().value();

  std::string intro_str(book_info["description"].get_string().value());

  for (const auto &line : klib::split_str(intro_str, "\n")) {
    kepub::push_back(intro_, line);
  }
}

Volumes::Volumes(std::string json) : JsonBase(std::move(json)) {
  for (auto volume : doc_["data"]["division_list"].get_array()) {
    std::string volume_id(volume["division_id"].get_string().value());
    std::string volume_name(volume["division_name"].get_string().value());

    volumes_.emplace_back(volume_id, volume_name);
  }
}

Chapters::Chapters(std::string json) : JsonBase(std::move(json)) {
  for (auto chapter : doc_["data"]["chapter_list"].get_array()) {
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
      chapters_.emplace_back(chapter_id, chapter_title, "");
    }
  }
}

ChaptersCommand::ChaptersCommand(std::string json) : JsonBase(std::move(json)) {
  command_ = doc_["data"]["command"].get_string().value();
}

Content::Content(std::string json) : JsonBase(std::move(json)) {
  content_ = doc_["data"]["chapter_info"]["txt_content"].get_string().value();
}
