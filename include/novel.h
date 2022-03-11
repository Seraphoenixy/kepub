#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace kepub {

struct Chapter {
  std::string title_;
  std::vector<std::string> texts_;
};

struct Volume {
  std::string title_;
  std::vector<Chapter> chapters_;
};

struct Novel {
  std::string name_;
  std::string author_;

  std::vector<std::string> introduction_;
  std::int32_t illustration_num_ = 0;
  std::vector<std::string> postscript_;

  std::string cover_path_;
  std::vector<std::string> image_paths_;

  std::vector<Volume> volumes_;
};

}  // namespace kepub
