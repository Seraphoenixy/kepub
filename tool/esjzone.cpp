#include <exception>
#include <string>
#include <thread>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <CLI/CLI.hpp>
#include <pugixml.hpp>

#include "html.h"
#include "http.h"
#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::esjzone;

namespace {

pugi::xml_document get_xml(const std::string &url, const std::string &proxy) {
  auto response = http_get(url, proxy);
  return kepub::html_to_xml(response);
}

std::pair<kepub::BookInfo, std::vector<kepub::Chapter>> get_info(
    const std::string &book_id, bool translation, const std::string &proxy) {
  kepub::BookInfo book_info;

  auto doc =
      get_xml("https://www.esjzone.cc/detail/" + book_id + ".html", proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                     "mb-3']/div[@class='col-md-9 book-detail']/h2")
                  .node();
  book_info.name_ = kepub::trans_str(node.text().as_string(), translation);

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-9 book-detail']/ul")
             .node();

  std::string prefix = "作者:";
  for (const auto &child : node.children()) {
    if (child.child("strong").text().as_string() == prefix) {
      book_info.author_ =
          kepub::trans_str(child.child("a").text().as_string(), translation);
    }
  }

  node =
      doc.select_node(
             "/html/body/div[@class='offcanvas-wrapper']/section/div/"
             "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='bg-secondary "
             "p-20 margin-top-1x']/div/div/div")
          .node();

  for (const auto &child : node.children()) {
    kepub::push_back(book_info.introduction_,
                     kepub::trans_str(child.text().as_string(), translation));
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "padding-top-1x mb-3']/div/div/div[@class='tab-pane fade "
                "active show']/div[@id='chapterList']")
             .node();

  std::vector<kepub::Chapter> chapters;
  for (const auto &child : node.children("a")) {
    std::string may_be_url = child.attribute("href").as_string();
    auto title =
        kepub::trans_str(child.child("p").text().as_string(), translation);

    if (!(may_be_url.starts_with("https://www.esjzone.cc/") ||
          may_be_url.starts_with("https://www.esjzone.net/"))) {
      klib::warn("url error: {}, title: {}", may_be_url, title);
    } else {
      chapters.emplace_back(may_be_url, title);
    }
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-3']/div[@class='product-gallery "
                "text-center mb-3']/a/img")
             .node();
  book_info.cover_path_ = node.attribute("src").as_string();

  klib::info("Book name: {}", book_info.name_);
  klib::info("Author: {}", book_info.author_);
  klib::info("Cover url: {}", book_info.cover_path_);

  auto ext = kepub::check_is_supported_format(
      kepub::url_to_file_name(book_info.cover_path_));

  if (ext) {
    std::string cover_name = "cover" + *ext;
    auto response = http_get(book_info.cover_path_, proxy);
    klib::write_file(cover_name, true, response);
    klib::info("Cover downloaded successfully: {}", cover_name);
  }

  return {book_info, chapters};
}

std::vector<std::string> get_content(const std::string &url, bool translation,
                                     const std::string &proxy) {
  auto doc = get_xml(url, proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 "
                     "p-r-30']/div[@class='forum-content mt-3']")
                  .node();

  std::vector<std::string> result;
  for (const auto &text : kepub::get_node_texts(node)) {
    for (const auto &line : klib::split_str(text, "\n")) {
      kepub::push_back(result, kepub::trans_str(line, translation));
    }
  }

  if (std::empty(result)) {
    klib::warn("No text: {}", url);
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

  auto hardware_concurrency = std::thread::hardware_concurrency();
  std::uint32_t max_concurrency = 0;
  app.add_option("-m,--multithreading", max_concurrency,
                 "Maximum number of concurrency to use when downloading")
      ->check(
          CLI::Range(1U, hardware_concurrency > 4 ? hardware_concurrency : 4))
      ->default_val(4);

  std::string proxy;
  app.add_flag("-p{http://127.0.0.1:1080},--proxy{http://127.0.0.1:1080}",
               proxy, "Use proxy")
      ->expected(0, 1);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);
  if (!std::empty(proxy)) {
    klib::info("Use proxy: {}", proxy);
  }
  klib::info("Maximum concurrency: {}", max_concurrency);

  kepub::BookInfo book_info;
  std::vector<kepub::Chapter> chapters;
  std::tie(book_info, chapters) = get_info(book_id, translation, proxy);

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(std::size(chapters), book_info.name_);

  tbb::task_arena limited(max_concurrency);
  tbb::task_group tg;
  limited.execute([&] {
    tg.run([&] {
      tbb::parallel_for_each(chapters, [&](kepub::Chapter &chapter) {
        bar.set_postfix_text(chapter.title_);
        chapter.texts_ = get_content(chapter.url_, translation, proxy);
        bar.tick();
      });
    });
  });
  limited.execute([&] { tg.wait(); });

  kepub::generate_txt(book_info, chapters);
  klib::info("Novel '{}' download completed", book_info.name_);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
