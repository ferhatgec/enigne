// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../../include/modules/enigne_lang_chars.hpp"
#include <algorithm>

const std::size_t enignelang_chars::len(const std::string& str) noexcept {
    return str.length();
}

const std::string enignelang_chars::starts_with(const std::string& str) noexcept {
    return (str.empty()) ? "" : std::string(1, str.front());
}

const std::string enignelang_chars::ends_with(const std::string& str) noexcept {
    return (str.empty()) ? "" : std::string(1, str.back());
}

const std::string enignelang_chars::to_upper(std::string str) noexcept {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return (str.empty()) ? "" : str;
}

const std::string enignelang_chars::to_lower(std::string str) noexcept {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return (str.empty()) ? "" : str;
}
