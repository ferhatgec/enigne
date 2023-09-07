// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../../include/modules/enigne_lang_math.hpp"
#include "../../include/enigne_lang_syntax.hpp"

const std::string enignelang_math::abs(const std::string& num) noexcept {
    return std::to_string(std::abs(enignelang_syntax::return_num(num)));
}

const std::string enignelang_math::ceil(const std::string& num) noexcept {
    return std::to_string(std::ceil(enignelang_syntax::return_num(num)));
}

const std::string enignelang_math::floor(const std::string& num) noexcept {
    return std::to_string(std::floor(enignelang_syntax::return_num(num)));
}

const std::string enignelang_math::round(const std::string& num) noexcept {
    return std::to_string(std::roundl(enignelang_syntax::return_num(num)));
}

const std::string enignelang_math::log(const std::string& num, const std::string& base) noexcept {
    return std::to_string(std::log(enignelang_syntax::return_num(num)) / std::log(enignelang_syntax::return_num(base)));
}

const std::string enignelang_math::sqrt(const std::string& num) noexcept {
    return std::to_string(std::sqrt(enignelang_syntax::return_num(num)));
}

const std::string enignelang_math::pi() noexcept {
    return std::to_string(constant_pi);
}

const std::string enignelang_math::e() noexcept {
    return std::to_string(constant_e);
}