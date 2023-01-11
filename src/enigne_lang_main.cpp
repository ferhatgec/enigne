// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_main.hpp"
#include "../include/enigne_lang_intptr.hpp"
#include "../include/modules/enigne_lang_fs.hpp"
#include <fstream>
#include <cstring>

void enignelang::file(const std::string data) noexcept {
    this->raw_file_data = data;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "enigne interpreter (" << argv[0] << ")\n"
                  << "--------------------\n"
                  
                  << argv[0] << " [arg/s] script\n" <<
                  "[arg/s]:\n" <<  
                  "-variant=variant_data : passing variants from command line\n";
        return 1;
    }
    
    std::string file_name;
    std::vector<enignelang_constant> variants;

    for(unsigned i = 1; i < argc; ++i) {
        if(strlen(argv[i]) > 0 && argv[i][0] == '-') {
            enignelang_constant __constant;

            for(unsigned l = 1; l < strlen(argv[i]); ++l) {
                if(argv[i][l] != '=') {
                    __constant.name.push_back(argv[i][l]);
                } else {
                    for(++l; l < strlen(argv[i]); ++l) {
                        __constant.data.push_back(argv[i][l]);
                    }

                    break;
                }
            }

            variants.push_back(__constant);
        } else {
            file_name = std::string(argv[i]);
        }
    }

    if(!enignelang_fs::is_file(file_name)) {
        std::cout << "note: seems not a file right?\n";
        return 1;
    }

    std::ifstream file(file_name);
    std::string total;
    
    for(std::string script_data; std::getline(file, script_data); total.append(script_data + "\n"))
        ; file.close();

    enignelang main;
    main.syntax.raw_file_data = total;
    main.syntax.tokenize();
    main.file(main.syntax.raw_file_data);

    for(unsigned i = 1; i < argc; i++) {
        main.parser.push_constant("arg_" + std::to_string(::abs(i - 1)), "\"" + 
            std::string(argv[i]) + "\"");
    }

    for(auto& val: variants) {
        main.parser.push_constant(val.name, "\"" + val.data + "\"");
    }

    main.parser.argc = argc;
    main.parser.push_constant("argc", std::to_string(static_cast<long double>(argc - 1)));
    
    main.parser.parse(main.syntax);
    
    enignelang_intptr data(main.parser.ast_main);
    data.parser = main.parser;
    data.walk(main.parser.ast_main, main.parser.ast_main, enignelang_syntax::GlobalNode,
              main.parser.ast_main->func_args);
    data.start();
}