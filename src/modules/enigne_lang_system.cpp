// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../../include/modules/enigne_lang_system.hpp"
#include <unistd.h>

#define SIZE 128

const std::string enignelang_system::output(const std::string& command) noexcept {
    char buffer[SIZE]; // command buffer
    std::string result;

    FILE* file = popen(command.c_str(), "r");

    if(!pipe)
        return "";

    while(!feof(file)) {
        if(fgets(buffer, SIZE, file) != NULL)
            result += buffer;
    } pclose(file);
    return result;
}