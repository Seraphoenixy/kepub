#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace kepub {

struct Chapter {
  std::string id_;
  std::string title_;
  std::vector<std::string> texts_;
};

struct Volume {
  std::string id_;
  std::string title_;
  std::vector<Chapter> chapters_;
};

struct BookInfo {
  std::string name_;
  std::string author_;
  std::vector<std::string> introduction_;
  std::string cover_path_;

  double point_;
};

struct Novel {
  BookInfo book_info_;

  std::int32_t illustration_num_ = 0;
  std::vector<std::string> image_paths_;
  std::vector<std::string> postscript_;

  std::vector<Volume> volumes_;
};

}  // namespace kepub
