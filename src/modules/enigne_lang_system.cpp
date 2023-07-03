// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../../include/modules/enigne_lang_system.hpp"

#ifdef _WIN32
#   include <Windows.h>
#   include <conio.h>
#else
#   include <unistd.h>
#   include <termios.h>
#endif

#define SIZE 128

const std::string enignelang_system::output(const std::string& command) noexcept {
    char buffer[SIZE]; // command buffer
    std::string result;
    FILE* file = popen(command.c_str(), "r");

    if(!file)
        return "";

    while(!feof(file))
        if(fgets(buffer, SIZE, file) != nullptr)
            result += buffer;

    pclose(file);

    return result;
}

const std::string enignelang_system::char_input() noexcept {
#ifdef _WIN32
    char ch = static_cast<char>(getch());
    return "\"" + std::string(1, ch) + "\"";
#else
    struct termios t;
    char ch;
    
    tcgetattr(0, &t);
    t.c_lflag &= ~ECHO + ~ICANON;
    tcsetattr(0, TCSANOW, &t);
    
    fflush(stdout);
    
    ch = getchar();
    
    t.c_lflag |= ICANON + ECHO;
    tcsetattr(0, TCSANOW, &t);
    
    return "\"" + std::string(1, ch) + "\"";
#endif
}