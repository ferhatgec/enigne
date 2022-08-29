// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../../include/modules/enigne_lang_fs.hpp"
#include <fstream>
#include <iterator>

bool enignelang_fs::path_exists(const std::string& path) noexcept {
    return std::filesystem::exists(std::filesystem::path(path));
}

bool enignelang_fs::is_dir(const std::string& path) noexcept {
    return std::filesystem::is_directory(std::filesystem::path(path));
} 

bool enignelang_fs::is_file(const std::string& path) noexcept {
    return std::filesystem::is_regular_file(std::filesystem::path(path));
}

bool enignelang_fs::is_symlink(const std::string& path) noexcept {
    return std::filesystem::is_symlink(std::filesystem::path(path));
} 

const std::string enignelang_fs::read_file(const std::string& path) noexcept {
    if(!enignelang_fs::path_exists(path))
        return "";
    
    std::ifstream file(path);
    std::string data;

    for(std::string temp; std::getline(file, temp); data.append(temp + "\n"))
        ; file.close();

    return data;
}

const std::string enignelang_fs::root_path() noexcept {
    return std::filesystem::current_path().root_path().string();
}

const std::string enignelang_fs::home_path() noexcept {
    return "";
}