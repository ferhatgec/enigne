// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_main.hpp"
#include "../include/enigne_lang_intptr.hpp"
#include "../include/modules/enigne_lang_fs.hpp"
#include <fstream>

void enignelang::file(const std::string data) noexcept {
    this->raw_file_data = data;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "enigne interpreter (" << argv[0] << ")\n"
                  << "--------------------\n"
                  << argv[0] << " script\n";
        return 1;
    }
    
    if(!enignelang_fs::is_file(std::string(argv[1]))) {
        std::cout << "note: seems not a file right?\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    std::string total;
    
    for(std::string script_data; std::getline(file, script_data); total.append(script_data + "\n"))
        ; file.close();

    enignelang main;
    main.syntax.raw_file_data = total;
    main.syntax.tokenize();
    main.file(main.syntax.raw_file_data);

    for(unsigned i = 1; i < argc; i++) {
        main.parser.push_constant("arg_" + std::to_string(::abs(i - 1)), "\"" + std::string(argv[i]) + "\"");
    }

    main.parser.argc = argc;
    main.parser.push_constant("argc", std::to_string(argc - 1));
    
    main.parser.parse(main.syntax);
    
    enignelang_intptr data(main.parser.ast_main);
    data.parser = main.parser;
    data.walk(main.parser.ast_main, main.parser.ast_main, enignelang_syntax::GlobalNode,
              main.parser.ast_main->func_args);
    data.start();
}