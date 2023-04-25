// MIT License
//
// Copyright (c) 2022-2023 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_parse.hpp"
#include "../libs/idkformat/include/idk.hpp"

void enignelang_parse::parse(enignelang_syntax syn) noexcept {
    // std::string constant_data;
    // enignelang_intptr intptr(nullptr);
    this->current = syn.tokenized_tokens;
    this->current.erase(std::remove_if(this->current.begin(), this->current.end(),
                                       [](const enignelang_syntax::enignelang_tokenized_tokens& elem) -> bool {
        return elem.token == " " || elem.token.empty();
    }), this->current.end());
    this->index = 0;
    this->handle_start(this->ast_main);
}

enignelang_ast* enignelang_parse::impl_generic_fn_call(const std::string name, 
                                            const std::string node_id,
                                            enignelang_syntax::enignelang_tokens syn) noexcept {
    enignelang_ast* node = new enignelang_ast();
    
    node->name = name; node->node_id = node_id; node->node_type = syn;

    for(; this->index < this->current.size();) {
        if(this->current[++this->index].token_type == enignelang_syntax::LeftPr) {
            ++this->index;
            if(this->current[this->index].token_type != enignelang_syntax::RightPr) {
                node->other.push_back(this->handle_single_argument(arg_handle, node));
                arg_handle.clear();
            }
        }

        if(this->current[this->index].token_type == enignelang_syntax::Comma) {
            ++this->index;
            node->other.push_back(this->handle_single_argument(arg_handle, node));
            arg_handle.clear();

            if(this->current[this->index].token_type == enignelang_syntax::RightPr) {
                ++this->index;
                break;
            }
        } else if(this->current[this->index++].token_type == enignelang_syntax::RightPr) {
            break;
        } else {
            --this->index;
        } 
        if(this->current[this->index].token_type == enignelang_syntax::Comma)
            --this->index;
    } return node;
}

