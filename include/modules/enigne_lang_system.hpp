// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#ifdef _WIN32
#   define popen _popen
#   define pclose _pclose
#endif

#include <filesystem>
#include <string>

namespace enignelang_system {
    const std::string output(const std::string& command) noexcept;
    const std::string char_input() noexcept;  
}