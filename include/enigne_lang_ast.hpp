// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include <string>
#include <any>
#include "enigne_lang_syntax.hpp"

typedef bool boolean;

class enignelang_ast {
public:
    std::string name;
    std::string node_id;
    std::string which_node;

    enignelang_syntax::enignelang_tokens
                node_type = enignelang_syntax::Undefined;

    enignelang_ast* optional_data = nullptr;
    enignelang_ast* node_l = nullptr;
    enignelang_ast* node_r = nullptr;

    std::vector<enignelang_ast*> other; // body
    std::vector<enignelang_ast*> statement_list; // elif, else
    std::size_t index_of_fn = 0;

    std::string node_current;
    std::string expr_opt;

    std::string goto_sign;

    std::vector<std::string> func_args;
    std::vector<std::string> expr;
public:
    enignelang_ast() {
        this->name = "enigne_global_node_";
        this->node_id = "global_node";
    }

    enignelang_ast(const std::string name,
                   const std::string node_id,
                   enignelang_syntax::enignelang_tokens node_type = enignelang_syntax::Undefined) {
        this->name = name;
        this->node_id = node_id;
        this->node_type = node_type;
    }

    ~enignelang_ast() {
        delete this->node_l; delete this->node_r; delete this->optional_data;
    }

    void clear() noexcept;
};

class enignelang_ast_main {
public:
    enignelang_syntax main_syntax;
    enignelang_ast* ast;
    std::vector<enignelang_ast*> global_decls;
public:
    enignelang_ast_main() {
        this->ast =  new enignelang_ast();
    }

    ~enignelang_ast_main() {
        delete this->ast;
    }
};