enignelang_ast* enignelang_parse::handle_single_argument(std::vector<enignelang_ast*>& arg_handle, 
                                            enignelang_ast* from) noexcept {
    for(; this->index < this->current.size(); ++this->index) {
        auto token = this->current[this->index];
        
        switch(token.token_type) {
            case enignelang_syntax::Ast:
            case enignelang_syntax::Div:
            case enignelang_syntax::Minus:
            case enignelang_syntax::Plus:
            case enignelang_syntax::Mod: {
                if(arg_handle.size() == 0) {
                    // empty queue = <empty> + <sth>
                    // push err or +num
                    break;
                }

                enignelang_ast* node = new enignelang_ast("bin_add",
                                                          "bin_add",
                                                          enignelang_syntax::BinOp);
                node->node_l = new enignelang_ast(
                        arg_handle.back()->name,
                        arg_handle.back()->node_id,
                        arg_handle.back()->node_type
                        );

                node->row = token.row;
                node->column = token.column;
                
                node->node_l = arg_handle.back();
                arg_handle.pop_back();
                node->node_current = token.token;
                arg_handle.push_back(std::forward<enignelang_ast*>(node));
                
                break;
            }

            case enignelang_syntax::NotEqualTo:
            case enignelang_syntax::EqualTo:
            case enignelang_syntax::And:
            case enignelang_syntax::Or:
            case enignelang_syntax::IsIn: {
                if(arg_handle.size() == 0) {
                    break;
                }

                if(from != nullptr && 
                    (from->node_type == enignelang_syntax::If ||
                    from->node_type == enignelang_syntax::Elif ||
                    from->node_type == enignelang_syntax::LoopIf ||
                    from->node_type == enignelang_syntax::LoopElif)) {
                    return (arg_handle.empty()) ? nullptr : arg_handle.back();
                }

                enignelang_ast* node = new enignelang_ast("bin_comp",
                                                          "bin_comp",
                                                          enignelang_syntax::BinComp);
                node->row = token.row;
                node->column = token.column;
                
                node->node_l = new enignelang_ast(
                        arg_handle.back()->name,
                        arg_handle.back()->node_id,
                        arg_handle.back()->node_type
                        );
                node->node_l->node_current = arg_handle.back()->node_current;
                node->node_l = arg_handle.back();
                arg_handle.pop_back();
                node->node_current = token.token;
                arg_handle.push_back(std::forward<enignelang_ast*>(node));

                break;
            }

            case enignelang_syntax::RightPr: {
                if(from != nullptr 
                    && from->node_type == enignelang_syntax::FunctionCall
                    && from->node_id == "inline_function_call") {
                    return nullptr;
                }

                return (arg_handle.empty()) ? nullptr : arg_handle.back();
            }

            case enignelang_syntax::LeftBPr: {
                std::vector<enignelang_ast*> args;

                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        last_arg->node_r = new enignelang_ast(
                              "array",
                              "array",
                              enignelang_syntax::LeftBPr
                                );
                        enignelang_ast* node = new enignelang_ast("array",
                                                          "array",
                                                          enignelang_syntax::LeftBPr);
                        node->row = token.row;
                        node->column = token.column;

                        while(this->check_index() && 
                            (this->current[++this->index].token_type != enignelang_syntax::RightBPr)) {
                            if(this->current[this->index].token_type == enignelang_syntax::LeftBPr) {
                                std::vector<enignelang_ast*> _args;
                                auto val = this->handle_single_argument(_args, node);

                                if(val != nullptr) {
                                    node->other.push_back(val);
                                }

                                if(this->current[this->index].token_type == enignelang_syntax::Sem) {
                                    --this->index; --this->index;
                                }

                                continue;
                            } else if(this->current[this->index - 1].token_type == enignelang_syntax::RightBPr) {
                                --this->index;
                                break;
                            }

                            if(this->current[this->index].token_type == enignelang_syntax::Sem
                                || this->current[this->index].token_type == enignelang_syntax::RightBPr) {
                                break;
                            }

                            node->other.push_back(this->handle_single_argument(args, node));
                        }
                        
                        last_arg->node_r = node;

                        break;
                    }
                }

                if(!arg_handle.empty() && arg_handle.back()->node_type == enignelang_syntax::Comma) {
                    arg_handle.pop_back();
                }
                
                enignelang_ast* node = new enignelang_ast("array",
                                                          "array",
                                                          enignelang_syntax::LeftBPr);
                node->row = token.row;
                node->column = token.column;

                // [5, 3, [5, 2]]
                while(this->check_index() && 
                    (this->current[++this->index].token_type != enignelang_syntax::RightBPr)) {
                    if(this->current[this->index].token_type == enignelang_syntax::LeftBPr) {
                        std::vector<enignelang_ast*> _args;
                        
                        auto val = this->handle_single_argument(_args, node);

                        if(val != nullptr) {
                            node->other.push_back(val);
                        }

                        if(this->current[this->index].token_type == enignelang_syntax::Sem) {
                            --this->index; --this->index; 
                        }

                        continue;
                    } else if(this->current[this->index - 1].token_type == enignelang_syntax::RightBPr) {
                        --this->index;
                        break;
                    }

                    if(this->current[this->index].token_type == enignelang_syntax::Sem
                        || this->current[this->index].token_type == enignelang_syntax::RightBPr) {
                        break;
                    }

                    node->other.push_back(this->handle_single_argument(args, node));
                }
                        

                arg_handle.push_back(std::forward<enignelang_ast*>(node));

                break;
            }

            case enignelang_syntax::RightBPr: {
                return (arg_handle.empty()) ? nullptr : arg_handle.back();
            }

            case enignelang_syntax::Comma: {
                if(from != nullptr
                    && from->node_type == enignelang_syntax::FunctionCall
                    && from->node_id == "inline_function_call") {
                    enignelang_ast* node = new enignelang_ast("argument_separator",
                                                              "argument_separator",
                                                              enignelang_syntax::Comma);
                    node->row = token.row;
                    node->column = token.column;
                    
                    arg_handle.push_back(std::forward<enignelang_ast*>(node));
                    break;
                }

                return (arg_handle.empty()) ? nullptr : arg_handle.back();
            }

            case enignelang_syntax::Sem: {
                return (arg_handle.empty()) ? nullptr : arg_handle.back();
            }

            case enignelang_syntax::Eq: {
                return (arg_handle.empty()) ? nullptr : arg_handle.back();
            }

            case enignelang_syntax::VariantLit: {
                if(this->index + 1 < this->current.size()) {
                    if(this->current[this->index + 1].token_type == enignelang_syntax::LeftPr) {
                        break;
                    } else if(this->current[this->index + 1].token_type == enignelang_syntax::Element) {
                        goto elem;
                    }
                }

                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        last_arg->node_r = new enignelang_ast(
                              token.token,
                              "variant_literal",
                              enignelang_syntax::VariantLit
                                );
                        last_arg->node_r->node_current = token.token;
                        break;
                    }
                }

                if(!arg_handle.empty() && arg_handle.back()->node_type == enignelang_syntax::Comma) {
                    arg_handle.pop_back();
                }

                enignelang_ast* node = new enignelang_ast(token.token,
                                                          "variant_literal",
                                                          enignelang_syntax::VariantLit);
                node->row = token.row;
                node->column = token.column;

                node->node_current = token.token;

                if(token.token.length() > 3 && (token.token == "argc" ||
                    (token.token.substr(0, 4) == "arg_"))) {
                    if(auto val = this->get_constant<std::string, std::string>(token.token);
                        !val.empty()) {
                        node->node_current = val;
                        node->node_id = "constant";
                        node->node_type = enignelang_syntax::Constant;
                    }
                }

                arg_handle.push_back(std::forward<enignelang_ast*>(node));

                if(from != nullptr && from->node_type == enignelang_syntax::Element)
                    return arg_handle.back();

                break;
            }

            case enignelang_syntax::Colon: {
                if(auto previous_token = this->current[++this->index];
                    this->is_fn(previous_token.token)) {
                        if(this->current[++this->index].token_type != enignelang_syntax::LeftPr) {
                            --this->index;
                            std::cout << "error: :function(...) -> expected '('\n";
                            std::exit(1);
                        }

                    enignelang_ast* node = new enignelang_ast(previous_token.token,
                                                              "inline_function_call",
                                                              enignelang_syntax::FunctionCall);
                    node->row = token.row;
                    node->column = token.column;

                    ++this->index;

                    if(this->check_index() && this->current[this->index].token_type != enignelang_syntax::RightPr) {
                        std::vector<enignelang_ast*> __args__;
                        this->handle_single_argument(__args__, node);
                        node->other.insert(node->other.end(), __args__.begin(), __args__.end());
                        __args__.clear();
                        while(this->current[this->index].token_type == enignelang_syntax::Comma) {
                            ++this->index;
                            this->handle_single_argument(__args__, node);
                            node->other.insert(node->other.end(), __args__.begin(), __args__.end());
                            __args__.clear();
                        }
                    }

                    if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        last_arg->node_r = new enignelang_ast(
                              previous_token.token,
                              "inline_function_call",
                              enignelang_syntax::FunctionCall
                                );
                            last_arg->node_r = node;
                            break;
                        }
                    }
                    
                    
                    // from->other.push_back(std::forward<enignelang_ast*>(node));
                    arg_handle.push_back(std::forward<enignelang_ast*>(node));
                }

                break;
            }

            case enignelang_syntax::IsFile:
            case enignelang_syntax::IsDir:
            case enignelang_syntax::IsSymlink:
            case enignelang_syntax::ReadFile:
            case enignelang_syntax::PathExists:
            case enignelang_syntax::Exec:
            case enignelang_syntax::Exit:
            case enignelang_syntax::Length:
            case enignelang_syntax::Absolute:
            case enignelang_syntax::Ceil:
            case enignelang_syntax::Floor:
            case enignelang_syntax::Logarithm:
            case enignelang_syntax::SquareRoot:
            case enignelang_syntax::Pi:
            case enignelang_syntax::Euler:
            case enignelang_syntax::StartsWith:
            case enignelang_syntax::EndsWith:
            case enignelang_syntax::ToUpper:
            case enignelang_syntax::ToLower:
            case enignelang_syntax::CharInput: {
                enignelang_ast* node = new enignelang_ast(token.token,
                                                            "inline_function_call",
                                                             token.token_type);
                node->row = token.row;
                node->column = token.column;

                ++this->index;

                if(this->current[this->index].token_type != enignelang_syntax::LeftPr) {
                    std::cout << "error: function(...) -> expected '('\n";
                    std::exit(1);
                } ++this->index;

                if(this->check_index() && this->current[this->index].token_type != enignelang_syntax::RightPr) {
                    std::vector<enignelang_ast*> __args__;
                    this->handle_single_argument(__args__, node);
                    node->other.insert(node->other.end(), __args__.begin(), __args__.end());
                    __args__.clear();
                    while(this->current[this->index].token_type == enignelang_syntax::Comma) {
                        ++this->index;
                        this->handle_single_argument(__args__, node);
                        node->other.insert(node->other.end(), __args__.begin(), __args__.end());
                        __args__.clear();
                    }
                }

                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        last_arg->node_r = new enignelang_ast(
                            token.token,
                            "inline_function_call",
                            token.token_type);
                        last_arg->node_r = node;
                            
                        break;
                    }
                }
                    
                // from->other.push_back(std::forward<enignelang_ast*>(node));
                arg_handle.push_back(std::forward<enignelang_ast*>(node));
                break;
            }

            case enignelang_syntax::FunctionVariant: {
                if(this->index + 1 < this->current.size()) {
                    if(this->current[this->index + 1].token_type == enignelang_syntax::LeftPr) {
                        break;
                    }
                }

                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        if(auto val = this->current[++this->index]; enignelang_syntax::is_valid_number(val.token)) {
                            last_arg->node_r = new enignelang_ast(
                                val.token,
                              "function_variant_argument", enignelang_syntax::FunctionVariant);
                             last_arg->node_r->node_current = val.token;
                        } else {
                            last_arg->node_r = this->handle_single_argument(arg_handle, last_arg);
                            if(last_arg->node_r == nullptr) {
                                std::cout << idk::format("error found at {A}:{A}\n", val.column, val.row);
                                std::exit(1);
                            } else {
                                std::cout << idk::format("{}\n", val.token);
                            }
                        }

                        break;
                    }
                }

                if(!arg_handle.empty() && arg_handle.back()->node_type == enignelang_syntax::Comma) {
                    arg_handle.pop_back();
                }

                enignelang_ast* node = new enignelang_ast("",
                                                          "function_variant_argument",
                                                          enignelang_syntax::FunctionVariant);
                node->row = token.row;
                node->column = token.column;

                if(auto val = this->current[++this->index]; enignelang_syntax::is_valid_number(val.token)) {
                    node->name = val.token;
                } else {
                    node->other.push_back(this->handle_single_argument(arg_handle, node));
                }

                arg_handle.push_back(std::forward<enignelang_ast*>(node));

                break;
            }


            case enignelang_syntax::Constant: {
                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        last_arg->node_r = new enignelang_ast(
                              "constant",
                              "string_constant",
                              enignelang_syntax::Constant
                                );
                        if(enignelang_syntax::is_valid_number(token.token)) {
                            last_arg->node_r->node_current = std::to_string(
                                enignelang_syntax::return_num(token.token));
                        } else {
                            last_arg->node_r->node_current = token.token;
                        }

                        break;
                    }
                }

                if(!arg_handle.empty() && arg_handle.back()->node_type == enignelang_syntax::Comma) {
                    arg_handle.pop_back();
                }

                enignelang_ast* node = new enignelang_ast("constant",
                                                          "string_constant",
                                                          enignelang_syntax::Constant);
                node->row = token.row;
                node->column = token.column;
                
                if(enignelang_syntax::is_valid_number(token.token)) {
                    node->node_current = std::to_string(
                        enignelang_syntax::return_num(token.token));
                } else {
                    node->node_current = token.token;
                }

                arg_handle.push_back(std::forward<enignelang_ast*>(node));

                if(from != nullptr && from->node_type == enignelang_syntax::Element)
                    return arg_handle.back();

                break;
            }

            case enignelang_syntax::LeftPr: {
                if(this->current[++this->index].token_type == enignelang_syntax::RightPr) {
                    break;
                }

                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        last_arg->node_r = new enignelang_ast(
                              token.token,
                              "variant_literal",
                              enignelang_syntax::VariantLit
                                );
                        std::vector<enignelang_ast*> __args__;
                        last_arg->node_r = this->handle_single_argument(__args__, nullptr);
                        break;
                    }
                }

                if(!arg_handle.empty() && arg_handle.back()->node_type == enignelang_syntax::Comma) {
                    arg_handle.pop_back();
                }

                auto val = this->handle_single_argument(arg_handle, nullptr);
                arg_handle.push_back(std::forward<enignelang_ast*>(val));

                if(from != nullptr && from->node_type == enignelang_syntax::Element)
                    return arg_handle.back();

                break;
            }

            case enignelang_syntax::Element: {
                elem: if(this->check_index() - 1 < 0) { break; }

                if(!arg_handle.empty()) {
                    if(auto& last_arg = arg_handle.back();
                        (last_arg->node_type == enignelang_syntax::BinOp
                        || last_arg->node_type == enignelang_syntax::BinComp)
                        && last_arg->node_r == nullptr) {
                        if(this->check_index() && this->current[++this->index].token_type == enignelang_syntax::Element) {
                            last_arg->node_r = new enignelang_ast(
                                this->current[this->index - 1].token,
                                "element",
                                enignelang_syntax::Element
                            );

                            std::vector<enignelang_ast*> __args__;
                            last_arg->node_r->other.push_back(this->handle_single_argument(__args__, last_arg->node_r));
                        }

                        break;
                    }
                }

                if(const auto val = this->current[this->index]; val.token_type == enignelang_syntax::VariantLit) {
                    if(this->check_index() && this->current[++this->index].token_type == enignelang_syntax::Element) {
                        enignelang_ast* node = new enignelang_ast(
                            val.token,
                            "element",
                            enignelang_syntax::Element
                        );

                        node->row = token.row;
                        node->column = token.column;

                        std::vector<enignelang_ast*> __args__;
                        node->other.push_back(this->handle_single_argument(__args__, node));

                        arg_handle.push_back(std::forward<enignelang_ast*>(node));
                    }

                    break;
                }

                break;
            }

            default: {
                break;
            }
        }
    }

    return nullptr;
}

