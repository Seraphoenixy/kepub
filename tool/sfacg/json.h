#pragma once

#include <string>
#include <vector>

#include "novel.h"

std::string serialize(const std::string &login_name,
                      const std::string &password);

void json_base(std::string json);

struct UserInfo {
  std::string nick_name_;
  bool login_expired_ = false;
};

UserInfo json_to_user_info(std::string json);

struct LoginInfo {
  UserInfo user_info_;
};

LoginInfo json_to_login_info(std::string json);

kepub::BookInfo json_to_book_info(std::string json);

std::vector<kepub::Volume> json_to_volumes(std::string json);

std::string json_to_chapter_text(std::string json);
