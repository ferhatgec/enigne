// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include <filesystem>
#include <string>

namespace enignelang_fs {
    bool path_exists(const std::string& path) noexcept;
    bool is_dir(const std::string& path) noexcept;
    bool is_file(const std::string& path) noexcept;
    bool is_symlink(const std::string& path) noexcept;

    const std::string read_file(const std::string& path) noexcept;
    const std::string root_path() noexcept;
    const std::string home_path() noexcept;        
}