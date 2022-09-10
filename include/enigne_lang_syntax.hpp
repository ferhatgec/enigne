// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <map>

#define IMPL_TOKEN(str, equivalent) {str, equivalent}

class enignelang_syntax {
public:
    enum enignelang_tokens : const int {
        Variant = 0, // variant
        Constant, // constant
        Function, // f
        Goto, // goto
        GotoTag, // goto_tag

        Loop, // loop
        LoopIf, // loopif
        LoopElif, // loopelif
        LoopElse, // loopelse

        Break, // break;
        Return, // return
        ConstEval, // const_eval
        Global, // global
        If, // if
        Elif, // elif
        Else, // else
        BuiltIn, // built_in
        Print, // print
        Exec, // exec
        EqualTo, // equal_to
        NotEqualTo, // not_equal_to
        GreaterThan, // greater_than
        LessThan, // less_than


        True, // true
        False, // false

        Iterator, // ->
        VariantID, // @
        FunctionVariant, // $
        LeftPr, // (
        RightPr, // )
        LeftBPr, // [
        RightBPr, // ]
        Sem, // ;
        Plus, // +
        Minus, // -
        Div, // /
        Ast, // *
        Exp, // ^
        Mod, // %
        Factorial, // !
        Space, // ' '
        Eq, // =
        Colon, // :
        Comma, // ,
        Tilde, // ~
        Newline, //

        GlobalNode,
        VariantLit,
        ConstantLit,

        Expr,
        BinOp,
        BinComp,

        Argument,
        StatementExpr,
        StatementBinOp,
        FunctionCall,

        PathExists, // path_exists
        IsDir, // is_dir
        IsFile, // is_file
        IsSymlink, // is_symlink
        ReadFile, // read_file
        Exit, // exit
        Length, // len
        Absolute, // abs
        Ceil, // ceil
        Floor, // floor
        Logarithm, // log
        SquareRoot, // sqrt
        Pi, // pi
        Euler, // e
        StartsWith, // starts_with
        EndsWith, // ends_with
        ToLower, // to_lower
        ToUpper, // to_upper
Undefined = -1
    };


    class enignelang_tokenized_tokens {
    public:
        std::string token;
        enignelang_tokens token_type;
        long long unsigned row = 1, column = 1;
    public:
    };


