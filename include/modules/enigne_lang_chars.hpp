// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include <string>

namespace enignelang_chars {
    const std::size_t len(const std::string& str) noexcept;
    const std::string starts_with(const std::string& str) noexcept;
    const std::string ends_with(const std::string& str) noexcept;
    const std::string to_upper(std::string str) noexcept;
    const std::string to_lower(std::string str) noexcept;
}