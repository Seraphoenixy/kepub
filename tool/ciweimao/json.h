#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include <simdjson.h>

std::string serialize(const std::string &account,
                            const std::string &login_token);

class Token {
 public:
  explicit Token(std::string json);

  [[nodiscard]] const std::string &account() const { return account_; }
  [[nodiscard]] const std::string &login_token() const { return login_token_; }

 private:
  std::string account_;
  std::string login_token_;
};

class JsonBase {
 public:
  explicit JsonBase(std::string json);

  [[nodiscard]] bool ok() const { return code_ == Code::Ok; }
  [[nodiscard]] bool login_expired() const {
    return code_ == Code::LoginExpired;
  }

 protected:
  simdjson::ondemand::document doc_;

 private:
  std::string json_;
  simdjson::ondemand::parser parser_;

  enum Code { Ok = 100000, LoginExpired = 200100 };
  std::int32_t code_;
};

class UserInfo : public JsonBase {
 public:
  explicit UserInfo(std::string json);

  [[nodiscard]] const std::string &nick_name() const { return nick_name_; }

 private:
  std::string nick_name_;
};

class LoginInfo : public JsonBase {
 public:
  explicit LoginInfo(std::string json);

  [[nodiscard]] const std::string &account() const { return account_; }
  [[nodiscard]] const std::string &login_token() const { return login_token_; }
  [[nodiscard]] const std::string &nick_name() const { return nick_name_; }

 private:
  std::string account_;
  std::string login_token_;
  std::string nick_name_;
};

class BookInfo : public JsonBase {
 public:
  explicit BookInfo(std::string json);

  [[nodiscard]] const std::string &book_name() const { return book_name_; }
  [[nodiscard]] const std::string &author() const { return author_; }
  [[nodiscard]] const std::vector<std::string> &intro() const { return intro_; }
  [[nodiscard]] const std::string &cover_url() const { return cover_url_; }

 private:
  std::string book_name_;
  std::string author_;
  std::vector<std::string> intro_;
  std::string cover_url_;
};

class Volumes : public JsonBase {
 public:
  using Volume = std::pair<std::string, std::string>;

  explicit Volumes(std::string json);

  [[nodiscard]] const std::vector<Volume> &volumes() const { return volumes_; }

 private:
  std::vector<Volume> volumes_;
};

class Chapters : public JsonBase {
 public:
  using Chapter = std::tuple<std::string, std::string, std::string>;

  explicit Chapters(std::string json);

  [[nodiscard]] const std::vector<Chapter> &chapters() const {
    return chapters_;
  }

 private:
  std::vector<Chapter> chapters_;
};

class ChaptersCommand : public JsonBase {
 public:
  explicit ChaptersCommand(std::string json);

  [[nodiscard]] const std::string &command() const { return command_; }

 private:
  std::string command_;
};

class Content : public JsonBase {
 public:
  explicit Content(std::string json);

  [[nodiscard]] const std::string &content() const { return content_; }

 private:
  std::string content_;
};
