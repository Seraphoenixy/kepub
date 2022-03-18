#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "config.h"

namespace kepub {

struct KEPUB_EXPORT Chapter {
  Chapter() = default;
  Chapter(std::uint64_t chapter_id, const std::string &title)
      : chapter_id_(chapter_id), title_(title) {}
  Chapter(const std::string &url, const std::string &title)
      : url_(url), title_(title) {}
  Chapter(const std::string &title, const std::vector<std::string> &texts)
      : title_(title), texts_(texts) {}

  std::uint64_t chapter_id_ = 0;
  std::string url_;
  std::string title_;
  std::vector<std::string> texts_;
};

struct KEPUB_EXPORT Volume {
  Volume() = default;
  explicit Volume(const std::string &title) : title_(title) {}
  explicit Volume(const std::vector<Chapter> &chapters) : chapters_(chapters) {}
  Volume(const std::string &title, const std::vector<Chapter> &chapters)
      : title_(title), chapters_(chapters) {}
  Volume(std::uint64_t volume_id, const std::string &title,
         const std::vector<Chapter> &chapters = {})
      : volume_id_(volume_id), title_(title), chapters_(chapters) {}

  std::uint64_t volume_id_ = 0;
  std::string title_;
  std::vector<Chapter> chapters_;
};

struct KEPUB_EXPORT BookInfo {
  std::string name_;
  std::string author_;
  std::vector<std::string> introduction_;
  std::string cover_path_;
  std::string cover_file_name_;

  double point_;
};

struct KEPUB_EXPORT Novel {
  BookInfo book_info_;

  std::int32_t illustration_num_ = 0;
  std::vector<std::string> image_paths_;
  std::vector<std::string> postscript_;

  std::vector<Volume> volumes_;
};

}  // namespace kepub
