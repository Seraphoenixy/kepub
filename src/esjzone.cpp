#include <unistd.h>
#include <wait.h>

#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>

#include "common.h"

bool command_success(std::int32_t status) {
  return status != -1 && WIFEXITED(status) && !WEXITSTATUS(status);
}

std::string get_url(const std::string &id) {
  for (const auto &c : id) {
    if (!std::isdigit(c)) {
      error("must num a number: {}", id);
    }
  }

  return "https://esjzone.cc/detail/" + id + ".html";
}

std::vector<std::string> get_html_file(const std::string &url) {
  boost::process::ipstream is;
  boost::process::child c{"/usr/bin/curl", "-s", url,
                          boost::process::std_out > is};

  std::vector<std::string> data;
  std::string line;

  while (c.running() && std::getline(is, line)) {
    boost::trim(line);
    if (!std::empty(line)) {
      data.push_back(line);
    }
  }

  if (std::empty(data)) {
    error("html file is empty");
  }

  return data;
}

std::tuple<std::vector<std::string>, std::vector<std::string>, std::string,
           std::string, std::vector<std::string>>
get_content(const std::string &id) {
  std::vector<std::string> urls;
  std::vector<std::string> titles;
  std::string book_name;
  std::string author;
  std::vector<std::string> description;

  const std::string url_prefix{"<a href=\"https://esjzone.cc/forum/" + id};
  const std::string url_prefix2{"<a href=\"https://www.esjzone.cc/forum/" + id};

  const std::string title_prefix{"data-title=\""};
  const std::string book_name_prefix{"<h2 class=\"p-t-10 text-normal\">"};
  const std::string author_prefix{
      "<li><strong>作者:</strong> <a href=\"/tags/"};
  const std::string description_prefix{"<div class=\"description\">"};

  auto lines{get_html_file(get_url(id))};
  auto lines_size{std::size(lines)};
  for (std::size_t idx{}; idx < lines_size; ++idx) {
    auto item{lines[idx]};

    if (item.starts_with(url_prefix) || item.starts_with(url_prefix2)) {
      auto size{std::size(std::string{"<a href=\""})};
      auto sub_item{item.substr(size, std::size(item) - size)};
      auto index{sub_item.find('\"')};
      auto url{sub_item.substr(0, index)};

      if (std::empty(url)) {
        error("url is empty");
      }
      urls.push_back(url);

      auto first{sub_item.find(title_prefix) + std::size(title_prefix)};
      auto end{sub_item.find('>')};
      auto title{trans_str(sub_item.substr(first, end - first - 1))};
      title = title.substr(0, title.find("https"));

      if (std::empty(title)) {
        error("title is empty");
      }

      titles.push_back(title);
    } else if (item.starts_with(book_name_prefix)) {
      auto first{item.find(book_name_prefix) + std::size(book_name_prefix)};
      auto end{item.find("</h2>")};
      book_name = trans_str(item.substr(first, end - first));
    } else if (item.starts_with(author_prefix)) {
      auto first{item.find(author_prefix) + std::size(author_prefix)};
      auto item_sub{item.substr(first)};
      auto index{item_sub.find('/')};
      author = trans_str(item_sub.substr(0, index));
    } else if (item.starts_with(description_prefix)) {
      auto index{item.find(description_prefix) + std::size(description_prefix)};
      auto item_sub{item.substr(index)};

      while (!item_sub.ends_with("</div>")) {
        ++idx;
        item_sub += lines[idx];
      }

      item_sub = item_sub.substr(0, std::size(item_sub) - 6);

      std::regex re{"</p>"};
      std::vector<std::string> temp{
          std::sregex_token_iterator(std::begin(item_sub), std::end(item_sub),
                                     re, -1),
          {}};

      for (const auto &s : temp) {
        auto str{trans_str(s)};
        if (!std::empty(str)) {
          description.push_back(str.substr(3));
        }
      }
    }
  }

  if (std::empty(urls)) {
    error("urls is empty");
  }
  if (std::empty(titles)) {
    error("titles is empty");
  }
  if (std::empty(book_name)) {
    error("book name is empty");
  }
  if (std::empty(author)) {
    error("author name is empty");
  }
  if (std::empty(description)) {
    error("description is empty");
  }

  return {urls, titles, book_name, author, description};
}

std::vector<std::string> get_text(const std::string &url) {
  std::vector<std::string> result;

  for (const auto &item : get_html_file(url)) {
    if (item.starts_with("<p>")) {
      if (item == "<p>") {
        error("html text format error");
      }

      auto item_sub{item.substr(3, std::size(item) - 3)};

      while (item_sub.starts_with("&nbsp;")) {
        item_sub = item_sub.substr(6);
      }

      auto index{item_sub.find("</p>")};
      item_sub = trans_str(item_sub.substr(0, index));

      if (item_sub == "<br>") {
        continue;
      }

      if (!std::empty(item_sub)) {
        result.push_back(item_sub);
      }
    }
  }

  if (std::empty(result)) {
    error("text result is empty");
  }

  return result;
}

int main(int argc, char *argv[]) {
  init_trans();

  auto [input_file, xhtml]{processing_cmd(argc, argv)};

  for (const auto &item : input_file) {
    auto [urls, titles, book_name, author, description]{get_content(item)};

    if (xhtml) {
      std::vector<std::string> texts;

      auto size{std::size(urls)};
      for (std::size_t index{}; index < size; ++index) {
        texts.push_back(titles[index]);
        for (const auto &line : get_text(urls[index])) {
          texts.push_back(line);
        }
      }

      generate_xhtml(book_name, texts);
    } else {
      create_epub_directory(book_name, description);

      auto size{std::size(urls)};
      for (std::size_t index{}; index < size; ++index) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);

        auto pid{fork()};
        if (pid < 0) {
          error("fork error");
        } else if (pid == 0) {
          auto title{titles[index]};

          auto filename{get_chapter_filename(book_name, index + 1)};
          std::ofstream ofs{filename};
          check_file_is_open(ofs, filename);

          ofs << chapter_file_begin(title);

          for (const auto &line : get_text(urls[index])) {
            ofs << chapter_file_text(line);
          }

          ofs << chapter_file_end() << std::flush;

          return EXIT_SUCCESS;
        }
      }

      std::int32_t status{};

      while (waitpid(-1, &status, 0) > 0) {
        if (!WIFEXITED(status) || WEXITSTATUS(status)) {
          error("waitpid Error");
        }
      }

      generate_content_opf(book_name, author, size + 1);
      generate_toc_ncx(book_name, titles);

      auto copy{boost::replace_all_copy(book_name, " ", "\\ ")};
      std::string cmd{"cd " + copy + " && zip -q -r "};
      cmd.append("../" + copy + ".epub");
      cmd.append(" *");

      if (!command_success(std::system(cmd.c_str()))) {
        error("zip error");
      }

      if (std::filesystem::remove_all(book_name) == 0) {
        error("can not remove directory: {}", book_name);
      }
    }
  }
}
