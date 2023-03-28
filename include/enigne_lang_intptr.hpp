// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include "enigne_lang_ast.hpp"
#include "enigne_lang_main.hpp"
#include <stack>
#include <iostream>
#include <functional>

constexpr std::uint8_t recursion_limit = 255;

class enignelang_include_info {
public:
    std::vector<enignelang_ast*> info;
    std::string name;
};

class enignelang_intptr {
    std::stack<enignelang_syntax::enignelang_tokens> def;
    std::vector<std::pair<std::string, std::string>> const_eval_functions;
    std::vector<std::string> current_node = {"global_node"};
    std::vector<std::string> elements, temp;
    
    IMPL_CHECK(need_else);
    IMPL_CHECK(function_skip);

    std::uint8_t recursion_fn_call = 0;
    gechint rcrs_fn = 0;
public:
    std::vector<enignelang_ast*> concatenate_include_infos, 
                                    global_variants;
                                    
    std::vector<enignelang_include_info> include_infos;

    enignelang_ast* main_structure, *jump;
    enignelang_parse parser;

    std::function<void(enignelang_syntax::enignelang_tokens, enignelang_ast*)> 
        callback_signal;
public:
    enignelang_intptr(enignelang_ast* data) : main_structure(data) {
        this->parser.ast_main = data;
    }
    ~enignelang_intptr() = default;

    void walk(enignelang_ast* node,
              enignelang_ast* from,
              enignelang_syntax::enignelang_tokens type,
              std::vector<std::string> func_args) noexcept;
    
    void start() noexcept;

   
    std::string handle_expr(enignelang_ast* expr) noexcept;

    enignelang_ast* copy_array_elements(enignelang_ast* node) noexcept;
    enignelang_ast* handle_var(enignelang_ast* var, const std::string& variable_name) noexcept;
    enignelang_ast* replace_handle_var(enignelang_ast* var, const std::string& variable_name) noexcept;


    void include_external_script(enignelang_ast* node) noexcept;
    void expand(enignelang_ast* node) noexcept;
    
    std::string add(const std::string& left, const std::string& right) noexcept;
    std::string sub(const std::string& left, const std::string& right) noexcept;
    std::string div(const std::string& left, const std::string& right) noexcept;
    std::string mul(const std::string& left, const std::string& right) noexcept;
    std::string mod(const std::string& left, const std::string& right) noexcept;
    
    void callback_method(enignelang_syntax::enignelang_tokens syn, enignelang_ast* node) {
        if(!this->callback_signal)
            return;

        this->callback_signal(syn, node); 
    }

    std::string get_variant_data(const std::string name) noexcept {
        if(name == "+" || name == "-"
        || name == "/" || name == "*"
        || name == "^" || name == "%"
        || this->parser.is_number(name) || (name.front() == '"' && name.back() == '"')) return name;


        for(auto&& val: this->main_structure->other) {
            if(val->node_type == enignelang_syntax::Variant &&
               val->name[0] == name[0]) {
                if(val->name == name) {
                    return (val->node_current);
                }
            }
        }
    }

    gechint safe_stoi(const std::string data) {
        if(data.empty()) return 0;

        try {
            return std::stoi(data);
        } catch(std::invalid_argument const& inv) {
            return 0;
        } catch(std::out_of_range const& oor) {
            return 0;
        }
    }

    long double safe_stold(const std::string data) {
        if(data.empty()) return 0.0l;

        try {
            return std::stold(data);
        } catch(std::invalid_argument const& inv) {
            return 0.0l;
        }
    }

    std::string remove_hints(std::string data) noexcept {
        if(data.empty()) return "";

        if(data.front() == '"' || data.front() == '\'')
            data.erase(data.begin());
        

        if(!data.empty() && (data.back() == '"' || data.back() == '\''))
            data.pop_back();

        return data;
    }
};
