#pragma once

#include <string>
#include <vector>

#include "config.h"
#include "novel.h"

namespace kepub {

namespace ciweimao {

std::string KEPUB_PUBLIC serialize(const std::string &account,
                                   const std::string &login_token);

struct KEPUB_PUBLIC Token {
  std::string account_;
  std::string login_token_;
};

Token KEPUB_PUBLIC get_token(std::string json);

struct KEPUB_PUBLIC UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_PUBLIC json_to_user_info(std::string json);

struct KEPUB_PUBLIC LoginInfo {
  Token token_;
  UserInfo user_info_;
};

LoginInfo KEPUB_PUBLIC json_to_login_info(std::string json);

kepub::BookInfo KEPUB_PUBLIC json_to_book_info(std::string json);

std::vector<kepub::Volume> KEPUB_PUBLIC get_volume_info(std::string json);

std::vector<kepub::Chapter> KEPUB_PUBLIC get_chapter_info(std::string json);

std::string KEPUB_PUBLIC get_chapter_command(std::string json);

std::string KEPUB_PUBLIC json_to_chapter_text(std::string json);

}  // namespace ciweimao

namespace sfacg {

std::string KEPUB_PUBLIC serialize(const std::string &login_name,
                                   const std::string &password);

void KEPUB_PUBLIC json_base(std::string json);

struct KEPUB_PUBLIC UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo KEPUB_PUBLIC json_to_user_info(std::string json);

struct KEPUB_PUBLIC LoginInfo {
  UserInfo user_info_;
};

LoginInfo KEPUB_PUBLIC json_to_login_info(std::string json);

kepub::BookInfo KEPUB_PUBLIC json_to_book_info(std::string json);

std::vector<kepub::Volume> KEPUB_PUBLIC json_to_volumes(std::string json);

std::string KEPUB_PUBLIC json_to_chapter_text(std::string json);

}  // namespace sfacg

}  // namespace kepub
