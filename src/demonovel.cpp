#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

void custom_trans(icu::UnicodeString &str) {
  str.findAndReplace("妳", "你");
  str.findAndReplace("壊", "坏");
  str.findAndReplace("拚", "拼");
  str.findAndReplace("噁", "恶");
  str.findAndReplace("歳", "岁");
  str.findAndReplace("経", "经");
  str.findAndReplace("験", "验");
  str.findAndReplace("険", "险");
  str.findAndReplace("撃", "击");
  str.findAndReplace("錬", "炼");
  str.findAndReplace("隷", "隶");
  str.findAndReplace("毎", "每");
  str.findAndReplace("捩", "折");
  str.findAndReplace("殻", "壳");
  str.findAndReplace("牠", "它");
  str.findAndReplace("矇", "蒙");
  str.findAndReplace("髮", "发");
}

std::pair<std::string, std::vector<std::string>>
read_file(const std::string &filename) {
  std::ifstream ifs{filename};
  if (!ifs) {
    std::cerr << "can not open this file: " << filename << '\n';
    std::exit(EXIT_FAILURE);
  }

  UErrorCode status{U_ZERO_ERROR};
  auto trans{
      icu::Transliterator::createInstance("Hant-Hans", UTRANS_FORWARD, status)};
  if (U_FAILURE(status)) {
    std::cerr << "error: " << u_errorName(status) << '\n';
    std::exit(EXIT_FAILURE);
  }

  std::vector<std::string> texts;
  std::string line;

  while (std::getline(ifs, line)) {
    icu::UnicodeString icu_str{line.c_str()};
    icu_str.trim();

    if (!icu_str.isEmpty()) {
      trans->transliterate(icu_str);
      custom_trans(icu_str);

      std::string str;
      texts.push_back(icu_str.toUTF8String(str));
    }
  }

  icu::UnicodeString book_name_icu{
      std::filesystem::path{filename}.filename().stem().string().c_str()};
  book_name_icu.trim();
  trans->transliterate(book_name_icu);
  custom_trans(book_name_icu);
  delete trans;

  std::string book_name;
  book_name_icu.toUTF8String(book_name);

  return {book_name, texts};
}

std::string num_to_str(std::int32_t i) {
  assert(i > 0 && i < 1000);

  if (i < 10) {
    return "00" + std::to_string(i);
  } else if (i < 100) {
    return "0" + std::to_string(i);
  } else {
    return std::to_string(i);
  }
}

