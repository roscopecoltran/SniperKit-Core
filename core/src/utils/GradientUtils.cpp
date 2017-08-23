#include "utils/CoreUtils.hpp"
#include "utils/GradientUtils.hpp"

using namespace utymap::mapcss;
using namespace utymap::utils;

namespace {
const std::string GradientPrefix = "gradient(";

const std::unordered_map<std::string, Color> colorMap =
    {
        {"activeborder", Color(180, 180, 180, 255)},
        {"activecaption", Color(153, 180, 209, 255)},
        {"activecaptiontext", Color(0, 0, 0, 255)},
        {"appworkspace", Color(171, 171, 171, 255)},
        {"control", Color(240, 240, 240, 255)},
        {"controldark", Color(160, 160, 160, 255)},
        {"controldarkdark", Color(105, 105, 105, 255)},
        {"controllight", Color(227, 227, 227, 255)},
        {"controllightlight", Color(255, 255, 255, 255)},
        {"controltext", Color(0, 0, 0, 255)},
        {"desktop", Color(0, 0, 0, 255)},
        {"graytext", Color(109, 109, 109, 255)},
        {"highlight", Color(51, 153, 255, 255)},
        {"highlighttext", Color(255, 255, 255, 255)},
        {"hottrack", Color(0, 102, 204, 255)},
        {"inactiveborder", Color(244, 247, 252, 255)},
        {"inactivecaption", Color(191, 205, 219, 255)},
        {"inactivecaptiontext", Color(0, 0, 0, 255)},
        {"info", Color(255, 255, 225, 255)},
        {"infotext", Color(0, 0, 0, 255)},
        {"menu", Color(240, 240, 240, 255)},
        {"menutext", Color(0, 0, 0, 255)},
        {"scrollbar", Color(200, 200, 200, 255)},
        {"window", Color(255, 255, 255, 255)},
        {"windowframe", Color(100, 100, 100, 255)},
        {"windowtext", Color(0, 0, 0, 255)},
        {"transparent", Color(255, 255, 255, 255)},
        {"aliceblue", Color(240, 248, 255, 255)},
        {"antiquewhite", Color(250, 235, 215, 255)},
        {"aqua", Color(0, 255, 255, 255)},
        {"aquamarine", Color(127, 255, 212, 255)},
        {"azure", Color(240, 255, 255, 255)},
        {"beige", Color(245, 245, 220, 255)},
        {"bisque", Color(255, 228, 196, 255)},
        {"black", Color(0, 0, 0, 255)},
        {"blanchedalmond", Color(255, 235, 205, 255)},
        {"blue", Color(0, 0, 255, 255)},
        {"blueviolet", Color(138, 43, 226, 255)},
        {"brown", Color(165, 42, 42, 255)},
        {"burlywood", Color(222, 184, 135, 255)},
        {"cadetblue", Color(95, 158, 160, 255)},
        {"chartreuse", Color(127, 255, 0, 255)},
        {"chocolate", Color(210, 105, 30, 255)},
        {"coral", Color(255, 127, 80, 255)},
        {"cornflowerblue", Color(100, 149, 237, 255)},
        {"cornsilk", Color(255, 248, 220, 255)},
        {"crimson", Color(220, 20, 60, 255)},
        {"cyan", Color(0, 255, 255, 255)},
        {"darkblue", Color(0, 0, 139, 255)},
        {"darkcyan", Color(0, 139, 139, 255)},
        {"darkgoldenrod", Color(184, 134, 11, 255)},
        {"darkgray", Color(169, 169, 169, 255)},
        {"darkgreen", Color(0, 100, 0, 255)},
        {"darkkhaki", Color(189, 183, 107, 255)},
        {"darkmagenta", Color(139, 0, 139, 255)},
        {"darkolivegreen", Color(85, 107, 47, 255)},
        {"darkorange", Color(255, 140, 0, 255)},
        {"darkorchid", Color(153, 50, 204, 255)},
        {"darkred", Color(139, 0, 0, 255)},
        {"darksalmon", Color(233, 150, 122, 255)},
        {"darkseagreen", Color(143, 188, 139, 255)},
        {"darkslateblue", Color(72, 61, 139, 255)},
        {"darkslategray", Color(47, 79, 79, 255)},
        {"darkturquoise", Color(0, 206, 209, 255)},
        {"darkviolet", Color(148, 0, 211, 255)},
        {"deeppink", Color(255, 20, 147, 255)},
        {"deepskyblue", Color(0, 191, 255, 255)},
        {"dimgray", Color(105, 105, 105, 255)},
        {"dodgerblue", Color(30, 144, 255, 255)},
        {"firebrick", Color(178, 34, 34, 255)},
        {"floralwhite", Color(255, 250, 240, 255)},
        {"forestgreen", Color(34, 139, 34, 255)},
        {"fuchsia", Color(255, 0, 255, 255)},
        {"gainsboro", Color(220, 220, 220, 255)},
        {"ghostwhite", Color(248, 248, 255, 255)},
        {"gold", Color(255, 215, 0, 255)},
        {"goldenrod", Color(218, 165, 32, 255)},
        {"gray", Color(128, 128, 128, 255)},
        {"green", Color(0, 128, 0, 255)},
        {"greenyellow", Color(173, 255, 47, 255)},
        {"honeydew", Color(240, 255, 240, 255)},
        {"hotpink", Color(255, 105, 180, 255)},
        {"indianred", Color(205, 92, 92, 255)},
        {"indigo", Color(75, 0, 130, 255)},
        {"ivory", Color(255, 255, 240, 255)},
        {"khaki", Color(240, 230, 140, 255)},
        {"lavender", Color(230, 230, 250, 255)},
        {"lavenderblush", Color(255, 240, 245, 255)},
        {"lawngreen", Color(124, 252, 0, 255)},
        {"lemonchiffon", Color(255, 250, 205, 255)},
        {"lightblue", Color(173, 216, 230, 255)},
        {"lightcoral", Color(240, 128, 128, 255)},
        {"lightcyan", Color(224, 255, 255, 255)},
        {"lightgoldenrodyellow", Color(250, 250, 210, 255)},
        {"lightgray", Color(211, 211, 211, 255)},
        {"lightgreen", Color(144, 238, 144, 255)},
        {"lightpink", Color(255, 182, 193, 255)},
        {"lightsalmon", Color(255, 160, 122, 255)},
        {"lightseagreen", Color(32, 178, 170, 255)},
        {"lightskyblue", Color(135, 206, 250, 255)},
        {"lightslategray", Color(119, 136, 153, 255)},
        {"lightsteelblue", Color(176, 196, 222, 255)},
        {"lightyellow", Color(255, 255, 224, 255)},
        {"lime", Color(0, 255, 0, 255)},
        {"limegreen", Color(50, 205, 50, 255)},
        {"linen", Color(250, 240, 230, 255)},
        {"magenta", Color(255, 0, 255, 255)},
        {"maroon", Color(128, 0, 0, 255)},
        {"mediumaquamarine", Color(102, 205, 170, 255)},
        {"mediumblue", Color(0, 0, 205, 255)},
        {"mediumorchid", Color(186, 85, 211, 255)},
        {"mediumpurple", Color(147, 112, 219, 255)},
        {"mediumseagreen", Color(60, 179, 113, 255)},
        {"mediumslateblue", Color(123, 104, 238, 255)},
        {"mediumspringgreen", Color(0, 250, 154, 255)},
        {"mediumturquoise", Color(72, 209, 204, 255)},
        {"mediumvioletred", Color(199, 21, 133, 255)},
        {"midnightblue", Color(25, 25, 112, 255)},
        {"mintcream", Color(245, 255, 250, 255)},
        {"mistyrose", Color(255, 228, 225, 255)},
        {"moccasin", Color(255, 228, 181, 255)},
        {"navajowhite", Color(255, 222, 173, 255)},
        {"navy", Color(0, 0, 128, 255)},
        {"oldlace", Color(253, 245, 230, 255)},
        {"olive", Color(128, 128, 0, 255)},
        {"olivedrab", Color(107, 142, 35, 255)},
        {"orange", Color(255, 165, 0, 255)},
        {"orangered", Color(255, 69, 0, 255)},
        {"orchid", Color(218, 112, 214, 255)},
        {"palegoldenrod", Color(238, 232, 170, 255)},
        {"palegreen", Color(152, 251, 152, 255)},
        {"paleturquoise", Color(175, 238, 238, 255)},
        {"palevioletred", Color(219, 112, 147, 255)},
        {"papayawhip", Color(255, 239, 213, 255)},
        {"peachpuff", Color(255, 218, 185, 255)},
        {"peru", Color(205, 133, 63, 255)},
        {"pink", Color(255, 192, 203, 255)},
        {"plum", Color(221, 160, 221, 255)},
        {"powderblue", Color(176, 224, 230, 255)},
        {"purple", Color(128, 0, 128, 255)},
        {"red", Color(255, 0, 0, 255)},
        {"rosybrown", Color(188, 143, 143, 255)},
        {"royalblue", Color(65, 105, 225, 255)},
        {"saddlebrown", Color(139, 69, 19, 255)},
        {"salmon", Color(250, 128, 114, 255)},
        {"sandybrown", Color(244, 164, 96, 255)},
        {"seagreen", Color(46, 139, 87, 255)},
        {"seashell", Color(255, 245, 238, 255)},
        {"sienna", Color(160, 82, 45, 255)},
        {"silver", Color(192, 192, 192, 255)},
        {"skyblue", Color(135, 206, 235, 255)},
        {"slateblue", Color(106, 90, 205, 255)},
        {"slategray", Color(112, 128, 144, 255)},
        {"snow", Color(255, 250, 250, 255)},
        {"springgreen", Color(0, 255, 127, 255)},
        {"steelblue", Color(70, 130, 180, 255)},
        {"tan", Color(210, 180, 140, 255)},
        {"teal", Color(0, 128, 128, 255)},
        {"thistle", Color(216, 191, 216, 255)},
        {"tomato", Color(255, 99, 71, 255)},
        {"turquoise", Color(64, 224, 208, 255)},
        {"violet", Color(238, 130, 238, 255)},
        {"wheat", Color(245, 222, 179, 255)},
        {"white", Color(255, 255, 255, 255)},
        {"whitesmoke", Color(245, 245, 245, 255)},
        {"yellow", Color(255, 255, 0, 255)},
        {"yellowgreen", Color(154, 205, 50, 255)},
        {"buttonface", Color(240, 240, 240, 255)},
        {"buttonhighlight", Color(255, 255, 255, 255)},
        {"buttonshadow", Color(160, 160, 160, 255)},
        {"gradientactivecaption", Color(185, 209, 234, 255)},
        {"gradientinactivecaption", Color(215, 228, 242, 255)},
        {"menubar", Color(240, 240, 240, 255)},
        {"menuhighlight", Color(51, 153, 255, 255)},
        {"grey", Color(190, 190, 190, 255)},
    };

char hexToInt(char hexChar) {
  switch (hexChar) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;

    case 'A':
    case 'a': return 10;

    case 'B':
    case 'b': return 11;

    case 'C':
    case 'c': return 12;

    case 'D':
    case 'd': return 13;

    case 'E':
    case 'e': return 14;

    case 'F':
    case 'f': return 15;
    default:throw std::invalid_argument(std::string("Unknown hex:") + hexChar);
  }
}