enignelang_ast* enignelang_parse::wrap_argument(std::vector<enignelang_ast*>& args) noexcept {
    if(args.size() == 1) {
        return args.front();
    } else if(args.empty()) {
        return nullptr;
    }
    
    enignelang_ast* arg = new enignelang_ast("argument", "argument", enignelang_syntax::Argument);
    arg->other = args;
    
    
    return std::move(arg);
}


void enignelang_parse::handle_start(enignelang_ast* __node__) noexcept {
    for(; this->index < this->current.size(); ++this->index) {
        auto token = this->current[this->index];
        
        switch(token.token_type) {
            case enignelang_syntax::Function: {
                if(this->current[++this->index].token_type == enignelang_syntax::Colon) {
                    ++this->index;
                }

                if(this->current[this->index].token.find(' ') == this->current[this->index].token.npos) {
                    enignelang_ast* node = new enignelang_ast(this->current[this->index].token,
                                                          "function_decl",
                                                          enignelang_syntax::Function);
                    node->row = token.row;
                    node->column = token.column;
                    
                    ++this->index;
                    if(this->current[this->index].token_type == enignelang_syntax::Eq) {
                        ++this->index;
                        __node__->other.push_back(std::forward<enignelang_ast*>(node));
                        this->handle_start(__node__->other.back());
                        if(!__node__->other.back()->other.empty()) {
                            if(__node__->other.back()->other.back()->node_type != enignelang_syntax::Return) {
                                enignelang_ast* return_node = new enignelang_ast("return",
                                                                          "return",
                                                                          enignelang_syntax::Return);
                                enignelang_ast* constant_node = new enignelang_ast("",
                                                                          "constant",
                                                                          enignelang_syntax::Constant);
                                constant_node->node_current = "\'\'";
                                return_node->other.push_back(std::forward<enignelang_ast*>(constant_node));

                                __node__->other.back()->other.push_back(std::forward<enignelang_ast*>(return_node));
                            }
                        }
                    } else {
                        // syntax error '=' expected
                    }
                } else {
                    // error
                    // f: .. .. = <body>;
                    //    ^^ ^^ ?
                }

                break;
            }

            // return <expr>;
            // return;
            case enignelang_syntax::Return: {
                enignelang_ast* node = new enignelang_ast(token.token,
                                                          "return_statement",
                                                          enignelang_syntax::Return);
                node->row = token.row;
                node->column = token.column;

                if(this->check_index() && (
                    this->current[this->index + 1].token_type != enignelang_syntax::Sem
                    || (__node__ != nullptr && __node__->node_type == enignelang_syntax::GlobalNode)))
                    node->other.push_back(this->handle_single_argument(arg_handle, node));
                else
                    node->other.push_back(nullptr);
                
                arg_handle.clear();
                __node__->other.push_back(std::forward<enignelang_ast*>(node));

                break;
            }

            case enignelang_syntax::Break: {
                enignelang_ast* node = new enignelang_ast(token.token,
                                                          "break",
                                                          enignelang_syntax::Break);
                node->row = token.row;
                node->column = token.column;

                __node__->other.push_back(std::forward<enignelang_ast*>(node));
                if(this->current[++this->index].token_type != enignelang_syntax::Sem) {
                    std::cout << "error found at break; <-\n";
                    std::exit(1);
                }

                break;
            }

            case enignelang_syntax::FunctionVariant: {
                enignelang_ast* node = new enignelang_ast("",
                                                          "function_variant_argument",
                                                          enignelang_syntax::FunctionVariant);
                node->row = token.row;
                node->column = token.column;

                if(auto val = this->current[this->index++]; enignelang_syntax::is_valid_number(val.token)) {
                    node->name = val.token;
                } else {
                    node->other.push_back(this->handle_single_argument(arg_handle, node));
                }
                __node__->other.push_back(std::forward<enignelang_ast*>(node));

                break;
            }

            // <var_token> <var_name> ? (;) : (= <expr>)
            case enignelang_syntax::Variant: {
                enignelang_ast* node = new enignelang_ast(this->current[++this->index].token,
                                                          "variant_decl",
                                                          enignelang_syntax::Variant);

                node->row = token.row;
                node->column = token.column;

                if(this->current[++this->index].token_type == enignelang_syntax::Eq) {
                    ++this->index;
                    arg_handle.clear();
                    
                    node->other.push_back(this->handle_single_argument(arg_handle, node));

                    arg_handle.clear();
                } else {
                    node->other.push_back(nullptr);
                }


                __node__->other.push_back(
                        std::forward<enignelang_ast*>(node)
                        );
                break;
            }

            // <enum_token> =
            //  Val,
            //  Val2,
            //  Val3
            // ;
            case enignelang_syntax::Enum: {
                if(this->current[++this->index].token_type == enignelang_syntax::Eq) {
                    unsigned count = 0;

                    do {
                        if(this->current[++this->index].token_type == enignelang_syntax::VariantLit
                            || this->current[--this->index].token_type == enignelang_syntax::Comma) {
                            enignelang_ast* node = new enignelang_ast(this->current[this->index].token,
                                                          "variant_decl",
                                                          enignelang_syntax::Variant);
                            
                            enignelang_ast* value = new enignelang_ast(this->current[this->index].token,
                                                          "constant",
                                                          enignelang_syntax::Constant);
                            value->node_current = std::to_string(count);
                                                                                  
                            node->row = token.row;
                            node->column = token.column;
                            node->other.push_back(std::forward<enignelang_ast*>(value));

                            __node__->other.push_back(std::forward<enignelang_ast*>(node));

                            ++count;
                        } else {
                            std::cerr << "syntax error found at " << 
                                this->current[this->index].row << ":" <<
                                this->current[this->index].column << 
                                " because there's no variant name before ',' or no ',' after the name; should be like this:\n" <<
                                " enum =\n" <<
                                "  Name,\n" <<
                                "  OtherName,\n" <<
                                "  AnotherName\n" <<
                                " ; # please consider using this template to use enumerations."; 
                            std::exit(1);
                        }

                        ++this->index;
                    } while(this->check_index() 
                        && this->current[this->index].token_type != enignelang_syntax::Sem);
                } else {
                    std::cerr << "'enum' keyword found at " << 
                                this->current[this->index].row << ":" <<
                                this->current[this->index].column << 
                                "but '=' not found, consider adding this to code base.\n";
                    std::exit(1);
                }


                break;
            }


            // include $"path" # uses current location to find script file.
            // include "path" # uses root location to find script file.
            case enignelang_syntax::Include: {
                if(this->check_index()) {
                    enignelang_ast* node = new enignelang_ast(
                        this->current[this->index].token,
                        "include_script",
                        enignelang_syntax::Include
                    );

                    if(this->current[++this->index].token_type == enignelang_syntax::Constant) 
                        node->name = this->current[this->index].token;
                    else if(this->current[this->index].token_type == enignelang_syntax::FunctionVariant) {
                        if(this->current[++this->index].token_type == enignelang_syntax::Constant) {
                            node->name = this->current[this->index].token;
                        } else {
                            std::cout << "There's no include path after $\n";
                            std::exit(1);
                        }

                        node->node_id = "include_script_src";
                    }

                    __node__->other.push_back(std::forward<enignelang_ast*>(
                        node
                    ));
                }

                break;
            }

            // delete "variant";
            case enignelang_syntax::Delete: {
                if(this->check_index()){ 
                    if(auto val = this->current[++this->index];
                        val.token_type == enignelang_syntax::Constant) {
                        if(this->current[++this->index].token_type != enignelang_syntax::Sem) {
                            std::cout << "expected semicolon after delete \"variant\" but got " 
                                + this->current[this->index].token << '\n';
                            std::exit(1);
                        }

                        enignelang_ast* node = new enignelang_ast(
                            val.token,
                            "delete_variant",
                            enignelang_syntax::Delete
                        );

                        __node__->other.push_back(std::forward<enignelang_ast*>(
                                node
                        ));
                    } else {
                        std::cout << "expected string literal like this: delete \"variant\";\n";
                        std::exit(1);
                    }
                } else {
                    std::cout << "EOF before string literal\n";
                    std::exit(1);
                }

                break;
            }

            case enignelang_syntax::Tilde: {
                ++this->index;

                enignelang_ast* node = new enignelang_ast(this->current[this->index++].token,
                                                          "variant_chng",
                                                          enignelang_syntax::Variant);
                node->row = token.row;
                node->column = token.column;
                
                if(this->current[this->index].token_type == enignelang_syntax::Eq) {
                    ++this->index;
                    node->other.push_back(this->handle_single_argument(arg_handle, node));

                    arg_handle.clear();
                } else {
                    std::cout << "note: ~... does nothing\n";
                    node->other.push_back(nullptr);
                }

                __node__->other.push_back(
                        std::forward<enignelang_ast*>(node)
                        );

                break;
            }

            // :<function_name>(<args>);
            case enignelang_syntax::Colon: {
                if(auto token = this->current[++this->index]; token.token_type != enignelang_syntax::LeftPr) {
                    for(std::size_t val_index = 0; val_index < this->ast_main->other.size(); ++val_index) {
                        if(this->ast_main->other[val_index]->node_type == enignelang_syntax::Function) {
                            if(this->ast_main->other[val_index]->name == token.token) {
                                enignelang_ast* node = new enignelang_ast(token.token,
                                                          "function_call",
                                                          enignelang_syntax::FunctionCall);

                                node->row = token.row;
                                node->column = token.column;

                                for(; this->index < this->current.size();) {
                                    if(this->current[++this->index].token_type == enignelang_syntax::LeftPr) {
                                        ++this->index;

                                        if(this->current[this->index].token_type != enignelang_syntax::RightPr) {
                                            node->other.push_back(this->handle_single_argument(arg_handle, node)); arg_handle.clear();
                                        }
                                    }

                                    if(this->current[this->index].token_type == enignelang_syntax::Comma) {
                                        ++this->index;
                                        node->other.push_back(this->handle_single_argument(arg_handle, node)); arg_handle.clear();

                                        if(this->current[this->index].token_type == enignelang_syntax::RightPr) {
                                            ++this->index;
                                            break;
                                        }
                                    } else if(this->current[this->index++].token_type == enignelang_syntax::RightPr) {
                                        break;
                                    } else {
                                        --this->index;
                                    }
                                }

                                node->index_of_fn = val_index;
                                __node__->other.push_back(std::forward<enignelang_ast*>(node));

                                break;
                            }
                        }
                    }
                } else {
                    // :(....);
                    //  ^ where's the function name?
                }

                break;
            }

            case enignelang_syntax::Sem: {
                if(__node__->node_type == enignelang_syntax::If 
                    || __node__->node_type == enignelang_syntax::Else
                    || __node__->node_type == enignelang_syntax::Elif
                    || __node__->node_type == enignelang_syntax::LoopIf) {
                    return;
                } if(__node__->node_type == enignelang_syntax::Function) {
                    return;
                }

                break;
            }

            // if <expr> =
            //  <body>
            // ; else =
            //  <body>
            // ;
            case enignelang_syntax::If: {
                enignelang_ast* node = new enignelang_ast("if",
                                                          "if_statement",
                                                          enignelang_syntax::If);
                node->row = token.row;
                node->column = token.column;

                ++this->index; this->is_if = true;
                node->node_l = new enignelang_ast("if",
                                                  "[node: left]if_statement",
                                                  enignelang_syntax::BinComp);
                node->node_l = this->handle_single_argument(arg_handle, node);

                node->node_current = this->current[this->index].token;
                ++this->index;
                node->node_r = new enignelang_ast("if",
                                                  "[node: right]if_statement",
                                                  enignelang_syntax::BinComp);
                node->node_r = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                ++this->index;

                this->handle_start(node);

                if(this->check_index() && 
                    this->current[++this->index].token_type == enignelang_syntax::Else) {
                    else_statement: if(this->check_index() &&
                        this->current[++this->index].token_type == enignelang_syntax::Eq) {
                        ++this->index; this->is_if = true;
                        enignelang_ast* else_node = new enignelang_ast("else",
                                                                        "else_statement",
                                                                        enignelang_syntax::Else);
                       
                        this->handle_start(else_node);

                        node->statement_list.push_back(std::forward<enignelang_ast*>(else_node));
                    } else {
                        std::cout << "syntax error found at ; else =\n"
                                     "                            ^^^ where?!!\n";
                        std::exit(1);
                    }
                } else if(this->current[this->index].token_type == enignelang_syntax::Elif) {
                   
                    while(this->current[this->index].token_type == enignelang_syntax::Elif) {
                        enignelang_ast* elif_node = new enignelang_ast("elif",
                                                                    "elif_statement",
                                                                    enignelang_syntax::Elif);
                        ++this->index; this->is_if = true;
                        elif_node->node_l = new enignelang_ast("elif",
                                                      "[node: left]elif_statement",
                                                      enignelang_syntax::BinComp);
                        elif_node->node_l = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                        elif_node->node_current = this->current[this->index].token;
                        ++this->index;
                        elif_node->node_r = new enignelang_ast("elif",
                                                      "[node: right]elif_statement",
                                                      enignelang_syntax::BinComp);
                        elif_node->node_r = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                        ++this->index;

                        this->handle_start(elif_node);
                        node->statement_list.push_back(std::forward<enignelang_ast*>(elif_node));
                       
                        ++this->index;
                    }

                    
                    if(this->index < this->current.size() && 
                        this->current[this->index].token_type == enignelang_syntax::Else) {
                        goto else_statement;
                    }
                } else {
                    --this->index;
                }
               
                __node__->other.push_back(std::forward<enignelang_ast*>(node));
                this->is_if = false;

                break;
            }

            // copying the structure is much better way to implement loopif at this moment
            case enignelang_syntax::LoopIf: {
                enignelang_ast* node = new enignelang_ast("loopif",
                                                          "loopif_statement",
                                                          enignelang_syntax::LoopIf);
                node->row = token.row;
                node->column = token.column;

                ++this->index; this->is_if = true;
                node->node_l = new enignelang_ast("loopif",
                                                  "[node: left]loopif_statement",
                                                  enignelang_syntax::BinComp);

                node->node_l = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                node->node_current = this->current[this->index].token;
                ++this->index;
                node->node_r = new enignelang_ast("loopif",
                                                  "[node: right]loopif_statement",
                                                  enignelang_syntax::BinComp);
                node->node_r = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                ++this->index;

                this->handle_start(node);

                if(this->check_index() && this->current[++this->index].token_type == enignelang_syntax::Else) {
                    loopelse_statement: if(this->index + 1 < this->current.size() &&
                        this->current[++this->index].token_type == enignelang_syntax::Eq) {
                        ++this->index; this->is_if = true;
                        enignelang_ast* else_node = new enignelang_ast("else",
                                                                        "else_statement",
                                                                        enignelang_syntax::Else);
                       
                        this->handle_start(else_node);

                        node->statement_list.push_back(std::forward<enignelang_ast*>(else_node));
                    } else {
                        std::cout << "syntax error found at ; else =\n"
                                     "                            ^^^ where?!!\n";
                        std::exit(1);
                    }
                } else if(this->check_index() && 
                    this->current[this->index].token_type == enignelang_syntax::Elif) {
                  
                    while(this->check_index() && this->current[this->index].token_type == enignelang_syntax::Elif) {
                        enignelang_ast* elif_node = new enignelang_ast("elif",
                                                                    "elif_statement",
                                                                    enignelang_syntax::Elif);
                        ++this->index; this->is_if = true;
                        elif_node->node_l = new enignelang_ast("elif",
                                                      "[node: left]elif_statement",
                                                      enignelang_syntax::BinComp);
                        elif_node->node_l = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                        elif_node->node_current = this->current[this->index].token;
                        ++this->index;
                        elif_node->node_r = new enignelang_ast("elif",
                                                      "[node: right]elif_statement",
                                                      enignelang_syntax::BinComp);
                        elif_node->node_r = this->handle_single_argument(arg_handle, node); arg_handle.clear();
                        ++this->index;

                        this->handle_start(elif_node);

                        node->statement_list.push_back(std::forward<enignelang_ast*>(elif_node));
                        ++this->index;
                    }

                   
                    if(this->index < this->current.size() && 
                        this->current[this->index].token_type == enignelang_syntax::Else) {
                        goto loopelse_statement;
                    }
                } else {
                    --this->index;
                }

                __node__->other.push_back(std::forward<enignelang_ast*>(node));
                this->is_if = false;
                break;
            }

            // <print_token>(<args>);
            case enignelang_syntax::Print: {
                enignelang_ast* node = new enignelang_ast("print",
                                                          "[built-in]function_call",
                                                          enignelang_syntax::Print);    

                node->row = token.row;
                node->column = token.column;
                
                for(; this->index < this->current.size();) {
                    if(this->current[++this->index].token_type == enignelang_syntax::LeftPr) {
                        ++this->index;

                        if(this->current[this->index].token_type != enignelang_syntax::RightPr) {
                            node->other.push_back(this->handle_single_argument(arg_handle, node)); arg_handle.clear();
                        }
                    }

                    if(this->current[this->index].token_type == enignelang_syntax::Comma) {
                        ++this->index;
                        if(this->current[this->index].token_type == enignelang_syntax::RightPr) {
                            std::cout << "error: print(..., expression) -> where is the expression?\n";
                            std::exit(1);
                        }

                        node->other.push_back(this->handle_single_argument(arg_handle, node)); arg_handle.clear();

                        if(this->current[this->index].token_type == enignelang_syntax::RightPr) {
                            ++this->index;
                            break;
                        }
                    } else if(this->current[this->index++].token_type == enignelang_syntax::RightPr) {
                        break;
                    }  else {
                        --this->index;
                    }

                    if(this->current[this->index].token_type == enignelang_syntax::Comma)
                        --this->index;
                }

                __node__->other.push_back(
                        std::forward<enignelang_ast*>(node)
                        );

                break;
            }

            case enignelang_syntax::IsFile:
            case enignelang_syntax::IsDir:
            case enignelang_syntax::IsSymlink:
            case enignelang_syntax::ReadFile:
            case enignelang_syntax::PathExists:
            case enignelang_syntax::Exec:
            case enignelang_syntax::Exit:
            case enignelang_syntax::Length:
            case enignelang_syntax::Absolute:
            case enignelang_syntax::Ceil:
            case enignelang_syntax::Floor:
            case enignelang_syntax::Logarithm:
            case enignelang_syntax::SquareRoot:
            case enignelang_syntax::Pi:
            case enignelang_syntax::Euler:
            case enignelang_syntax::StartsWith:
            case enignelang_syntax::EndsWith:
            case enignelang_syntax::ToUpper:
            case enignelang_syntax::ToLower:
            case enignelang_syntax::CharInput: {
                __node__->other.push_back(
                        std::forward<enignelang_ast*>(this->impl_generic_fn_call(token.token, 
                                                                                "[built-in]function_call", 
                                                                                token.token_type))
                        );
                break;
            }
        }
    }
}

