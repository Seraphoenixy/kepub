#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "compress.h"
#include "epub.h"
#include "parse_html.h"
#include "util.h"

std::int32_t deal_with_content_opf(const std::string &dir, std::int32_t count) {
  auto str = kepub::read_file_to_str(dir);

  kepub::XHTML xhtml(str);
  xhtml.move_by_name("manifest");

  auto last = xhtml.last_child();
  std::int32_t last_num =
      std::stoi(std::string(last.attribute("id").value()).substr(7, 3));

  auto id = last_num + 1;
  for (std::int32_t i = 0; i < count; ++i) {
    kepub::Node node("item");
    node.add_attr("id", kepub::num_to_chapter_name(id));
    node.add_attr("href", "Text/" + kepub::num_to_chapter_name(id++));
    node.add_attr("media-type", "application/xhtml+xml");
    xhtml.push_back(node);
  }

  xhtml.previous();
  xhtml.move_by_name("spine");

  id = last_num + 1;
  for (std::int32_t i = 0; i < count; ++i) {
    kepub::Node node("itemref");
    node.add_attr("idref", kepub::num_to_chapter_name(id++));
    xhtml.push_back(node);
  }

  xhtml.save(dir);

  return last_num;
}

void deal_with_toc_ncx(const std::string &dir,
                       const std::vector<std::string> &titles) {
  auto str = kepub::read_file_to_str(dir);

  kepub::XHTML xhtml(str);
  xhtml.move_by_name("navMap");

  auto last = xhtml.last_child();
  std::int32_t last_num =
      std::stoi(std::string(last.attribute("playOrder").value()));

  std::int32_t chapter_num =
      std::stoi(std::string(last.child("content").attribute("src").value())
                    .substr(12, 3)) +
      1;

  auto size = std::size(titles);
  std::int32_t id = last_num + 1;
  for (std::size_t i = 0; i < size; ++i) {
    kepub::Node node("navPoint");
    node.add_attr("id", "navPoint-" + std::to_string(id));
    node.add_attr("playOrder", std::to_string(id++));

    kepub::Node nav_label("navLabel");
    kepub::Node text("text");
    text.set_text(titles[i]);
    nav_label.add_child(text);

    kepub::Node content("content");
    content.add_attr("src",
                     "Text/" + kepub::num_to_chapter_name(chapter_num++));
    node.add_child(nav_label);
    node.add_child(content);

    xhtml.push_back(node);
  }

  xhtml.save(dir);
}

void deal_with_chapter(const std::string &dir, std::int32_t last_num,
                       const std::vector<kepub::Content> &contents) {
  auto root = std::filesystem::path(dir);

  for (const auto &item : contents) {
    std::ofstream ofs(root / kepub::num_to_chapter_name(++last_num));
    kepub::check_and_write_file(ofs, kepub::Epub::generate_chapter(item));
  }
}

int main(int argc, char *argv[]) {
  auto file_name = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem();
  auto epub_name = book_name.string() + ".epub";
  auto zip_name = book_name.string() + ".zip";
  kepub::check_file_exist(epub_name);

  std::filesystem::rename(epub_name, zip_name);

  kepub::decompress(zip_name, book_name);
  std::filesystem::rename(zip_name, book_name.string() + "-back-up.zip");

  std::vector<kepub::Content> contents;
  auto vec = kepub::read_file_to_vec(file_name);
  auto size = std::size(vec);
  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with("[WEB] ")) {
      kepub::Content content(vec[i].substr(6));
      ++i;

      for (; i < size && !vec[i].starts_with("[WEB] "); ++i) {
        content.push_line(vec[i]);
      }
      --i;

      contents.push_back(content);
    }
  }

  std::vector<std::string> titles;
  titles.reserve(std::size(titles));
  for (const auto &item : contents) {
    titles.push_back(item.get_title());
  }

  auto last_num = deal_with_content_opf(book_name / "OEBPS" / "content.opf",
                                        std::size(titles));
  deal_with_toc_ncx(book_name / "OEBPS" / "toc.ncx", titles);
  deal_with_chapter(book_name / "OEBPS" / "Text", last_num, contents);

  kepub::compress(book_name);
  std::filesystem::rename(zip_name, epub_name);

  std::filesystem::remove(file_name);
  std::filesystem::remove_all(book_name);
}
