#include "trans.h"

#include <vector>

#include <klib/util.h>
#include <opencc.h>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include "util.h"

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

class Trans {
 public:
  ~Trans();

  Trans(const Trans &) = delete;
  Trans(Trans &&) = delete;
  Trans &operator=(const Trans &) = delete;
  Trans &operator=(Trans &&) = delete;

  static const Trans &get();
  [[nodiscard]] std::string trans_str(std::string_view str,
                                      bool translation) const;

 private:
  Trans();

  icu::Transliterator *fullwidth_halfwidth_;
};

void replace_all_punct(icu::UnicodeString &str, const std::string &old_text,
                       std::string_view new_text) {
  std::string space = " ";
  auto new_text_str = icu::UnicodeString::fromUTF8(new_text);

  str.findAndReplace((space + old_text).c_str(), new_text_str);
  str.findAndReplace((old_text + space).c_str(), new_text_str);
  str.findAndReplace(old_text.c_str(), new_text_str);
}

void replace_all_multi(icu::UnicodeString &str, const std::string &text) {
  auto search_text = icu::UnicodeString::fromUTF8(text + text);
  auto new_text = icu::UnicodeString::fromUTF8(text);

  while (str.indexOf(search_text) != -1) {
    str.findAndReplace(search_text, new_text);
  }
}

void custom_trans(icu::UnicodeString &str, bool translation) {
  replace_error_char(str);

  str.findAndReplace("\t", " ");
  replace_all_multi(str, " ");

  str.findAndReplace("⋯", "…");
  str.findAndReplace("─", "—");

  // https://zh.wikipedia.org/wiki/%E5%85%A8%E5%BD%A2%E5%92%8C%E5%8D%8A%E5%BD%A2
  replace_all_punct(str, "“", "“");
  replace_all_punct(str, "”", "”");
  replace_all_punct(str, "｢", "「");
  replace_all_punct(str, "｣", "」");
  replace_all_punct(str, "『", "『");
  replace_all_punct(str, "』", "』");
  replace_all_punct(str, "《", "《");
  replace_all_punct(str, "》", "》");
  replace_all_punct(str, "【", "【");
  replace_all_punct(str, "】", "】");
  replace_all_punct(str, "(", "（");
  replace_all_punct(str, ")", "）");
  replace_all_punct(str, "…", "…");
  replace_all_punct(str, "—", "—");

  replace_all_punct(str, ",", "，");
  replace_all_punct(str, ":", "：");
  replace_all_punct(str, ";", "；");
  replace_all_punct(str, "｡", "。");
  replace_all_punct(str, "､", "、");
  replace_all_punct(str, "!", "！");
  replace_all_punct(str, "?", "？");
  replace_all_punct(str, "ｰ", "ー");
  replace_all_punct(str, "･", "・");
  replace_all_punct(str, "~", "～");

  replace_all_punct(str, "♂", "♂");
  replace_all_punct(str, "♀", "♀");
  replace_all_punct(str, "◇", "◇");
  replace_all_punct(str, "￮", "￮");
  replace_all_punct(str, "+", "+");
  replace_all_punct(str, "=", "=");
  replace_all_punct(str, "￪", "↑");
  replace_all_punct(str, "￬", "↓");
  replace_all_punct(str, "￩", "←");
  replace_all_punct(str, "￫", "→");

  replace_all_multi(str, "，");
  replace_all_multi(str, "。");
  replace_all_multi(str, "、");

  if (translation) {
    static std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> map{
        {"妳", "你"}, {"壊", "坏"}, {"拚", "拼"}, {"噁", "恶"}, {"歳", "岁"},
        {"経", "经"}, {"験", "验"}, {"険", "险"}, {"撃", "击"}, {"錬", "炼"},
        {"隷", "隶"}, {"毎", "每"}, {"捩", "折"}, {"殻", "壳"}, {"牠", "它"},
        {"矇", "蒙"}, {"髮", "发"}, {"姊", "姐"}, {"黒", "黑"}, {"歴", "历"},
        {"様", "样"}, {"甦", "苏"}, {"牴", "抵"}, {"銀", "银"}, {"齢", "龄"},
        {"従", "从"}, {"酔", "醉"}, {"値", "值"}, {"発", "发"}, {"続", "续"},
        {"転", "转"}, {"剣", "剑"}, {"砕", "碎"}, {"鉄", "铁"}, {"甯", "宁"},
        {"鬪", "斗"}, {"寛", "宽"}, {"変", "变"}, {"鳮", "鸡"}, {"悪", "恶"},
        {"霊", "灵"}, {"戦", "战"}, {"権", "权"}, {"効", "效"}, {"応", "应"},
        {"覚", "觉"}, {"観", "观"}, {"気", "气"}, {"覧", "览"}, {"殭", "僵"},
        {"郞", "郎"}, {"虊", "药"}, {"踼", "踢"}, {"逹", "达"}, {"鑜", "锁"},
        {"髲", "发"}, {"髪", "发"}, {"実", "实"}, {"內", "内"}, {"穨", "颓"},
        {"糸", "系"}, {"賍", "赃"}, {"掦", "扬"}, {"覇", "霸"}, {"姉", "姐"},
        {"楽", "乐"}, {"継", "继"}, {"隠", "隐"}, {"巻", "卷"}, {"膞", "膊"},
        {"髑", "骷"}, {"劄", "札"}, {"擡", "抬"}, {"⼈", "人"}, {"⾛", "走"},
        {"⼤", "大"}, {"⽤", "用"}, {"⼿", "手"}, {"⼦", "子"}, {"⽽", "而"},
        {"⾄", "至"}, {"⽯", "石"}, {"⼗", "十"}, {"⽩", "白"}, {"⽗", "父"},
        {"⽰", "示"}, {"⾁", "肉"}, {"⼠", "士"}, {"⽌", "止"}, {"⼀", "一"},
        {"⺠", "民"}, {"揹", "背"}, {"镳", "镖"}, {"佈", "布"}, {"勐", "猛"},
        {"嗳", "哎"}, {"纔", "才"}, {"繄", "紧"}, {"勧", "劝"}, {"鐡", "铁"},
        {"犠", "牺"}, {"繊", "纤"}, {"郷", "乡"}, {"亊", "事"}, {"騒", "骚"},
        {"聡", "聪"}, {"遅", "迟"}, {"唖", "哑"}, {"獣", "兽"}, {"読", "读"},
        {"囙", "因"}, {"寘", "置"}, {"対", "对"}, {"処", "处"}, {"団", "团"},
        {"祢", "你"}, {"閙", "闹"}, {"谘", "咨"}, {"摀", "捂"}, {"類", "类"},
        {"諷", "讽"}, {"唿", "呼"}, {"噹", "当"}, {"沒", "没"}, {"別", "别"},
        {"歿", "殁"}, {"羅", "罗"}, {"給", "给"}, {"頽", "颓"}, {"來", "来"},
        {"裝", "装"}, {"燈", "灯"}, {"蓋", "盖"}, {"迴", "回"}, {"單", "单"},
        {"勢", "势"}, {"結", "结"}, {"砲", "炮"}, {"採", "采"}, {"財", "财"},
        {"頂", "顶"}, {"倆", "俩"}};
    for (const auto &[from, to] : map) {
      str.findAndReplace(from, to);
    }
  }

  static std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> map2{
      {"幺", "么"},
      {"颠复", "颠覆"},
      {"赤果果", "赤裸裸"},
      {"赤果", "赤裸"},
  };
  for (const auto &[from, to] : map2) {
    str.findAndReplace(from, to);
  }
}

