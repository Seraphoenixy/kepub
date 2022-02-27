#include "trans.h"

#include <unordered_map>

#include <klib/unicode.h>
#include <klib/util.h>
#include <opencc.h>
#include <boost/algorithm/string.hpp>

extern char tw2s[];
extern int tw2s_size;

extern char TSPhrases[];
extern int TSPhrases_size;

extern char TWVariantsRevPhrases[];
extern int TWVariantsRevPhrases_size;

extern char TWVariantsRev[];
extern int TWVariantsRev_size;

extern char TSCharacters[];
extern int TSCharacters_size;

namespace kepub {

namespace {

class Converter {
 public:
  Converter() {
    klib::write_file("/tmp/tw2s.json", false, tw2s, tw2s_size);
    klib::write_file("/tmp/TSPhrases.ocd2", true, TSPhrases, TSPhrases_size);
    klib::write_file("/tmp/TWVariantsRevPhrases.ocd2", true,
                     TWVariantsRevPhrases, TWVariantsRevPhrases_size);
    klib::write_file("/tmp/TWVariantsRev.ocd2", true, TWVariantsRev,
                     TWVariantsRev_size);
    klib::write_file("/tmp/TSCharacters.ocd2", true, TSCharacters,
                     TSCharacters_size);
  }

  [[nodiscard]] std::string convert(const std::string &str) const {
    const static opencc::SimpleConverter converter("/tmp/tw2s.json");
    return converter.Convert(str);
  }
};

std::u32string custom_trans(const std::u32string &str, bool translation) {
  std::u32string result;
  result.reserve(std::size(str));

  constexpr auto space = U' ';
  for (auto code_point : str) {
    // https://en.wikipedia.org/wiki/Word_joiner
    if (code_point == U'\uFEFF') [[unlikely]] {
      continue;
    }

    if (klib::is_whitespace(code_point)) {
      // e.g. foo foo
      if (!std::empty(result) &&
          !klib::is_chinese_punctuation(
              static_cast<std::int32_t>(result.back()))) {
        result.push_back(space);
      }
    } else if (klib::is_chinese_punctuation(code_point) ||
               klib::is_english_punctuation(code_point)) {
      // e.g. foo ，
      if (!std::empty(result) && result.back() == space) [[unlikely]] {
        result.pop_back();
      }

      if (code_point == U'?') {
        result.push_back(U'？');
      } else if (code_point == U'!') {
        result.push_back(U'！');
      } else if (code_point == U',') {
        result.push_back(U'，');
      } else if (code_point == U':') {
        result.push_back(U'：');
      } else if (code_point == U';') {
        // https://zh.wikipedia.org/wiki/%E4%B8%8D%E6%8D%A2%E8%A1%8C%E7%A9%BA%E6%A0%BC
        if (result.ends_with(U"&nbsp")) [[unlikely]] {
          boost::erase_tail(result, 5);
          result.push_back(U' ');
        }  // https://pugixml.org/docs/manual.html#loading.options
        else if (result.ends_with(U"&lt")) [[unlikely]] {
          boost::erase_tail(result, 3);
          result.push_back(U'<');
        } else if (result.ends_with(U"&gt")) [[unlikely]] {
          boost::erase_tail(result, 3);
          result.push_back(U'>');
        } else if (result.ends_with(U"&quot")) [[unlikely]] {
          boost::erase_tail(result, 5);
          result.push_back(U'"');
        } else if (result.ends_with(U"&apos")) [[unlikely]] {
          boost::erase_tail(result, 5);
          result.push_back(U'\'');
        } else if (result.ends_with(U"&amp")) [[unlikely]] {
          boost::erase_tail(result, 4);
          result.push_back(U'&');
        } else [[likely]] {
          result.push_back(U'；');
        }
      } else if (code_point == U'(') {
        result.push_back(U'（');
      } else if (code_point == U')') {
        result.push_back(U'）');
      } else if (code_point == U'。') {
        if (!std::empty(result) && result.back() == U'。') {
          continue;
        }
        result.push_back(code_point);
      } else if (code_point == U'，') {
        if (!std::empty(result) && result.back() == U'，') {
          continue;
        }
        result.push_back(code_point);
      } else if (code_point == U'、') {
        if (!std::empty(result) && result.back() == U'、') {
          continue;
        }
        result.push_back(code_point);
      } else {
        result.push_back(code_point);
      }
    } else if (code_point == U'~') {
      result.push_back(U'～');
    } else if (translation) {
      static const std::unordered_map<char32_t, char32_t> map{
          {U'妳', U'你'}, {U'壊', U'坏'}, {U'拚', U'拼'}, {U'噁', U'恶'},
          {U'歳', U'岁'}, {U'経', U'经'}, {U'験', U'验'}, {U'険', U'险'},
          {U'撃', U'击'}, {U'錬', U'炼'}, {U'隷', U'隶'}, {U'毎', U'每'},
          {U'捩', U'折'}, {U'殻', U'壳'}, {U'牠', U'它'}, {U'矇', U'蒙'},
          {U'髮', U'发'}, {U'姊', U'姐'}, {U'黒', U'黑'}, {U'歴', U'历'},
          {U'様', U'样'}, {U'甦', U'苏'}, {U'牴', U'抵'}, {U'銀', U'银'},
          {U'齢', U'龄'}, {U'従', U'从'}, {U'酔', U'醉'}, {U'値', U'值'},
          {U'発', U'发'}, {U'続', U'续'}, {U'転', U'转'}, {U'剣', U'剑'},
          {U'砕', U'碎'}, {U'鉄', U'铁'}, {U'甯', U'宁'}, {U'鬪', U'斗'},
          {U'寛', U'宽'}, {U'変', U'变'}, {U'鳮', U'鸡'}, {U'悪', U'恶'},
          {U'霊', U'灵'}, {U'戦', U'战'}, {U'権', U'权'}, {U'効', U'效'},
          {U'応', U'应'}, {U'覚', U'觉'}, {U'観', U'观'}, {U'気', U'气'},
          {U'覧', U'览'}, {U'殭', U'僵'}, {U'郞', U'郎'}, {U'虊', U'药'},
          {U'踼', U'踢'}, {U'逹', U'达'}, {U'鑜', U'锁'}, {U'髲', U'发'},
          {U'髪', U'发'}, {U'実', U'实'}, {U'內', U'内'}, {U'穨', U'颓'},
          {U'糸', U'系'}, {U'賍', U'赃'}, {U'掦', U'扬'}, {U'覇', U'霸'},
          {U'姉', U'姐'}, {U'楽', U'乐'}, {U'継', U'继'}, {U'隠', U'隐'},
          {U'巻', U'卷'}, {U'膞', U'膊'}, {U'髑', U'骷'}, {U'劄', U'札'},
          {U'擡', U'抬'}, {U'⼈', U'人'}, {U'⾛', U'走'}, {U'⼤', U'大'},
          {U'⽤', U'用'}, {U'⼿', U'手'}, {U'⼦', U'子'}, {U'⽽', U'而'},
          {U'⾄', U'至'}, {U'⽯', U'石'}, {U'⼗', U'十'}, {U'⽩', U'白'},
          {U'⽗', U'父'}, {U'⽰', U'示'}, {U'⾁', U'肉'}, {U'⼠', U'士'},
          {U'⽌', U'止'}, {U'⼀', U'一'}, {U'⺠', U'民'}, {U'揹', U'背'},
          {U'佈', U'布'}, {U'勐', U'猛'}, {U'嗳', U'哎'}, {U'纔', U'才'},
          {U'繄', U'紧'}, {U'勧', U'劝'}, {U'鐡', U'铁'}, {U'犠', U'牺'},
          {U'繊', U'纤'}, {U'郷', U'乡'}, {U'亊', U'事'}, {U'騒', U'骚'},
          {U'聡', U'聪'}, {U'遅', U'迟'}, {U'唖', U'哑'}, {U'獣', U'兽'},
          {U'読', U'读'}, {U'囙', U'因'}, {U'寘', U'置'}, {U'対', U'对'},
          {U'処', U'处'}, {U'団', U'团'}, {U'祢', U'你'}, {U'閙', U'闹'},
          {U'谘', U'咨'}, {U'摀', U'捂'}, {U'類', U'类'}, {U'諷', U'讽'},
          {U'唿', U'呼'}, {U'噹', U'当'}, {U'沒', U'没'}, {U'別', U'别'},
          {U'歿', U'殁'}, {U'羅', U'罗'}, {U'給', U'给'}, {U'頽', U'颓'},
          {U'來', U'来'}, {U'裝', U'装'}, {U'燈', U'灯'}, {U'蓋', U'盖'},
          {U'迴', U'回'}, {U'單', U'单'}, {U'勢', U'势'}, {U'結', U'结'},
          {U'砲', U'炮'}, {U'採', U'采'}, {U'財', U'财'}, {U'頂', U'顶'},
          {U'倆', U'俩'}, {U'幺', U'么'}};
      if (auto iter = map.find(code_point); iter != std::end(map)) {
        result.push_back(iter->second);
      } else {
        result.push_back(code_point);
      }
    } else {
      result.push_back(code_point);
    }
  }

  if (translation) {
    boost::replace_all(result, U"颠复", U"颠覆");
  }
  boost::replace_all(result, U"赤果果", U"赤裸裸");
  boost::replace_all(result, U"赤果", U"赤裸");

  return result;
}

std::string do_trans_str(const std::u32string &str, bool translation) {
  auto std_str = klib::utf32_to_utf8(custom_trans(str, translation));
  klib::trim(std_str);
  return std_str;
}

}  // namespace

std::string trans_str(const char *str, bool translation) {
  return trans_str(std::string_view(str), translation);
}

std::string trans_str(std::string_view str, bool translation) {
  return trans_str(std::string(std::data(str), std::size(str)), translation);
}

std::string trans_str(const std::string &str, bool translation) {
  std::string std_str(std::data(str), std::size(str));
  if (translation) {
    static const Converter converter;
    std_str = converter.convert(std_str);
  }

  return do_trans_str(klib::utf8_to_utf32(std_str), translation);
}

}  // namespace kepub
