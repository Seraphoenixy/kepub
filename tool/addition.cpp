#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/archive.h>
#include <klib/error.h>
#include <klib/util.h>
#include <pugixml.hpp>

#include "util.h"

std::int32_t last_chapter_num(const pugi::xml_node &node) {
  auto item = node.select_node("/package/manifest").node();

  std::vector<std::int32_t> ids;
  for (const auto &child : item.children("item")) {
    std::string id = child.attribute("id").as_string();
    if (id.starts_with("chapter")) {
      auto num_str = id.substr(7);
      num_str = num_str.substr(0, std::size(num_str) - 6);
      ids.push_back(std::stoi(num_str));
    }
  }
  std::sort(std::begin(ids), std::end(ids));

  return ids.back() + 1;
}

std::int32_t deal_with_content_opf(const std::string &path, std::size_t count,
                                   bool old_style) {
  auto str = klib::read_file(path, false);

  pugi::xml_document doc;
  doc.load_string(str.c_str());

  auto first_num = last_chapter_num(doc);

  auto id = first_num;
  for (std::size_t i = 0; i < count; ++i) {
    auto file_name = kepub::num_to_chapter_name(id++);
    std::string file_path;
    if (old_style) {
      file_path = "OEBPS/Text/" + file_name;
    } else {
      file_path = "Text/" + file_name;
    }

    auto node = doc.select_node("/package/manifest").node();
    auto item = node.append_child("id");
    item.append_attribute("id") = file_name.c_str();
    item.append_attribute("href") = file_path.c_str();
    item.append_attribute("media-type") = "application/xhtml+xml";

    node = doc.select_node("/package/spine").node();
    auto itemref = node.append_child("itemref");
    itemref.append_attribute("idref") = file_name.c_str();
  }

  doc.save_file(path.c_str(), "    ");

  return first_num;
}

void deal_with_toc_ncx(const std::string &path,
                       const std::vector<std::string> &titles,
                       std::int32_t first_num, bool old_style) {
  auto str = klib::read_file(path, false);

  pugi::xml_document doc;
  doc.load_string(str.c_str());

  auto id = doc.select_node("/ncx/navMap")
                .node()
                .last_child()
                .attribute("playOrder")
                .as_int() +
            1;

  for (const auto &title : titles) {
    auto file_name = kepub::num_to_chapter_name(first_num++);
    std::string file_path;
    if (old_style) {
      file_path = "OEBPS/Text/" + file_name;
    } else {
      file_path = "Text/" + file_name;
    }

    auto node = doc.select_node("/ncx/navMap").node();
    auto nav_point = node.append_child("navPoint");
    nav_point.append_attribute("id") =
        ("navPoint-" + std::to_string(id)).c_str();
    nav_point.append_attribute("playOrder") = id;

    nav_point.append_child("navLabel").append_child("text").text() =
        title.c_str();
    nav_point.append_child("content").append_attribute("src") =
        file_path.c_str();
  }

  doc.save_file(path.c_str(), "    ");
}

void deal_with_chapter(
    const std::string &path, std::int32_t first_num,
    const std::vector<std::pair<std::string, std::vector<std::string>>>
        &contents,
    bool old_style) {
  auto root = std::filesystem::path(path);

  for (const auto &[title, text] : contents) {
    std::ofstream ofs(root / kepub::num_to_chapter_name(first_num++));
    (void)title;
    (void)text;
  }
  (void)old_style;
}

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();
  auto epub_name = book_name + ".epub";
  auto zip_name = book_name + ".zip";
  kepub::check_file_exist(epub_name);

  std::filesystem::rename(epub_name, zip_name);

  klib::decompress(zip_name, book_name);
  std::filesystem::rename(zip_name, book_name + "-back-up.zip");

  std::vector<std::pair<std::string, std::vector<std::string>>> contents;

  auto vec = kepub::read_file_to_vec(file_name);
  auto size = std::size(vec);
  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      ++i;

      std::vector<std::string> text;
      for (; i < size && !vec[i].starts_with(title_prefix); ++i) {
        kepub::push_back(text, vec[i], options.connect_chinese_);
      }
      --i;

      contents.emplace_back(title, text);
    }
  }

  auto ptr = std::make_unique<klib::ChangeWorkingDir>(book_name);
  bool old_style = false;
  if (std::filesystem::is_regular_file("content.opf")) {
    old_style = true;
  }

  std::filesystem::path prefix = book_name;
  if (!old_style) {
    prefix = prefix / "OEBPS";
  }

  std::vector<std::string> titles;
  for (const auto &[title, text] : contents) {
    titles.push_back(title);
  }

  auto first_num = deal_with_content_opf(prefix / "content.opf",
                                         std::size(titles), old_style);
  deal_with_toc_ncx(prefix / "toc.ncx", titles, first_num, old_style);
  deal_with_chapter(std::filesystem::path(book_name) / "OEBPS" / "Text",
                    first_num, contents, old_style);

  klib::compress(book_name, klib::Algorithm::Zip, book_name + ".epub", false);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