inline Color fromName(const std::string &colorStr) {
  auto color = colorMap.find(colorStr);
  return color!=colorMap.end()
         ? color->second
         : colorMap.find("grey")->second;
}

/// Gets integer representation of color from color string.
inline Color fromHex(const std::string &colorStr) {
  auto r = hexToInt(colorStr[2]) + hexToInt(colorStr[1])*16;
  auto g = hexToInt(colorStr[4]) + hexToInt(colorStr[3])*16;
  auto b = hexToInt(colorStr[6]) + hexToInt(colorStr[5])*16;
  return Color(r, g, b, 255);
}
}

/// TODO assume that regex is thread safe
const std::regex GradientUtils::gradientRegEx =
    std::regex(
        "rgb ?\\([ 0-9.%,]+?\\)|#[0-9a-fA-F]{3,6}\\s[0-9]{1,3}[%|px]|#[0-9a-fA-F]{3,6}|([a-zA-Z]+){1}(\\s[0-9]{1,3}\\s*[%|px]?)?");

std::unique_ptr<const ColorGradient> GradientUtils::parseGradient(const std::string &gradientStr) {
  const static std::string prefix = "gradient";
  auto begin = std::sregex_iterator(
      gradientStr.begin() + (gradientStr.compare(0, prefix.size(), prefix) ? 0 : prefix.size()),
      gradientStr.end(),
      gradientRegEx);
  std::sregex_iterator end;

  ColorGradient::GradientData data;
  data.reserve(static_cast<std::size_t>(std::distance(begin, end)));

  for (std::sregex_iterator iter = begin; iter!=end; ++iter) {
    std::stringstream ss(iter->str());
    std::pair<double, Color> pair(iter==begin ? 0 : 1, 0);
    std::string partStr;
    int part = 0;
    while (std::getline(ss, partStr, ' ')) {
      if (part++==0) {
        pair.second = parseColor(partStr);
        continue;
      }
      // NOTE assume that for all intermediate colors time is always defined
      // and skipped for first and last.
      pair.first = utymap::utils::lexicalCast<double>(partStr.substr(0, partStr.size() - 1))/100.;
    }
    data.push_back(pair);
  }

  // NOTE this happens when gradientStr is invalid gradient string.
  if (data.empty() && !gradientStr.empty()) {
    data.push_back(std::make_pair(0., colorMap.find("red")->second));
  }

  return utymap::utils::make_unique<const ColorGradient>(data);
}

bool GradientUtils::isGradient(const std::string &gradientStr) {
  return (GradientPrefix.length() <= gradientStr.length() &&
      std::equal(GradientPrefix.begin(), GradientPrefix.end(), gradientStr.begin())) ||
      gradientStr[0]=='#' || // NOTE we don't check all hex codes
      colorMap.find(gradientStr)!=colorMap.end();
}

Color GradientUtils::parseColor(const std::string &colorStr) {
  return colorStr[0]=='#' ? fromHex(colorStr) : fromName(colorStr);
}

const ColorGradient &GradientUtils::evaluateGradient(const StyleProvider &styleProvider,
                                                     const Style &style,
                                                     const std::string &key) {
  // TODO evaluate gradient using tags
  return styleProvider.getGradient(style.getString(key));
}
