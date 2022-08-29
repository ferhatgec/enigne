// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include <string>

#include "enigne_lang_ast.hpp"
#include "enigne_lang_syntax.hpp"
#include "enigne_lang_parse.hpp"

using gechint = long long unsigned;

class enignelangfi {
    std::string current_path;
    std::string extension;
    long long unsigned lines;
public:
};

class enignelang {
    std::string raw_file_data;
public:
    enignelang_syntax syntax;
    enignelangfi file_info;
    enignelang_parse parser;
public:
    void file(const std::string data) noexcept;
};