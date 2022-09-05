// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once
#define _USE_MATH_DEFINES

#include <cmath>
#include <string>

constexpr long double constant_pi = M_PI, 
                      constant_e = M_E;

namespace enignelang_math {
    const std::string abs(const std::string& num) noexcept;
    const std::string ceil(const std::string& num) noexcept;
    const std::string floor(const std::string& num) noexcept;
    const std::string log(const std::string& num, 
                            const std::string& base) noexcept;
    const std::string sqrt(const std::string& num) noexcept;

    const std::string pi() noexcept;
    const std::string e() noexcept;
}

#undef _USE_MATH_DEFINES