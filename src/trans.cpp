#include "trans.h"

#include <unicode/uclean.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include "error.h"

namespace {

void custom_trans(icu::UnicodeString &str) {
  str.findAndReplace("&", "&amp;");
  str.findAndReplace("<", "&lt;");
  str.findAndReplace(">", "&gt;");

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
  str.findAndReplace("歴", "历");
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
}

}  // namespace

Trans::Trans() {
  UErrorCode status = U_ZERO_ERROR;

  trans_ =
      icu::Transliterator::createInstance("Hant-Hans", UTRANS_FORWARD, status);

  if (U_FAILURE(status)) {
    error("error: {}", u_errorName(status));
  }
}

Trans::~Trans() { delete trans_; }

std::string Trans::trans_str(const std::string &str) {
  icu::UnicodeString icu_str(str.c_str());
  icu_str.trim();

  trans_->transliterate(icu_str);
  custom_trans(icu_str);

  std::string temp;
  return icu_str.toUTF8String(temp);
}

void clean_up() { u_cleanup(); }
