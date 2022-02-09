#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <klib/http.h>
#include <simdjson.h>

std::string serialize(const std::string &login_name,
                      const std::string &password);

class JsonBase {
 public:
  explicit JsonBase(std::string json);

  [[nodiscard]] bool ok() const {
    return http_code_ == 200 && error_code_ == 200;
  }
  [[nodiscard]] bool login_expired() const {
    return http_code_ == 401 && error_code_ == 502;
  }

 protected:
  simdjson::ondemand::document doc_;

 private:
  std::string json_;
  simdjson::ondemand::parser parser_;

  std::int32_t http_code_;
  std::int32_t error_code_;
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

  [[nodiscard]] const std::string &nick_name() const { return nick_name_; }

 private:
  std::string nick_name_;
};

class BookInfo : public JsonBase {
 public:
  explicit BookInfo(std::string json);

  [[nodiscard]] const std::string &book_name() const { return book_name_; }
  [[nodiscard]] const std::string &author() const { return author_; }
  [[nodiscard]] const std::vector<std::string> &intro() const { return intro_; }
  [[nodiscard]] const std::string &cover_url() const { return cover_url_; }

  [[nodiscard]] double point() const { return point_; }

 private:
  std::string book_name_;
  std::string author_;
  std::vector<std::string> intro_;
  std::string cover_url_;

  double point_;
};

class VolumeChapter : public JsonBase {
 public:
  explicit VolumeChapter(std::string json);

  [[nodiscard]] const auto &get_volume_chapter() const {
    return volume_chapter_;
  }

 private:
  std::vector<
      std::pair<std::string,
                std::vector<std::tuple<std::string, std::string, std::string>>>>
      volume_chapter_;
};

class Content : public JsonBase {
 public:
  explicit Content(std::string json);

  [[nodiscard]] const std::string &content() const { return content_; }

 private:
  std::string content_;
};
