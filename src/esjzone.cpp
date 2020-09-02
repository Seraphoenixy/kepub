#include <cctype>
#include <string>

// temp
#include <iostream>

#include "common.h"

std::string get_html_file(const std::string &item) {
  for (const auto &c : item) {
    if (!std::isdigit(c)) {
      error("must num a number: {}", item);
    }
  }

  return "fuck you";
}

int main(int argc, char *argv[]) {
  auto [input_file, xhtml]{processing_cmd(argc, argv)};

  for (const auto &item : input_file) {
    std::cout << get_html_file(item) << '\n';
  }
}
