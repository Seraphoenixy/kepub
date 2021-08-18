#include "trans.h"

#include <unicode/translit.h>
#include <unicode/uclean.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include "util.h"

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
  [[nodiscard]] std::string trans_str(const std::string &str) const;

 private:
  Trans();

  icu::Transliterator *hant_hans_;
  icu::Transliterator *fullwidth_halfwidth_;
};

void custom_trans(icu::UnicodeString &str) {
  // https://en.wikipedia.org/wiki/Word_joiner
  str.findAndReplace("\uFEFF", " ");

  str.findAndReplace("&nbsp;", " ");
  str.findAndReplace("\u00A0", " ");

  str.findAndReplace("&lt;", "<");
  str.findAndReplace("&gt;", ">");
  str.findAndReplace("&quot;", "\"");
  str.findAndReplace("&apos;", "'");
  str.findAndReplace("&amp;", "&");

  str.findAndReplace(";", "；");

  // https://zh.wikipedia.org/wiki/%E5%85%A8%E5%BD%A2%E5%92%8C%E5%8D%8A%E5%BD%A2
  str.findAndReplace("!", "！");
  str.findAndReplace("(", "（");
  str.findAndReplace(")", "）");
  str.findAndReplace(",", "，");
  str.findAndReplace(":", "：");
  str.findAndReplace("?", "？");
  str.findAndReplace("｡", "。");
  str.findAndReplace("､", "、");
  str.findAndReplace("･", "・");
  str.findAndReplace("~", "～");
  str.findAndReplace("ｰ", "ー");
  str.findAndReplace("￫", "→");
  str.findAndReplace("￩", "←");

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
  str.findAndReplace("姊", "姐");
  str.findAndReplace("黒", "黑");
  str.findAndReplace("歴", "历");
  str.findAndReplace("様", "样");
  str.findAndReplace("甦", "苏");
  str.findAndReplace("牴", "抵");
  str.findAndReplace("銀", "银");
  str.findAndReplace("齢", "龄");
  str.findAndReplace("祂", "他");
  str.findAndReplace("従", "从");
  str.findAndReplace("酔", "醉");
  str.findAndReplace("値", "值");
  str.findAndReplace("発", "发");
  str.findAndReplace("続", "续");
  str.findAndReplace("転", "转");
  str.findAndReplace("剣", "剑");
  str.findAndReplace("砕", "碎");
  str.findAndReplace("鉄", "铁");
  str.findAndReplace("甯", "宁");
  str.findAndReplace("鬪", "斗");
  str.findAndReplace("寛", "宽");
  str.findAndReplace("変", "变");
  str.findAndReplace("鳮", "鸡");
  str.findAndReplace("悪", "恶");
  str.findAndReplace("霊", "灵");
  str.findAndReplace("戦", "战");
  str.findAndReplace("権", "权");
  str.findAndReplace("効", "效");
  str.findAndReplace("応", "应");
  str.findAndReplace("覚", "觉");
  str.findAndReplace("観", "观");
  str.findAndReplace("気", "气");
  str.findAndReplace("覧", "览");
  str.findAndReplace("殭", "僵");
  str.findAndReplace("郞", "郎");
  str.findAndReplace("虊", "药");
  str.findAndReplace("踼", "踢");
  str.findAndReplace("逹", "达");
  str.findAndReplace("鑜", "锁");
  str.findAndReplace("髲", "发");
  str.findAndReplace("髪", "发");
  str.findAndReplace("実", "实");
  str.findAndReplace("內", "内");
  str.findAndReplace("穨", "颓");
  str.findAndReplace("糸", "系");
  str.findAndReplace("賍", "赃");
  str.findAndReplace("掦", "扬");
  str.findAndReplace("覇", "霸");
  str.findAndReplace("姉", "姐");
  str.findAndReplace("楽", "乐");
  str.findAndReplace("継", "继");
  str.findAndReplace("隠", "隐");
  str.findAndReplace("巻", "卷");
  str.findAndReplace("膞", "膊");
  str.findAndReplace("髑", "骷");
  str.findAndReplace("劄", "札");
  str.findAndReplace("擡", "抬");
  str.findAndReplace("⼈", "人");
  str.findAndReplace("⾛", "走");
  str.findAndReplace("⼤", "大");
  str.findAndReplace("⽤", "用");
  str.findAndReplace("⼿", "手");
  str.findAndReplace("⼦", "子");
  str.findAndReplace("⽽", "而");
  str.findAndReplace("⾄", "至");
  str.findAndReplace("⽯", "石");
  str.findAndReplace("⼗", "十");
  str.findAndReplace("⽩", "白");
  str.findAndReplace("⽗", "父");
  str.findAndReplace("⽰", "示");
  str.findAndReplace("⾁", "肉");
  str.findAndReplace("⼠", "士");
  str.findAndReplace("⽌", "止");
  str.findAndReplace("⼀", "一");
  str.findAndReplace("⺠", "民");
  str.findAndReplace("揹", "背");
  str.findAndReplace("镳", "镖");
  str.findAndReplace("佈", "布");
  str.findAndReplace("勐", "猛");
  str.findAndReplace("嗳", "哎");
  str.findAndReplace("纔", "才");
  str.findAndReplace("繄", "紧");
  str.findAndReplace("勧", "劝");
  str.findAndReplace("鐡", "铁");
  str.findAndReplace("犠", "牺");
  str.findAndReplace("繊", "纤");
  str.findAndReplace("郷", "乡");
  str.findAndReplace("亊", "事");
  str.findAndReplace("騒", "骚");
  str.findAndReplace("聡", "聪");
  str.findAndReplace("遅", "迟");
  str.findAndReplace("唖", "哑");
  str.findAndReplace("獣", "兽");
  str.findAndReplace("読", "读");
  str.findAndReplace("囙", "因");
  str.findAndReplace("寘", "置");
  str.findAndReplace("対", "对");
  str.findAndReplace("処", "处");
  str.findAndReplace("団", "团");
  str.findAndReplace("祢", "你");
  str.findAndReplace("閙", "闹");
  str.findAndReplace("谘", "咨");
  str.findAndReplace("摀", "捂");
  str.findAndReplace("類", "类");
  str.findAndReplace("諷", "讽");
  str.findAndReplace("唿", "呼");

  str.findAndReplace("摔交", "摔跤");
  str.findAndReplace("万事具备", "万事俱备");
  str.findAndReplace("具乐部", "俱乐部");
  str.findAndReplace("两败具伤", "两败俱伤");
  str.findAndReplace("万念具灰", "万念俱灰");
  str.findAndReplace("身心具疲", "身心俱疲");
  str.findAndReplace("着称", "著称");
  str.findAndReplace("色香味具全", "色香味俱全");
  str.findAndReplace("形神具灭", "形神俱灭");
  str.findAndReplace("玉石具焚", "玉石俱焚");
  str.findAndReplace("与生具来", "与生俱来");
  str.findAndReplace("了望", "瞭望");
  str.findAndReplace("与日具增", "与日俱增");
}

Trans::~Trans() {
  delete hant_hans_;
  delete fullwidth_halfwidth_;
  u_cleanup();
}

const Trans &Trans::get() {
  static Trans trans;
  return trans;
}

std::string Trans::trans_str(const std::string &str) const {
  icu::UnicodeString icu_str(str.c_str());

  hant_hans_->transliterate(icu_str);
  fullwidth_halfwidth_->transliterate(icu_str);
  custom_trans(icu_str);

  icu_str.trim();

  std::string temp;
  return icu_str.toUTF8String(temp);
}

Trans::Trans() {
  UErrorCode status = U_ZERO_ERROR;

  hant_hans_ =
      icu::Transliterator::createInstance("Hant-Hans", UTRANS_FORWARD, status);
  check_icu(status);

  fullwidth_halfwidth_ = icu::Transliterator::createInstance(
      "Fullwidth-Halfwidth", UTRANS_FORWARD, status);
  check_icu(status);
}

}  // namespace

std::string trans_str(const std::string &str) {
  return Trans::get().trans_str(str);
}

}  // namespace kepub
