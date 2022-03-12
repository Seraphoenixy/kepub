#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <klib/archive.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <CLI/CLI.hpp>
#include <pugixml.hpp>

#include "epub.h"
#include "novel.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

namespace {

std::string get_root_file_path() {
  kepub::check_file_exist(std::data(kepub::Epub::container_xml_path));

  pugi::xml_document doc;
  doc.load_file(std::data(kepub::Epub::container_xml_path));

  auto node = doc.select_node("/container/rootfiles/rootfile").node();
  if (node.empty()) {
    klib::error("No rootfile: {}", kepub::Epub::container_xml_path);
  }

  return node.attribute("full-path").as_string();
}

std::string get_author(const std::string &root_file_path) {
  kepub::check_file_exist(root_file_path);

  pugi::xml_document doc;
  doc.load_file(root_file_path.c_str());

  auto node = doc.select_node("/package/metadata/dc:creator").node();
  if (node.empty()) {
    klib::warn("No author: {}", root_file_path);
  }

  return node.text().as_string();
}

std::vector<std::string> get_image_paths(const std::string &root_file_path) {
  kepub::check_file_exist(root_file_path);

  pugi::xml_document doc;
  doc.load_file(root_file_path.c_str());

  auto node = doc.select_node("/package/manifest").node();
  if (node.empty()) {
    klib::error("No manifest: {}", root_file_path);
  }

  std::vector<std::string> result;
  const std::string jpeg_media_type = "image/jpeg";
  for (const auto &item : node.children("item")) {
    if (item.attribute("media-type").as_string() == jpeg_media_type) {
      result.emplace_back(item.attribute("href").as_string());
    }
  }

  return result;
}

std::vector<std::string> get_spine_file_paths(
    const std::string &root_file_path) {
  kepub::check_file_exist(root_file_path);

  pugi::xml_document doc;
  doc.load_file(root_file_path.c_str());

  auto node = doc.select_node("/package/spine").node();
  if (node.empty()) {
    klib::error("No spine: {}", root_file_path);
  }

  std::vector<std::string> itemrefs;
  for (const auto &item : node.children("itemref")) {
    itemrefs.emplace_back(item.attribute("idref").as_string());
  }

  node = doc.select_node("/package/manifest").node();
  if (node.empty()) {
    klib::error("No manifest: {}", root_file_path);
  }

  std::vector<std::string> hrefs;
  for (const auto &itemref : itemrefs) {
    auto item = node.find_child_by_attribute("id", itemref.c_str());
    hrefs.emplace_back(item.attribute("href").as_string());
  }

  return hrefs;
}

kepub::Chapter get_file_texts(const std::string &file_path) {
  if (file_path.ends_with("cover.xhtml") ||
      file_path.ends_with("message.xhtml") ||
      (file_path.find("illustration") != std::string::npos)) {
    return {};
  }

  kepub::check_file_exist(file_path);

  pugi::xml_document doc;
  doc.load_file(file_path.c_str());

  auto div = doc.select_node("/html/body/div").node();
  if (div.empty()) {
    klib::error("No div: {}", file_path);
  }

  kepub::Chapter result;

  const std::string h1_name = "h1";
  const std::string p_name = "p";
  const std::string div_name = "div";
  for (const auto &node : div.children()) {
    if (node.name() == h1_name) {
      result.title_ = node.text().as_string();
    } else if (node.name() == p_name) {
      kepub::push_back(result.texts_, node.text().as_string());
    } else if (node.name() == div_name) {
      auto image = node.child("img");
      if (image.empty()) {
        klib::error("No image: {}", file_path);
      }
      auto src = image.attribute("src").as_string();
      auto file_name = std::filesystem::path(src).filename().string();
      result.texts_.push_back("[IMAGE] " + file_name);
    } else {
      klib::warn("Unknown node: '{}' in '{}'", node.name(), file_path);
    }
  }

  if (std::empty(result.title_)) {
    klib::warn("No title: {}", file_path);
  }
  if (std::empty(result.texts_)) {
    klib::warn("No text: {}", file_path);
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string file_name;
  app.add_option("file", file_name, "EPUB file to be processed")->required();

  CLI11_PARSE(app, argc, argv);

  kepub::check_is_epub_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();

  if (std::filesystem::exists(book_name)) {
    kepub::remove_file_or_dir(book_name);
  }
  klib::decompress(file_name, book_name);

  auto dir = std::make_unique<klib::ChangeWorkingDir>(book_name);
  auto root_file_path = get_root_file_path();
  auto author = get_author(root_file_path);
  auto spine_file_paths = get_spine_file_paths(root_file_path);
  auto image_paths = get_image_paths(root_file_path);
  dir.reset();

  std::ostringstream oss;

  oss << "[AUTHOR]"
      << "\n\n"
      << author << "\n\n";

  auto path_prefix =
      (std::filesystem::path(book_name) / root_file_path).parent_path();
  for (const auto &file_path : spine_file_paths) {
    auto chapter = get_file_texts(path_prefix / file_path);
    if (std::empty(chapter.title_) && std::empty(chapter.texts_)) {
      continue;
    }

    if (file_path.ends_with("introduction.xhtml")) {
      oss << "[INTRO]"
          << "\n\n";
    } else if (file_path.ends_with("postscript.xhtml")) {
      oss << "[POST]"
          << "\n\n";
    } else if (file_path.find("volume") != std::string::npos) {
      oss << "[VOLUME] " << chapter.title_ << "\n\n";
    } else {
      oss << "[WEB] " << chapter.title_ << "\n\n";
    }

    for (const auto &line : chapter.texts_) {
      oss << line << '\n';
    }
    oss << '\n';
  }

  std::string str = oss.str();
  // '\n'
  str.pop_back();

  std::ofstream ofs(book_name + ".txt");
  ofs << str << std::flush;

  for (const auto &image_path : image_paths) {
    auto path = path_prefix / image_path;
    if (!std::filesystem::exists(path)) {
      klib::warn("No image: {}", image_path);
    }

    auto image_file_name = std::filesystem::path(image_path).filename();
    auto to_path = std::filesystem::current_path() / image_file_name;
    if (std::filesystem::exists(to_path)) {
      kepub::remove_file_or_dir(to_path);
    }

    std::filesystem::copy(path, to_path);
  }

  kepub::remove_file_or_dir(book_name);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