    const std::map<std::string, enignelang_tokens> tokens = {
            IMPL_TOKEN("global", this->Variant),
            IMPL_TOKEN("variant", this->Variant),

            IMPL_TOKEN("constant", this->Constant),
            IMPL_TOKEN("const", this->Constant),

            IMPL_TOKEN("f", this->Function),
            IMPL_TOKEN("function", this->Function),
            IMPL_TOKEN("func", this->Function),
            IMPL_TOKEN("fn", this->Function),

            IMPL_TOKEN("goto", this->Goto),
            IMPL_TOKEN("goto_tag", this->GotoTag),

            IMPL_TOKEN("loop", this->Loop),
            IMPL_TOKEN("loopif", this->LoopIf),
            IMPL_TOKEN("loopelif", this->LoopElif),
            IMPL_TOKEN("loopelse", this->LoopElse),

            IMPL_TOKEN("break", this->Break),
            IMPL_TOKEN("return", this->Return),

            IMPL_TOKEN("const_eval", this->ConstEval),
            IMPL_TOKEN("~>", this->ConstEval),

            IMPL_TOKEN("global", this->Global),
            IMPL_TOKEN("else if", this->Elif),

            IMPL_TOKEN("if", this->If),
            IMPL_TOKEN("elif", this->Elif),
            IMPL_TOKEN("else", this->Else),

            IMPL_TOKEN("built_in", this->BuiltIn),
            IMPL_TOKEN("print", this->Print),
            IMPL_TOKEN("exec", this->Exec),
            IMPL_TOKEN("equal_to", this->EqualTo),
            IMPL_TOKEN("not_equal_to", this->NotEqualTo),
            IMPL_TOKEN("greater_than", this->GreaterThan),
            IMPL_TOKEN("less_than", this->LessThan),


            IMPL_TOKEN("true", this->True),
            IMPL_TOKEN("false", this->False),

            IMPL_TOKEN("==", this->EqualTo),
            IMPL_TOKEN("!=", this->NotEqualTo),
            IMPL_TOKEN(">>", this->GreaterThan),
            IMPL_TOKEN("<<", this->LessThan),

            IMPL_TOKEN("->", this->Iterator),
            IMPL_TOKEN("until", this->Iterator),

            IMPL_TOKEN("@", this->VariantID),
            IMPL_TOKEN("variant_id", this->VariantID),

            IMPL_TOKEN("$", this->FunctionVariant),
            IMPL_TOKEN("(", this->LeftPr),
            IMPL_TOKEN(")", this->RightPr),
            IMPL_TOKEN("[", this->LeftBPr),
            IMPL_TOKEN("]", this->RightBPr),
            IMPL_TOKEN(";", this->Sem),

            IMPL_TOKEN("+", this->Plus),
            IMPL_TOKEN("add", this->Plus),

            IMPL_TOKEN("-", this->Minus),
            IMPL_TOKEN("sub", this->Minus),

            IMPL_TOKEN("/", this->Div),
            IMPL_TOKEN("div", this->Div),

            IMPL_TOKEN("*", this->Ast),
            IMPL_TOKEN("mul", this->Ast),

            IMPL_TOKEN("^", this->Exp),
            IMPL_TOKEN("pow", this->Exp),

            IMPL_TOKEN("%", this->Mod),
            IMPL_TOKEN("mod", this->Mod),

            IMPL_TOKEN("!", this->Factorial),

            IMPL_TOKEN(" ", this->Space),
            IMPL_TOKEN("=", this->Eq),
            IMPL_TOKEN(":", this->Colon),
            IMPL_TOKEN(",", this->Comma),
            IMPL_TOKEN("~", this->Tilde),
            IMPL_TOKEN("\n", this->Newline),
            
            IMPL_TOKEN("path_exists", this->PathExists),
            IMPL_TOKEN("is_dir", this->IsDir),
            IMPL_TOKEN("is_file", this->IsFile),
            IMPL_TOKEN("is_symlink", this->IsSymlink),
            IMPL_TOKEN("read_file", this->ReadFile),
            IMPL_TOKEN("exit", this->Exit),
            IMPL_TOKEN("len", this->Length),
            IMPL_TOKEN("abs", this->Absolute),
            IMPL_TOKEN("ceil", this->Ceil),
            IMPL_TOKEN("floor", this->Floor),
            IMPL_TOKEN("log", this->Logarithm),
            IMPL_TOKEN("sqrt", this->SquareRoot),
            IMPL_TOKEN("pi", this->Pi),
            IMPL_TOKEN("e", this->Euler),
            IMPL_TOKEN("starts_with", this->StartsWith),
            IMPL_TOKEN("ends_with", this->EndsWith),
            IMPL_TOKEN("to_lower", this->ToLower),
            IMPL_TOKEN("to_upper", this->ToUpper)
    };

    std::string raw_file_data;
    std::vector<std::pair<std::string, std::pair<long long unsigned, long long unsigned>>> unparsed_tokens;
    std::vector<enignelang_tokenized_tokens> tokenized_tokens;
public:
    enignelang_tokens match(std::string val) noexcept;
    void add_file_data(const std::string file) noexcept;
    void tokenize() noexcept;

    static inline void ltrim(std::string& str) {
        str.erase(str.begin(),
                  std::find_if(str.begin(), str.end(),
                               [](unsigned char ch) { return !std::isspace(ch); }
                               ));
    }

    static inline void rtrim(std::string& str) {
        str.erase(std::find_if(str.rbegin(), str.rend(),
                               [](unsigned char ch) { return !std::isspace(ch); }
                               ).base(), str.end());
    }

    static inline void trim(std::string& str) {
        ltrim(str); rtrim(str);
    }

    static inline bool is_valid_number(const std::string& data) {
        const char *p = data.c_str();
        if(!*p || *p == '?') return false;

        long long int s = 1;

        while(*p == ' ') p++;

        if(*p == '-') { s = -1; p++; }

        long double acc = 0;
        while(*p >= '0' && *p <= '9')
            acc = acc * 10 + *p++ - '0';

        if (*p == '.') {
            long double k = 0.1;
            p++;
            while (*p >= '0' && *p <= '9') {
                acc += (*p++ - '0') * k;
                k *= 0.1;
            }
        }

        if(*p) return false;

        return true;
    }

    static inline long double return_num(const std::string& data) {
        const char *p = data.c_str();
        if(!*p || *p == '?' || *p == '"' || *p == '\'') return 0;

        long long int s = 1;

        while(*p == ' ') p++;

        if(*p == '-') { s = -1; p++; }

        long double acc = 0;
        while(*p >= '0' && *p <= '9')
            acc = acc * 10 + *p++ - '0';

        if (*p == '.') {
            long double k = 0.1;
            p++;
            while (*p >= '0' && *p <= '9') {
                acc += (*p++ - '0') * k;
                k *= 0.1;
            }
        }

        if(*p) return 0;

        return acc * s;
    }
};

#undef IMPL_TOKEN