#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <string>
#include <string_view>

namespace Exchange {

std::string trimCopy(std::string_view sv, std::string_view separators = " \t\r\n");
std::string trimAndUpperCopy(std::string_view sv, std::string_view separators = " \t\r\n");

} // namespace Exchange

#endif // COMMON_UTILS_H 