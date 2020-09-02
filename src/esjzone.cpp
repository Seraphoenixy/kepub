#include <cctype>
#include <cstddef>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>

#include "common.h"

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
    data.push_back(line);
  }

  return data;
}

std::tuple<std::vector<std::string>, std::vector<std::string>, std::string,
           std::string>
get_content(const std::string &id) {
  std::vector<std::string> urls;
  std::vector<std::string> titles;
  std::string book_name;
  std::string author;

  std::string url_prefix{"<a href=\"https://esjzone.cc/forum/" + id};
  std::string book_name_prefix{"<h2 class=\"p-t-10 text-normal\">"};
  std::string author_prefix{"<li><strong>作者:</strong> <a href=\"/tags/"};

  for (const auto &item : get_html_file(get_url(id))) {
    if (item.starts_with(url_prefix)) {
      auto size{std::size(std::string{"<a href=\""})};
      auto sub_item{item.substr(size, std::size(item) - size)};
      auto index{sub_item.find('\"')};
      auto url{sub_item.substr(0, index)};
      urls.push_back(url);

      auto first{sub_item.find("<p>") + 3};
      auto end{sub_item.find("</p>")};
      auto title{trans_str(sub_item.substr(first, end - first))};

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
    }
  }

  if (std::empty(book_name)) {
    error("book name is empty");
  }
  if (std::empty(author)) {
    error("author name is empty");
  }

  return {urls, titles, book_name, author};
}

std::vector<std::string> get_text(const std::string &url) {
  std::vector<std::string> result;

  for (const auto &item : get_html_file(url)) {
    if (item.starts_with("<p>")) {
      auto item_sub{item.substr(3, std::size(item) - 3)};

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

  return result;
}

int main(int argc, char *argv[]) {
  init_trans();

  auto [input_file, xhtml]{processing_cmd(argc, argv)};

  for (const auto &item : input_file) {
    auto [urls, titles, book_name, author]{get_content(item)};

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
      create_epub_directory(book_name);

      auto size{std::size(urls)};
      for (std::size_t index{}; index < size; ++index) {
        auto title{titles[index]};

        auto filename{get_chapter_filename(book_name, index + 1)};
        std::ofstream ofs{filename};
        check_file_is_open(ofs, filename);

        ofs << chapter_file_begin(title);

        for (const auto &line : get_text(urls[index])) {
          ofs << chapter_file_text(line);
        }

        ofs << chapter_file_end();
      }

      generate_content_opf(book_name, author, size + 1);
      generate_toc_ncx(book_name, titles);
    }
  }
}