void do_create_directory(const std::string &dir) {
  if (!std::filesystem::create_directory(dir)) {
    std::cerr << "can not create directory: " << dir << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void create_directory(const std::string &book_name) {
  if (std::filesystem::exists(book_name)) {
    if (std::filesystem::remove_all(book_name) == 0) {
      std::cerr << "can not remove directory: " << book_name << '\n';
      std::exit(EXIT_FAILURE);
    }
  }

  if (!std::filesystem::exists("MStiffHei PRC Black.ttf")) {
    std::cerr << "can not find: MStiffHei PRC Black.ttf\n";
    std::exit(EXIT_FAILURE);
  }

  do_create_directory(book_name);
  do_create_directory(book_name + "/META-INF");
  do_create_directory(book_name + "/OEBPS");
  do_create_directory(book_name + "/OEBPS/Images");
  do_create_directory(book_name + "/OEBPS/Fonts");
  do_create_directory(book_name + "/OEBPS/Text");

  std::filesystem::copy_file("MStiffHei PRC Black.ttf",
                             book_name +
                                 "/OEBPS/Fonts/MStiffHei PRC Black.ttf");

  std::ofstream stylesheet{book_name + "/stylesheet.css"};
  stylesheet << ".calibre {\n"
                "  display: block;\n"
                "  font-size: 1em;\n"
                "  line-height: 1.2;\n"
                "  text-align: justify;\n"
                "  margin: 0 5pt;\n"
                "  padding: 0% 0;\n"
                "}\n"
                ".calibre1 {\n"
                "  display: block;\n"
                "  line-height: 1.2;\n"
                "  text-align: justify;\n"
                "  margin: 0;\n"
                "  padding: 0;\n"
                "}\n"
                ".calibre2 {\n"
                "  display: block;\n"
                "  line-height: 1.3;\n"
                "  text-indent: 2em;\n"
                "  margin: 0.6em 0;\n"
                "}\n"
                ".calibre3 {\n"
                "  display: block;\n"
                "}\n"
                ".calibre4 {\n"
                "  display: block;\n"
                "  font-size: 1.66667em;\n"
                "  font-weight: bold;\n"
                "  line-height: 1.2;\n"
                "  text-align: center;\n"
                "  margin: 1em 0 1.2em;\n"
                "}\n"
                ".calibre5 {\n"
                "  height: auto;\n"
                "  width: auto;\n"
                "}\n"
                ".calibre6 {\n"
                "  ruby-align: center;\n"
                "}\n"
                ".calibre7 {\n"
                "  font-size: 0.75em;\n"
                "}\n"
                ".center {\n"
                "  display: block;\n"
                "  line-height: 1.2;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: 0;\n"
                "  padding: 0;\n"
                "}\n"
                ".center1 {\n"
                "  display: block;\n"
                "  duokan-bleed: leftright;\n"
                "  line-height: 1.2;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: 0;\n"
                "  padding: 0;\n"
                "}\n"
                ".color {\n"
                "  color: #000;\n"
                "  display: block;\n"
                "  font-size: 1.66667em;\n"
                "  font-weight: bold;\n"
                "  line-height: 1.2;\n"
                "  text-align: center;\n"
                "  margin: 1em 0 1.2em;\n"
                "}\n"
                ".cutline {\n"
                "  display: block;\n"
                "  line-height: 1.2;\n"
                "  text-align: justify;\n"
                "  margin: 0;\n"
                "  padding: 0.25em 0.5em;\n"
                "  border-top: black double medium;\n"
                "  border-bottom: black double medium;\n"
                "}\n"
                ".font {\n"
                "  display: block;\n"
                "  font-size: 0.83333em;\n"
                "  line-height: 1.3;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: 0.6em 0;\n"
                "}\n"
                ".font1 {\n"
                "  color: #000;\n"
                "  display: block;\n"
                "  font-size: 1em;\n"
                "  line-height: 1.3;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: -0.5em 0 0.6em;\n"
                "}\n"
                ".makerifm {\n"
                "  display: block;\n"
                "  line-height: 1.2;\n"
                "  text-indent: 0;\n"
                "  margin: 0.2em 0;\n"
                "}\n"
                ".mt {\n"
                "  display: block;\n"
                "  font-family: black, serif;\n"
                "  font-size: 2em;\n"
                "  line-height: 1.3;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: 1.5em 0 0.6em;\n"
                "}\n"
                ".mt1 {\n"
                "  color: #000;\n"
                "  display: block;\n"
                "  font-family: black, serif;\n"
                "  font-size: 1.66667em;\n"
                "  line-height: 1.3;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: 0.5em 0 0.6em;\n"
                "}\n"
                ".mtb {\n"
                "  display: block;\n"
                "  font-family: black, serif;\n"
                "  font-size: 2em;\n"
                "  line-height: 1.3;\n"
                "  text-align: center;\n"
                "  text-indent: 0;\n"
                "  margin: -0.9em 0 0.6em;\n"
                "}\n"
                ".tco {\n"
                "  color: #fff;\n"
                "  text-shadow: 0 0 5px rgb(147, 75, 95);\n"
                "}\n"
                ".tco1 {\n"
                "  color: #fff;\n"
                "  text-shadow: 0 0 5px rgb(214, 119, 37);\n"
                "}\n"
                ".tco2 {\n"
                "  color: #fff;\n"
                "  text-shadow: 0 0 5px rgb(76, 121, 56);\n"
                "}\n"
                ".tco3 {\n"
                "  color: #fff;\n"
                "  text-shadow: 0 0 5px rgb(17, 82, 102);\n"
                "}\n"
                ".tco4 {\n"
                "  color: #fff;\n"
                "  text-shadow: 0 0 5px rgb(180, 100, 65);\n"
                "}\n"
                ".tco5 {\n"
                "  color: #fff;\n"
                "  text-shadow: 0 0 5px rgb(180, 86, 32);\n"
                "}"
             << std::endl;

  std::ofstream page_styles{book_name + "/page_styles.css"};
  page_styles << "@page {\n"
                 "  margin-bottom: 5pt;\n"
                 "  margin-top: 5pt;\n"
                 "}\n"
                 "@font-face {\n"
                 "  font-family: \"black\";\n"
                 "  src: url(OEBPS/Fonts/MStiffHei%20PRC%20Black.ttf);\n"
                 "}\n"
                 "@font-face {\n"
                 "  font-family: \"Light\";\n"
                 "  src: url(\"OEBPS/Fonts/MStiffHei PRC Light.ttf.ttf\");\n"
                 "}"
              << std::endl;

  std::ofstream mimetype{book_name + "/mimetype"};
  mimetype << "application/epub+zip" << std::endl;

  std::ofstream container{book_name + "/META-INF/container.xml"};
  container << "<?xml version=\"1.0\"?>\n"
               "<container version=\"1.0\" "
               "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
               "   <rootfiles>\n"
               "      <rootfile full-path=\"content.opf\" "
               "media-type=\"application/oebps-package+xml\"/>\n"
               "   </rootfiles>\n"
               "</container>"
            << std::endl;

  std::ofstream introduction{book_name + "/OEBPS/Text/introduction.xhtml"};
  introduction << "<?xml version='1.0' encoding='utf-8'?>\n"
                  "<html\n"
                  "  xmlns=\"http://www.w3.org/1999/xhtml\"\n"
                  "  xmlns:epub=\"http://www.idpf.org/2007/ops\"\n"
                  "  xml:lang=\"zh-CN\"\n"
                  ">\n"
                  "  <head>\n"
                  "    <title>简介</title>\n"
                  "    <meta http-equiv=\"Content-Type\" content=\"text/html; "
                  "charset=utf-8\" />\n"
                  "    <link href=\"../../stylesheet.css\" rel=\"stylesheet\" "
                  "type=\"text/css\" />\n"
                  "    <link href=\"../../page_styles.css\" rel=\"stylesheet\" "
                  "type=\"text/css\" />\n"
                  "  </head>\n"
                  "  <body class=\"calibre\">\n"
                  "    <div class=\"calibre1\">\n"
                  "      <h1 class=\"color\">简介</h1>\n"
                  "    </div>\n"
                  "  </body>\n"
                  "</html>"
               << std::endl;
}

int main(int argc, char *argv[]) {
  boost::program_options::options_description generic("Generic options");
  generic.add_options()("version,v", "print version string");
  generic.add_options()("help,h", "produce help message");

  boost::program_options::options_description config("Configuration");
  config.add_options()("xhtml,x", "only generate xhtml file");

  boost::program_options::options_description hidden("Hidden options");
  hidden.add_options()(
      "input-file", boost::program_options::value<std::vector<std::string>>(),
      "input file");

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  boost::program_options::options_description visible("Allowed options");
  visible.add(generic).add(config);

  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  boost::program_options::variables_map vm;
  store(boost::program_options::command_line_parser(argc, argv)
            .options(cmdline_options)
            .positional(p)
            .run(),
        vm);
  notify(vm);

  if (vm.contains("help")) {
    std::cout << "Usage: " << argv[0] << " [options] file...\n\n";
    std::cout << visible << "\n";
    std::exit(EXIT_SUCCESS);
  }

  if (vm.contains("version")) {
    std::cout << argv[0] << " version: " << __DATE__ << " " << __TIME__ << '\n';
    std::exit(EXIT_SUCCESS);
  }

  if (!vm.contains("input-file")) {
    std::cerr << "need a text file name\n";
    std::exit(EXIT_FAILURE);
  }

  for (const auto &item : vm["input-file"].as<std::vector<std::string>>()) {
    auto [book_name, texts]{read_file(item)};

    if (vm.contains("xhtml")) {
      std::ofstream xhtml{book_name + ".xhtml"};

      for (const auto &line : texts) {
        xhtml << "<p class=\"calibre2\">" << line << "</p>\n";
      }
    } else {
      create_directory(book_name);

      std::int32_t count{1};
      std::vector<std::string> titles;
      auto size{std::size(texts)};

      if (size < 2) {
        std::cerr << "file too small\n";
        std::exit(EXIT_FAILURE);
      }

      const auto author{texts[1]};

      for (std::size_t index{}; index < size; ++index) {
        if (texts[index].starts_with("－－－－－－－－－－－－－－－BEGIN")) {
          index += 2;
          auto title{texts[index]};
          titles.push_back(title);
          index += 2;

          auto file_name{book_name + "/OEBPS/Text/" + "chapter" +
                         num_to_str(count) + ".xhtml"};
          ++count;

          std::ofstream ofs{file_name};
          if (!ofs) {
            std::cerr << "can not create this file: " << file_name << '\n';
            std::exit(EXIT_FAILURE);
          }

          ofs << "<?xml version='1.0' encoding='utf-8'?>\n"
                 "<html\n"
                 "  xmlns=\"http://www.w3.org/1999/xhtml\"\n"
                 "  xmlns:epub=\"http://www.idpf.org/2007/ops\"\n"
                 "  xml:lang=\"zh-CN\"\n"
                 ">\n"
                 "  <head>\n"
                 "    <title>"
              << title
              << "</title>\n"
                 "    <meta http-equiv=\"Content-Type\" content=\"text/html; "
                 "charset=utf-8\" />\n"
                 "    <link href=\"../../stylesheet.css\" rel=\"stylesheet\" "
                 "type=\"text/css\" />\n"
                 "    <link href=\"../../page_styles.css\" rel=\"stylesheet\" "
                 "type=\"text/css\" />\n"
                 "  </head>\n"
                 "  <body class=\"calibre\">\n"
                 "    <div class=\"calibre1\">\n"
                 "      <h1 class=\"color\">"
              << title << "</h1>\n";

          for (; index < size &&
                 !texts[index].starts_with("－－－－－－－－－－－－－－－END");
               ++index) {
            ofs << "      <p class=\"calibre2\">" << texts[index] << "</p>\n";
          }

          ofs << "    </div>\n"
                 "  </body>\n"
                 "</html>"
              << std::endl;
        }
      }

      auto file_name{book_name + "/" + "content.opf"};
      std::ofstream ofs{file_name};
      if (!ofs) {
        std::cerr << "can not create this file: " << file_name << '\n';
        std::exit(EXIT_FAILURE);
      }

      ofs << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
             "<package xmlns=\"http://www.idpf.org/2007/opf\" "
             "unique-identifier=\"uuid_id\" version=\"2.0\">\n"
             "  <metadata "
             "xmlns:calibre=\"http://calibre.kovidgoyal.net/2009/metadata\" "
             "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
             "xmlns:dcterms=\"http://purl.org/dc/terms/\" "
             "xmlns:opf=\"http://www.idpf.org/2007/opf\" "
             "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
             "    <dc:title>"
          << book_name
          << "</dc:title>\n"
             "    <dc:creator opf:role=\"aut\" opf:file-as=\"kaiser\">"
          << author
          << "</dc:creator>\n"
             "    <dc:contributor opf:role=\"bkp\">calibre (4.21.0) "
             "[https://calibre-ebook.com]</dc:contributor>\n"
             "    <dc:rights>kaiser</dc:rights>\n"
             "    <dc:identifier id=\"uuid_id\" "
             "opf:scheme=\"uuid\">b1868e98-d78f-4035-bfe6-ff1e670b6d18</"
             "dc:identifier>\n"
             "    <dc:date>2020-08-02T3:00:00+00:00</dc:date>\n"
             "    <dc:subject>轻小说</dc:subject>\n"
             "    <dc:language>zh</dc:language>\n"
             "    <dc:identifier "
             "opf:scheme=\"calibre\">b1868e98-d78f-4035-bfe6-ff1e670b6d18</"
             "dc:identifier>\n"
             "    <meta name=\"calibre:title_sort\" content=\""
          << book_name
          << "\"/>\n"
             "    <meta name=\"calibre:timestamp\" "
             "content=\"2020-07-31T12:46:51.406000+00:00\"/>\n"
             "    <meta name=\"cover\" content=\"cover\"/>\n"
             "    <meta name=\"calibre:author_link_map\" content=\"{&quot;"
          << author
          << "&quot;: &quot;&quot;}\"/>\n"
             "  </metadata>\n"
             "  <manifest>\n"
             "    <item href=\"OEBPS/Text/introduction.xhtml\" "
             "id=\"introduction.xhtml\" "
             "media-type=\"application/xhtml+xml\"/>\n";

      for (std::int32_t i{1}; i < count; ++i) {
        auto str{num_to_str(i)};
        ofs << "    <item href=\"OEBPS/Text/chapter" << str
            << ".xhtml\" id=\"chapter" << str
            << ".xhtml\" media-type=\"application/xhtml+xml\"/>\n";
      }
      ofs << "    <item href=\"toc.ncx\" id=\"ncx\" "
             "media-type=\"application/x-dtbncx+xml\"/>\n"
             "    <item href=\"page_styles.css\" id=\"page_css\" "
             "media-type=\"text/css\"/>\n"
             "    <item href=\"stylesheet.css\" id=\"css\" "
             "media-type=\"text/css\"/>\n"
             "    <item href=\"OEBPS/Fonts/MStiffHei PRC Black.ttf\" "
             "id=\"MStiffHei_PRC_Black.ttf\" "
             "media-type=\"application/x-font-ttf\"/>\n"
             "</manifest>\n"
             "  <spine toc=\"ncx\">\n"
             "    <itemref idref=\"introduction.xhtml\"/>\n";

      for (std::int32_t i{1}; i < count; ++i) {
        auto str{num_to_str(i)};
        ofs << "    <itemref idref=\"chapter" << str << ".xhtml\"/>\n";
      }
      ofs << "  </spine>\n"
             "  <guide>\n"
             "  </guide>\n"
             "</package>"
          << std::endl;

      ofs.close();

      file_name = book_name + "/" + "toc.ncx";
      ofs.open(file_name);
      if (!ofs) {
        std::cerr << "can not create this file: " << file_name << '\n';
        std::exit(EXIT_FAILURE);
      }

      ofs << "<?xml version='1.0' encoding='utf-8'?>\n"
             "<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" "
             "version=\"2005-1\" xml:lang=\"zho\">\n"
             "  <head>\n"
             "    <meta content=\"b1868e98-d78f-4035-bfe6-ff1e670b6d18\" "
             "name=\"dtb:uid\"/>\n"
             "    <meta content=\"2\" name=\"dtb:depth\"/>\n"
             "    <meta content=\"calibre (4.21.0)\" name=\"dtb:generator\"/>\n"
             "    <meta content=\"0\" name=\"dtb:totalPageCount\"/>\n"
             "    <meta content=\"0\" name=\"dtb:maxPageNumber\"/>\n"
             "  </head>\n"
             "  <docTitle>\n"
             "    <text>"
          << book_name
          << "</text>\n"
             "  </docTitle>\n"
             "  <navMap>\n"
             "    <navPoint class=\"chapter\" id=\"navPoint-1\" "
             "playOrder=\"1\">\n"
             "      <navLabel>\n"
             "        <text>简介</text>\n"
             "      </navLabel>\n"
             "      <content src=\"OEBPS/Text/introduction.xhtml\"/>\n"
             "    </navPoint>\n";

      std::int32_t i{2};
      for (const auto &title : titles) {
        ofs << "    <navPoint class=\"chapter\" id=\"navPoint-" << i
            << "\" playOrder=\"" << i
            << "\">\n"
               "      <navLabel>\n"
               "        <text>"
            << title
            << "</text>\n"
               "      </navLabel>\n"
               "      <content src=\"OEBPS/Text/chapter"
            << num_to_str(i - 1)
            << ".xhtml\"/>\n"
               "    </navPoint>\n";
        ++i;
      }
      ofs << "  </navMap>\n"
             "</ncx>"
          << std::endl;
    }
  }
}
