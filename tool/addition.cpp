#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "compress.h"
#include "epub.h"
#include "error.h"
#include "parse_xml.h"
#include "util.h"

std::int32_t last_chapter_num(kepub::XHTML &xhtml) {
  xhtml.reset();
  xhtml.move_by_name("manifest");
  auto result =
      std::stoi(std::string(xhtml.last_child_attr("id")).substr(7, 3)) + 1;
  xhtml.reset();
  return result;
}

std::int32_t deal_with_content_opf(const std::string &dir, std::size_t count) {
  auto str = kepub::read_file_to_str(dir);

  kepub::XHTML xhtml(str);
  auto first_num = last_chapter_num(xhtml);

  auto id = first_num;
  for (std::size_t i = 0; i < count; ++i) {
    auto file_name = kepub::num_to_chapter_name(id++);
    kepub::Epub::add_file_in_content_opf(xhtml, file_name, "Text/" + file_name,
                                         "application/xhtml+xml");
  }

  xhtml.save(dir);

  return first_num;
}

void deal_with_toc_ncx(const std::string &dir,
                       const std::vector<std::string> &titles,
                       std::int32_t first_num) {
  auto str = kepub::read_file_to_str(dir);

  kepub::XHTML xhtml(str);

  auto id = first_num;
  for (const auto &title : titles) {
    auto file_name = kepub::num_to_chapter_name(id++);
    kepub::Epub::add_nav_point(xhtml, title, "Text/" + file_name);
  }

  xhtml.save(dir);
}

void deal_with_chapter(const std::string &dir, std::int32_t first_num,
                       const std::vector<kepub::Content> &contents) {
  auto root = std::filesystem::path(dir);

  for (const auto &item : contents) {
    std::ofstream ofs(root / kepub::num_to_chapter_name(first_num++));
    kepub::check_and_write_file(ofs, kepub::Epub::generate_chapter(item));
  }
}

int main(int argc, char *argv[]) try {
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
  std::string start = "[WEB] ";
  auto start_size = std::size(start);

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(start)) {
      kepub::Content content(vec[i].substr(start_size));
      ++i;

      for (; i < size && !vec[i].starts_with(start); ++i) {
        content.push_line(vec[i]);
      }
      --i;

      contents.push_back(content);
    }
  }

  std::vector<std::string> titles;
  titles.reserve(std::size(contents));
  for (const auto &item : contents) {
    titles.push_back(item.get_title());
  }

  auto first_num = deal_with_content_opf(book_name / "OEBPS" / "content.opf",
                                         std::size(titles));
  deal_with_toc_ncx(book_name / "OEBPS" / "toc.ncx", titles, first_num);
  deal_with_chapter(book_name / "OEBPS" / "Text", first_num, contents);

  kepub::compress(book_name);
  std::filesystem::rename(zip_name, epub_name);

  std::filesystem::remove(file_name);
  std::filesystem::remove_all(book_name);
} catch (const std::exception &err) {
  kepub::error(err.what());
} catch (...) {
  kepub::error("unknown exception");
}
