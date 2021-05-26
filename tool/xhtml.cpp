#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "error.h"
#include "trans.h"
#include "util.h"

int main(int argc, char *argv[]) {
  auto file_name = kepub::processing_cmd(argc, argv);
  if (std::filesystem::path(file_name).extension() != ".txt") {
    kepub::error("need a txt file");
  }

  std::vector<std::string> result;

  for (const auto &item : kepub::read_file_to_vec(file_name)) {
    kepub::push_back(result, kepub::trans_str(item));
  }

  std::string out = std::filesystem::path(file_name).stem().string() + ".xhtml";
  std::ofstream ofs(out);
  if (!ofs) {
    kepub::error("can not open: {}", out);
  }

  for (const auto &item : result) {
    ofs << kepub::chapter_line(item) << '\n';
  }
}