/*unsigned i = 0;

void enignelang_parse::read_ast(enignelang_ast* node) noexcept {
    if(node == nullptr) return;

    for(unsigned x = 0; x < i; x++) { std::cout << " "; }
    if(!node->node_current.empty())
        std::cout << "!value: " << (node->node_current);

    std::cout << "[node " << node->name << "], node_id: [" << node->node_id << "]\n";

    for(auto& x : node->other) {
        for(unsigned x = 0; x < i; x++) { std::cout << " "; }

        if(!x->node_current.empty())
            std::cout << "!value: " << (x->node_current);

        std::cout << "[" << x->name << "], id: [" << x->node_id << "]" << '\n';
        read_ast(x->node_l); read_ast(x->node_r);
        read_ast(x->optional_data);
        read_ast(x); ++i;
    }
}

void enignelang_parse::built_pre_ast(enignelang_ast data) noexcept {}

enignelang_ast* enignelang_parse::get_last(enignelang_ast* node) noexcept {
    if(node != nullptr) {
        if(node->other.size() == 0 && node->node_type != enignelang_syntax::Function)
            return std::forward<enignelang_ast*>(node);

        return this->get_last(node->other.back());
    }

    return std::forward<enignelang_ast*>(node);
}

enignelang_ast* enignelang_parse::get_last_node_from_id(enignelang_ast* node, const std::string id) noexcept {
    if(node != nullptr) {
        if((node->node_id == id)) {
            return std::forward<enignelang_ast*>(node);
        }

        for(auto& val : node->statement_list) {
            if(val->node_id == id)
                return std::forward<enignelang_ast*>(val);

        }


        if(node->other.size() > 0) {
            for(auto& val : node->other) {
                if(val->node_id == id) {
                    return std::forward<enignelang_ast*>(val);
                }

                for(auto& data : val->statement_list) {
                    if(data->node_id == id)
                        return std::forward<enignelang_ast*>(data);
                }
            }

            goto here;
        } else
            return std::forward<enignelang_ast*>(node);
    }


    here: return std::forward<enignelang_ast*>(node);
}

enignelang_ast* enignelang_parse::get_last_node_from_name_with_type(enignelang_ast* node,
                                                                    const std::string name,
                                                                    const
                                                                    enignelang_syntax::enignelang_tokens data) noexcept {
    if(node != nullptr) {
        if((node->name == name) && node->node_type == data) {
            return std::forward<enignelang_ast*>(node);
        }

        for(auto& val : node->statement_list)
            if((val->name == name) && val->node_type == data)
                return std::forward<enignelang_ast*>(val);


        if(node->other.size() > 0) {
            for(auto& val : node->other)
                if((val->name == name) && val->node_type == data)
                    return std::forward<enignelang_ast*>(val);


            goto here;
        } else
            return this->get_last_node_from_name_with_type(node, std::move(name), data);
    }

    here: return std::forward<enignelang_ast*>(node);
}*/

unsigned enignelang_parse::calculate_same_ids(enignelang_ast *node, enignelang_syntax::enignelang_tokens node_type) noexcept {
    unsigned i = 0;
    for(auto& val : node->other)
        if(val->node_type == node_type)
            ++i;

    return i;
}