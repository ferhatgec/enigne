// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#pragma once

#include <iostream>
#include <stdexcept>
#include <map>
#include "enigne_lang_ast.hpp"
#include "enigne_lang_syntax.hpp"

#define IMPL_CHECK(name) bool name = false;
#define IMPL_CONSTANT(insert_to, name, data) insert_to.insert(name, data);

class enignelang_constant {
public:
    std::string name;
    std::string data;
public:
};

class enignelang_parse {
    IMPL_CHECK(is_variable)
    IMPL_CHECK(is_constant)
    IMPL_CHECK(is_function)
    IMPL_CHECK(is_expression)
    IMPL_CHECK(is_loop)
    IMPL_CHECK(is_if)
    IMPL_CHECK(is_elif)
    IMPL_CHECK(is_else)
    IMPL_CHECK(is_body)
    IMPL_CHECK(is_function_call)
    IMPL_CHECK(is_return)
    IMPL_CHECK(is_built_in_function)
    IMPL_CHECK(is_function_call_argument)
    IMPL_CHECK(is_const_eval)
    IMPL_CHECK(is_global)
    IMPL_CHECK(is_variant_id)
    IMPL_CHECK(is_variant_addr)
    IMPL_CHECK(is_iterator)
    IMPL_CHECK(is_possible_variant_id)

    enignelang_ast* ast_decl;
    enignelang_ast_main* ast;

    std::vector<std::pair<std::string, enignelang_syntax::enignelang_tokens>> current_nodes
        = { std::make_pair("global_node", enignelang_syntax::GlobalNode) };

    std::vector<std::string> current_expr;

    std::vector<enignelang_ast*> args;

    std::string current_node = "global_node";
    unsigned i_if = 0, i_addr = 0, i_loop = 0, i_call = 0;

    std::size_t index = 0;
    std::vector<enignelang_ast*> arg_handle;

    std::vector<enignelang_syntax::enignelang_tokenized_tokens> current;

    unsigned inline_prs = 0;
public:
    std::vector<enignelang_constant> constants;

    unsigned argc = 0;

    std::vector<enignelang_ast*> const_evals;
    enignelang_ast* ast_main;
public:
    enignelang_parse() {
        this->ast = new enignelang_ast_main();

        this->ast_main = new enignelang_ast();
        this->ast_decl = new enignelang_ast();

        this->ast_main->node_type = enignelang_syntax::GlobalNode;

        this->push_constant("cpp_time", __TIME__);
        this->push_constant("cpp_date", __DATE__);
    }

    ~enignelang_parse() {}

    void parse(enignelang_syntax syn) noexcept;
    void built_pre_ast(enignelang_ast data) noexcept;
    // void read_ast(enignelang_ast* node) noexcept;

    boolean check_index() {
        return this->index + 1 < this->current.size();
    }

    enignelang_ast* handle_single_argument(std::vector<enignelang_ast*>& arg_handle, 
                                            enignelang_ast* from) noexcept;
    
    void handle_start(enignelang_ast* node) noexcept;

    /*enignelang_ast* get_last(enignelang_ast* node) noexcept;
    enignelang_ast* get_last_node_from_id(enignelang_ast* node,
                                          const std::string id) noexcept;

    enignelang_ast* get_last_node_from_name_with_type(enignelang_ast* node,
                                                      const std::string name,
                                                      const
                                                      enignelang_syntax::enignelang_tokens
                                                        data) noexcept;*/
    
    enignelang_ast* wrap_argument(std::vector<enignelang_ast*>& node) noexcept;

    static inline bool is_number(const std::string& str) noexcept {
        if(str.empty()) return false;

        try {
            std::stold(str);
        } catch(std::invalid_argument const& invalid) {
            return false;
        }

        return true;
    }

    enignelang_ast* impl_generic_fn_call(const std::string name,
                              const std::string node_id, 
                              enignelang_syntax::enignelang_tokens syn) noexcept;

    template<typename Name, typename Data>
    void push_constant(Name name, Data data) noexcept {
        this->constants.push_back(enignelang_constant {
            .name = std::basic_string<char>(name),
            .data = std::basic_string<char>(data)
        });

        this->add_pre_variant(std::basic_string<char>(name),
                              std::basic_string<char>(name));
    }

    template<typename Name, typename Data>
    Data get_constant(Name name) noexcept {
        for(auto& val: this->constants) {
            if(val.name == name) {
                return val.data;
            }
        } return "";
    }


    unsigned calculate_same_ids(enignelang_ast* node, enignelang_syntax::enignelang_tokens node_type) noexcept;

    void add_pre_variant(std::string name, std::string constant_name) noexcept {
        enignelang_ast* node = new enignelang_ast();
        //  enignelang_ast* node = new enignelang_ast(this->current[++this->index].token,
        //                                                          "variant_decl",
        //                                                          enignelang_syntax::Variant);
        //
        //                if(this->current[++this->index].token_type == enignelang_syntax::Eq) {
        //                    ++this->index;
        //                    arg_handle.clear();
        //
        //                    node->other.push_back(this->handle_single_argument(arg_handle, node));
        //
        //                    arg_handle.clear();
        //                } else {
        //                    node->other.push_back(nullptr);
        //                }

        node->name = name;
        node->node_id = node->name + "variant_decl";
        node->node_type = enignelang_syntax::Variant;

        for(auto& val : this->constants) {
            if(val.name == name) {
                enignelang_ast* _pre_node = new enignelang_ast("constant",
                                                               "constant",
                                                               enignelang_syntax::Constant);
                _pre_node->node_current = val.data;

                node->other.push_back(std::forward<enignelang_ast*>(_pre_node));
                break;
            }
        }

        this->ast_main->other.push_back(std::forward<enignelang_ast*>(node));
    }

    std::string back() noexcept {
        return this->current_nodes.back().first;
    }

    enignelang_syntax::enignelang_tokens back_type() noexcept {
        return this->current_nodes.back().second;
    }

    bool is_variant(const std::string name) noexcept {
        for(auto&& val: this->ast_main->other) {
            if(val->node_type == enignelang_syntax::Variant &&
               val->name[0] == name[0]) {
                if(val->name == name) {
                    return true;
                }
            }
        }
        return false;
    }

    bool is_fn(const std::string name) noexcept {
        for(long long unsigned i = 0; i < this->ast_main->other.size(); ++i) {
            if(this->ast_main->other[i]->node_type == enignelang_syntax::Function
                && this->ast_main->other[i]->name[0] == name[0]) {
                if(this->ast_main->other[i]->name == name) {
                    return true;
                }
            }
        }

        return false;
    }

    static unsigned sub(unsigned& val) noexcept {
        return (val == 0) ? val : --val;
    }
};

#undef IMPL_CONSTANT