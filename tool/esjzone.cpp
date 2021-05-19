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

#include <curl/curl.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "epub.h"
#include "error.h"
#include "trans.h"

std::string html_tidy(const std::string &html) {
  const char *input = html.c_str();
  TidyBuffer output = {};
  TidyBuffer errbuf = {};

  TidyDoc tdoc = tidyCreate();

  std::int32_t rc = -1;
  auto ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
  if (ok) {
    rc = tidySetErrorBuffer(tdoc, &errbuf);
  }
  if (rc >= 0) {
    rc = tidyParseString(tdoc, input);
  }
  if (rc >= 0) {
    rc = tidyCleanAndRepair(tdoc);
  }
  if (rc >= 0) {
    rc = tidyRunDiagnostics(tdoc);
  }
  if (rc > 1) {
    rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
  }
  if (rc >= 0) {
    rc = tidySaveBuffer(tdoc, &output);
  }

  std::string xhtml;
  if (rc >= 0) {
    xhtml.assign(reinterpret_cast<const char *>(output.bp), output.size);
  } else {
    error("A severe error ({}) occurred\n", rc);
  }

  tidyBufFree(&output);
  tidyBufFree(&errbuf);
  tidyRelease(tdoc);

  return xhtml;
}

pugi::xml_node find(const pugi::xml_node &parent, std::string_view name,
                    std::string_view attr_class = "") {
  if (std::empty(attr_class)) {
    return parent.child(name.data());
  }

  for (const auto &node : parent.children(name.data())) {
    if (node.attribute("class").value() == std::string(attr_class)) {
      return node;
    }
  }

  throw std::runtime_error("no child");
}

void get_text(const pugi::xml_node &node, std::string &str) {
  if (node.children().begin() == node.children().end()) {
    str += node.text().as_string();
    return;
  } else {
    for (const auto &child : node.children()) {
      get_text(child, str);
    }
  }
}

std::size_t callback_func_std_string(void *contents, std::size_t size,
                                     std::size_t nmemb, std::string *s) {
  s->append(static_cast<char *>(contents), size * nmemb);
  return size * nmemb;
}

std::string get_url(const std::string &id) {
  for (const auto &c : id) {
    if (!std::isdigit(c)) {
      error("must num a number: {}", id);
    }
  }

  return "https://esjzone.cc/detail/" + id + ".html";
}

std::string get_html_file(const std::string &url) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  auto http_handle = curl_easy_init();
  if (!http_handle) {
    error("curl_easy_init() error");
  }

#ifndef NDEBUG
  curl_easy_setopt(http_handle, CURLOPT_VERBOSE, 1);
#endif

  curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 1);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 2);
  curl_easy_setopt(http_handle, CURLOPT_CAPATH, "/etc/ssl/certs");
  curl_easy_setopt(http_handle, CURLOPT_CAINFO,
                   "/etc/ssl/certs/ca-certificates.crt");
  curl_easy_setopt(
      http_handle, CURLOPT_USERAGENT,
      "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
      "Chrome/91.0.4456.0 Safari/537.36 Edg/91.0.845.2");

  std::string str;
  curl_easy_setopt(http_handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, &str);
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION,
                   callback_func_std_string);

  if (curl_easy_perform(http_handle) != CURLE_OK) {
    error("curl_easy_perform() error");
  }

  curl_easy_cleanup(http_handle);
  curl_global_cleanup();

  if (std::empty(str)) {
    error("get page error");
  }

  return str;
}

std::tuple<std::vector<std::string>, std::vector<std::string>, std::string,
           std::string, std::vector<std::string>>
get_content(const std::string &id) {
  Trans trans;

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

  auto sss{get_html_file(get_url(id))};
  std::vector<std::string> lines;
  boost::split(lines, sss, boost::is_any_of("\n"), boost::token_compress_on);

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
      auto title{trans.trans_str(sub_item.substr(first, end - first - 1))};
      title = title.substr(0, title.find("https"));

      if (std::empty(title)) {
        error("title is empty");
      }

      titles.push_back(title);
    } else if (item.starts_with(book_name_prefix)) {
      auto first{item.find(book_name_prefix) + std::size(book_name_prefix)};
      auto end{item.find("</h2>")};
      book_name = trans.trans_str(item.substr(first, end - first));
    } else if (item.starts_with(author_prefix)) {
      auto first{item.find(author_prefix) + std::size(author_prefix)};
      auto item_sub{item.substr(first)};
      auto index{item_sub.find('/')};
      author = trans.trans_str(item_sub.substr(0, index));
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
        auto str{trans.trans_str(s)};
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
  Trans trans;

  auto xhtml = html_tidy(get_html_file(url));

  pugi::xml_document doc;
  pugi::xml_parse_result err = doc.load_string(xhtml.c_str());
  if (!err) {
    error(err.description());
  }

  auto node = find(doc, "html");
  node = find(node, "body");
  node = find(node, "div", "offcanvas-wrapper");
  node = find(node, "section", "container");
  node = find(node, "div", "row");
  node = find(node, "div", "col-xl-9 col-lg-8 p-r-30");

  auto text_node = find(node, "div", "forum-content mt-3");
  for (const auto &child : text_node) {
    std::string str;
    get_text(child, str);

    if (str.find('\n') != std::string::npos) {
      std::vector<std::string> lines;
      boost::split(lines, str, boost::is_any_of("\n"),
                   boost::token_compress_on);
      for (const auto &item : lines) {
        auto temp = trans.trans_str(item);
        if (!std::empty(temp)) {
          result.push_back(temp);
        }
      }
    } else {
      auto temp = trans.trans_str(str);
      if (!std::empty(temp)) {
        result.push_back(temp);
      }
    }
  }

  if (std::empty(result)) {
    error("text result is empty");
  }

  return result;
}

int main(int argc, char *argv[]) {
  auto [input_file, xhtml]{processing_cmd(argc, argv)};

  for (const auto &item : input_file) {
    auto [urls, titles, book_name, author, description]{get_content(item)};

    if (xhtml) {
      std::vector<std::string> texts;

      auto size{std::size(urls)};
      for (std::size_t index{}; index < size; ++index) {
        texts.push_back("[WEB] " + titles[index]);
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

          auto filename{get_chapter_filename(
              book_name, static_cast<std::int32_t>(index) + 1)};
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

      generate_content_opf(book_name, author,
                           static_cast<std::int32_t>(size) + 1);
      generate_toc_ncx(book_name, titles);
    }
  }

  clean_up();
}
