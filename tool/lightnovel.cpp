#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "html.h"
#include "http.h"
#include "trans.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::lightnovel;

namespace {

pugi::xml_document get_xml(const std::string &url, const std::string &proxy) {
  auto response = http_get(url, proxy);
  return kepub::html_to_xml(response);
}

std::vector<std::string> get_content(const pugi::xml_document &doc,
                                     bool translation,
                                     const std::string &proxy) {
  auto node = doc.select_node(
                     "/html/body/div/div/div/div[@class='layout-container']/"
                     "div/div/div[@class='left-contents']/article/"
                     "div[@class='article-content']/article")
                  .node();
  CHECK_NODE(node);

  std::vector<std::string> result;

  static std::int32_t count = 0;
  const static std::string image_prefix = "[IMAGE] ";
  const static auto image_prefix_size = std::size(image_prefix);

  for (const auto &text : kepub::get_node_texts(node, true)) {
    for (const auto &line : klib::split_str(text, "\n")) {
      if (line.starts_with(image_prefix)) {
        try {
          const auto image_url = line.substr(image_prefix_size);
          const auto image = http_get_rss(image_url, proxy);
          const auto image_extension = kepub::image_to_extension(image);
          if (!image_extension) {
            klib::warn("Image is not a supported format: {}", image_url);
            continue;
          }

          std::string new_image_name;
          if (count == 0) {
            new_image_name = "cover" + *image_extension;
            ++count;
          } else {
            new_image_name = kepub::num_to_str(count++) + *image_extension;
            kepub::push_back(result, image_prefix + new_image_name);
          }

          klib::write_file(new_image_name, true, image);
          klib::info("Image download complete: {}", new_image_name);
        } catch (const klib::RuntimeError &err) {
          klib::warn("{}: {}", err.what(), line);
        }
      } else {
        kepub::push_back(result, kepub::trans_str(line, translation));
      }
    }
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.footer(kepub::footer_str());
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  std::string proxy;
  app.add_flag("-p{http://127.0.0.1:1080},--proxy{http://127.0.0.1:1080}",
               proxy, "Use proxy")
      ->expected(0, 1);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);
  if (!std::empty(proxy)) {
    klib::info("Use proxy: {}", proxy);
  }

  const std::string url = "https://www.lightnovel.us/cn/detail/" + book_id;
  klib::info("Download novel from {}", url);

  klib::info("Start downloading novel content");
  auto doc = get_xml(url, proxy);

  auto content = get_content(doc, translation, proxy);
  auto book_name = content.front();

  klib::write_file(book_name + ".txt", false,
                   boost::join(content, "\n") + "\n");
  klib::info("Novel '{}' download completed", book_name);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