Trans::~Trans() { delete fullwidth_halfwidth_; }

const Trans &Trans::get() {
  static Trans trans;
  return trans;
}

std::string Trans::trans_str(std::string_view str, bool translation) const {
  std::string std_str(str);
  if (translation) {
    const static opencc::SimpleConverter converter("/tmp/tw2s.json");
    std_str = converter.Convert(std_str);
  }

  auto icu_str = icu::UnicodeString::fromUTF8(std_str.c_str());
  fullwidth_halfwidth_->transliterate(icu_str);
  custom_trans(icu_str, translation);

  icu_str.trim();

  std::string temp;
  return icu_str.toUTF8String(temp);
}

Trans::Trans() {
  klib::write_file("/tmp/tw2s.json", false, tw2s, tw2s_size);
  klib::write_file("/tmp/TSPhrases.ocd2", true, TSPhrases, TSPhrases_size);
  klib::write_file("/tmp/TWVariantsRevPhrases.ocd2", true, TWVariantsRevPhrases,
                   TWVariantsRevPhrases_size);
  klib::write_file("/tmp/TWVariantsRev.ocd2", true, TWVariantsRev,
                   TWVariantsRev_size);
  klib::write_file("/tmp/TSCharacters.ocd2", true, TSCharacters,
                   TSCharacters_size);

  UErrorCode status = U_ZERO_ERROR;
  fullwidth_halfwidth_ = icu::Transliterator::createInstance(
      "Fullwidth-Halfwidth", UTRANS_FORWARD, status);
  check_icu(status);
}

}  // namespace

std::string trans_str(const char *str, bool translation) {
  return trans_str(std::string_view(str), translation);
}

std::string trans_str(std::string_view str, bool translation) {
  return Trans::get().trans_str(str, translation);
}

std::string trans_str(const std::string &str, bool translation) {
  return trans_str(std::string_view(str), translation);
}

}  // namespace kepub
