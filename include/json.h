#pragma once

#include <string>
#include <vector>

#include "kepub_export.h"
#include "novel.h"

namespace kepub {

namespace masiro {

void KEPUB_EXPORT json_base(std::string json);

}  // namespace masiro

namespace ciweimao {

std::string KEPUB_EXPORT serialize(const std::string &account,
                                   const std::string &login_token);

struct KEPUB_EXPORT Token {
  std::string account_;
  std::string login_token_;
};

Token KEPUB_EXPORT get_token(std::string json);

struct KEPUB_EXPORT UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_EXPORT json_to_user_info(std::string json);

struct KEPUB_EXPORT LoginInfo {
  Token token_;
  UserInfo user_info_;
};

LoginInfo KEPUB_EXPORT json_to_login_info(std::string json);

kepub::BookInfo KEPUB_EXPORT json_to_book_info(std::string json);

std::vector<kepub::Volume> KEPUB_EXPORT get_volume_info(std::string json);

std::vector<kepub::Chapter> KEPUB_EXPORT get_chapter_info(std::string json);

std::string KEPUB_EXPORT get_chapter_command(std::string json);

std::string KEPUB_EXPORT json_to_chapter_text(std::string json);

}  // namespace ciweimao

namespace sfacg {

std::string KEPUB_EXPORT serialize(const std::string &login_name,
                                   const std::string &password);

void KEPUB_EXPORT json_base(std::string json);

struct KEPUB_EXPORT UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_EXPORT json_to_user_info(std::string json);

struct KEPUB_EXPORT LoginInfo {
  UserInfo user_info_;
};

LoginInfo KEPUB_EXPORT json_to_login_info(std::string json);

kepub::BookInfo KEPUB_EXPORT json_to_book_info(std::string json);

std::vector<kepub::Volume> KEPUB_EXPORT json_to_volumes(std::string json);

std::string KEPUB_EXPORT json_to_chapter_text(std::string json);

}  // namespace sfacg

}  // namespace kepub
