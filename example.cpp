// c++ example.cpp -o example -lenigne -L/usr/local/lib -Wl,-R/usr/local/lib

#include <enigne/enigne_lang_intptr.hpp>
#include <enigne/enigne_lang_main.hpp>
#include <enigne/modules/enigne_lang_fs.hpp>

int main() {
    enignelang main;
    main.syntax.raw_file_data = "print(\"Hello world!\\n\");";
    main.syntax.tokenize();
    main.file(main.syntax.raw_file_data);
    main.parser.parse(main.syntax);
    
    enignelang_intptr data(main.parser.ast_main);
    data.parser = main.parser;
    data.walk(main.parser.ast_main, main.parser.ast_main, enignelang_syntax::GlobalNode,
              main.parser.ast_main->func_args);
    data.start();
}