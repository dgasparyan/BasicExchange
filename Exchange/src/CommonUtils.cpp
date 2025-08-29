#include "CommonUtils.h"
#include <algorithm>
#include <cctype>

namespace Exchange {

std::string trimCopy(std::string_view sv, std::string_view separators) {
  auto start = sv.find_first_not_of(separators);
  if (start == std::string_view::npos) return "";

  auto end = sv.find_last_not_of(separators);
  sv = sv.substr(start, end - start + 1);

  return std::string(sv);
}

std::string trimAndUpperCopy(std::string_view sv, std::string_view separators) {
    std::string result(trimCopy(sv, separators));
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

} // namespace Exchange 