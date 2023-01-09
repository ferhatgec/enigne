// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_intptr.hpp"
#include "../include/enigne_lang_syntax.hpp"
#include "../include/modules/enigne_lang_chars.hpp"
#include "../include/modules/enigne_lang_fs.hpp"
#include "../include/modules/enigne_lang_system.hpp"
#include "../include/modules/enigne_lang_math.hpp"
#include <iostream>
#include <any>
#include <ranges>
#include <cmath>

enignelang_ast* prev;

unsigned index = 0;
std::string current_data;

void enignelang_intptr::expand(enignelang_ast* node) noexcept {
    if(node == nullptr) return;

    std::cout << "[";
    for(auto& val: node->other) {
        if(val->node_type == enignelang_syntax::LeftBPr) {
            this->expand(val);
        } else if(val->node_type == enignelang_syntax::Constant) {
            std::cout << val->node_current + ", ";
        } else if(val->node_type == enignelang_syntax::BinOp 
            || val->node_type == enignelang_syntax::BinComp) {
            std::cout << this->handle_expr(val) + ", ";        
        } else if(val->node_type == enignelang_syntax::FunctionCall) {
            std::cout << this->handle_expr(val) + ", ";
        }
    }
    std::cout << "]";
}

std::string enignelang_intptr::handle_expr(enignelang_ast *expr) noexcept {
    if(expr == nullptr) return "";

    if(expr->node_type == enignelang_syntax::BinOp) {
        if((expr->node_l != nullptr && expr->node_l->node_type == enignelang_syntax::LeftBPr)
            || (expr->node_r != nullptr && expr->node_r->node_type == enignelang_syntax::LeftBPr)) {
            std::cout << "note: do not try to add constants directly to array\n";
        }

        auto left_val = this->handle_expr(expr->node_l);
        auto right_val = this->handle_expr(expr->node_r);

        if(expr->node_current.empty()) return "";

        switch(expr->node_current.front()) {
            case '+': {
                // ya tam sayi ise ne yapacaksin lmao
                return this->add(left_val, right_val);
            }

            case '-': {
                return this->sub(left_val, right_val);
            }

            case '/': {
                return this->div(left_val, right_val);
            }

            case '*': {
                return this->mul(left_val, right_val);
            }

            case '%': {
                return this->mod(left_val, right_val);
            }
        }
    } else if(expr->node_type == enignelang_syntax::BinComp) {
        auto left_val = this->handle_expr(expr->node_l);
        auto right_val = this->handle_expr(expr->node_r);

        if(expr->node_current.empty()) return "";

        if(expr->node_current == "equal_to") {
            return (left_val == right_val) ? std::to_string(static_cast<long double>(1)) : "0";
        } else if(expr->node_current == "not_equal_to") {
            return (left_val != right_val) ? std::to_string(static_cast<long double>(1)) : "0";
        }
    } else if(expr->node_type == enignelang_syntax::Constant) {
        return expr->node_current;
    } else if(expr->node_type == enignelang_syntax::VariantLit) {
        for(auto& val: this->global_variants) {
            if(val->name == expr->name) {
                if(!val->other.empty()) {
                    if(val->other[0]->node_type == enignelang_syntax::Constant) {
                        return val->other[0]->node_current;
                    } else {
                        return this->handle_expr(val->other[0]);
                    }
                }
                break;
            }
        }
    } else if(expr->node_type == enignelang_syntax::FunctionVariant) {
        if(!expr->name.empty())
            if(unsigned index = enignelang_syntax::return_num(expr->name); index < this->jump->other.size()) {
                auto val = this->jump->other[index];
            
                if(val->node_type == enignelang_syntax::Argument && index < val->other.size()) {
                    return this->handle_expr(val->other[index]);
                }

                return this->handle_expr(val);
            } else {
                return "";
            }
        else {
            // $(0 + 2)
        }
    } else if(expr->node_type == enignelang_syntax::FunctionCall) {
        for(auto& Val : expr->other) {
            if(Val == nullptr) continue;
            if(Val->node_type == enignelang_syntax::Argument) {
                if(Val->node_current.empty()) {
                    for(auto& var : Val->other) {
                        if(var == nullptr) continue;
                    }
                }
            } else if(Val->node_type == enignelang_syntax::FunctionCall) {
                auto callback = [this, Val](enignelang_ast* node) {
                    if(this->parser.is_fn(node->name)) {
                        for(auto val : this->main_structure->other) {
                            if(val->node_type == enignelang_syntax::Function &&
                                val->name == node->name) {
                                this->jump = new enignelang_ast();
                                this->jump->other.assign(Val->other.begin(), Val->other.end());
                                
                                for(auto block: val->other) {
                                    if(block->node_type == enignelang_syntax::Return) {        
                                        Val->node_type = enignelang_syntax::Constant;

                                        if(block->other.empty())
                                            Val->node_current = "";
                                        else
                                            Val->node_current = this->handle_expr(block->other[0]);
                                        
                                        break;
                                    }

                                    this->walk(block, val, block->node_type, {});
                                } break;
                            }
                        }
                    }

                    // for(auto& cb: Val->other)
                    //  ...
                }; callback(Val);
            } 
        }

        for(auto& val: this->main_structure->other) {
            if(val == nullptr) continue;
            if(val->node_type == enignelang_syntax::Function && val->name == expr->name) {
                this->jump = new enignelang_ast();
                this->jump->other.assign(expr->other.begin(), expr->other.end());
                boolean _return_control_ = false;
                enignelang_ast* last = nullptr;

                for(auto block: val->other) {
                    if(block == nullptr) continue;
                    if(block->node_type == enignelang_syntax::Return) {
                        if(!block->other.empty())
                            return this->handle_expr(block->other[0]);
                                        
                        break;
                    } else if(block->node_type == enignelang_syntax::If) {
                        if(!this->jump->other.empty()) {                   
                            enignelang_ast* add = new enignelang_ast();
                            add->name = "_return_control_";
                            add->node_current = "1";
                            last = add;
                            this->jump->other.push_back(std::forward<enignelang_ast*>(add));
                        }
                    }

                    walk(block, expr, block->node_type, {});

                    if(last != nullptr && last->name == "_return_control_ok_") {
                        return last->node_current;
                    }
                } break;
            }
        }
    } else if(expr->node_type == enignelang_syntax::LeftBPr) {
        std::string __data__ = "[";

        for(auto& val: expr->other) {
            if(val == nullptr) continue;
            
            const std::string data = this->handle_expr(val);
            __data__.append(data + ", ");
        }

        return __data__ + "]";
    } else if(expr->node_type == enignelang_syntax::Element) {
        for(auto& val: this->global_variants) {
            if(val->name == expr->name) {
                if(val->other.empty()) return "";

                if(auto __val__ = val->other.front(); __val__->node_type == enignelang_syntax::LeftBPr) {
                    std::string index = "";
                    
                    if(expr->other.empty()) index = "0";
                    else index = this->handle_expr(expr->other[0]);

                    if(auto __index__ = static_cast<std::size_t>(enignelang_syntax::return_num(index));
                        __index__ < __val__->other.size()) {
                        return this->handle_expr(__val__->other[__index__]);
                    }
                } else if(__val__->node_type == enignelang_syntax::Constant) {
                    std::string index = "";
                    
                    if(expr->other.empty()) index = "0";
                    else index = this->handle_expr(expr->other[0]);

                    if(auto __index__ = static_cast<std::size_t>(enignelang_syntax::return_num(index));
                        __index__ < __val__->node_current.length()) {
                        return "\"" + std::string(1, __val__->node_current[__index__]) + "\"";
                    } // else {
                      //  // out of range   
                      // }
                }
            }
        }
    } if(expr->node_type == enignelang_syntax::IsFile) {
        if(expr->other.empty()) {
            return "0";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::is_file(val)));
        }
    } else if(expr->node_type == enignelang_syntax::IsDir) {
        if(expr->other.empty()) {
            return "0";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::is_dir(val)));
        }
    } else if(expr->node_type == enignelang_syntax::IsSymlink) {
        if(expr->other.empty()) {
            return "0";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::is_symlink(val)));
        }
    } else if(expr->node_type == enignelang_syntax::PathExists) {
        if(expr->other.empty()) {
            return "0";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::path_exists(val)));
        }
    } else if(expr->node_type == enignelang_syntax::ReadFile) {
        if(expr->other.empty()) {
            return "";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            
            if(enignelang_fs::is_file(val)) {
                return enignelang_fs::read_file(val);
            } else {
                return "";
            }
        }
    } else if(expr->node_type == enignelang_syntax::Exec) {
        if(expr->other.empty()) {
            return "";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            
            return enignelang_system::output(val);
        }
    } else if(expr->node_type == enignelang_syntax::Exit) {
        if(expr->other.empty())
            std::exit(1);
        
        std::exit(0);
    } else if(expr->node_type == enignelang_syntax::Length) {
        if(expr->other.empty()) {
            return "0";
        } else {
            if(auto __node_type = expr->other[0]; __node_type->node_type == enignelang_syntax::VariantLit) {
                for(auto& var: this->global_variants) {
                    if(var->name == __node_type->name)
                        if(var->other.empty()) 
                            return "0";
                        else if(var->other[0]->node_type == enignelang_syntax::LeftBPr)
                            return std::to_string(static_cast<long double>(var->other[0]->other.size()));
                }
            } else if(__node_type->node_type == enignelang_syntax::LeftBPr) {
                return std::to_string(static_cast<long double>(__node_type->other.size()));
            }

            auto val = this->handle_expr(expr->other[0]);
            
            if(enignelang_syntax::is_valid_number(val)) {
                val = std::to_string(enignelang_syntax::return_num(val));
                val = val.erase(val.find_last_not_of('0') + 1, std::string::npos);
                
                if(val.back() == '.')
                    val.pop_back();
            }

            return std::to_string(static_cast<long double>(this->remove_hints(val).length()));
        }
    } else if(expr->node_type == enignelang_syntax::Absolute) {
        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::abs(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Ceil) {
        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::ceil(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Floor) {
        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::floor(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Logarithm) {
        if(expr->other.size() < 2) {
            return "0";
        } 
        
        return enignelang_math::log(
            this->remove_hints(this->handle_expr(expr->other[0])), this->remove_hints(this->handle_expr(expr->other[1])));
    } else if(expr->node_type == enignelang_syntax::SquareRoot) {
        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::sqrt(
            this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Pi) {
        return enignelang_math::pi();    
    } else if(expr->node_type == enignelang_syntax::Euler) {
        return enignelang_math::e();
    } else if(expr->node_type == enignelang_syntax::StartsWith) {
        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::starts_with(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::EndsWith) {
        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::ends_with(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::ToUpper) {
        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::to_upper(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::ToLower) {
        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::to_lower(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::CharInput) {
        return enignelang_system::char_input();
    }

    return "";
}

enignelang_ast* enignelang_intptr::handle_var(enignelang_ast* var, 
                                             const std::string& variable_name) noexcept {
    if(var == nullptr) return nullptr;
    
    if(var->node_type == enignelang_syntax::BinOp) {
        auto left_val = this->handle_var(var->node_l, variable_name);
        auto right_val = this->handle_var(var->node_r, variable_name);
    } else if(var->node_type == enignelang_syntax::BinComp) {
        auto left_val = this->handle_var(var->node_l, variable_name);
        auto right_val = this->handle_var(var->node_r, variable_name);
    } else if(var->name == variable_name || var->node_type == enignelang_syntax::VariantLit) {
        if(var->name == variable_name) {
            for(auto& val: this->global_variants) {
                if(val->name == variable_name) {
                   // if(!val->other.empty()) {
                        var->node_type = enignelang_syntax::Constant;
                            
                        if(!val->other.empty() && 
                            val->other.back()->node_type == enignelang_syntax::Constant) {
                            var->node_current = val->other.back()->node_current;
                        } else {
                            this->handle_var(val->other.back(), variable_name);
                            var->node_current = this->handle_expr(val->other.back());
                        }
                    //} 
                    break;
                }
            }
        }
    } return var;
}


std::string enignelang_intptr::add(const std::string &left, const std::string &right) noexcept {
    if(left.empty()) return right;
    if(right.empty()) return left;

    if(enignelang_syntax::is_valid_number(left)) {
        if(enignelang_syntax::is_valid_number(right)) {
            std::string temp = std::to_string(enignelang_syntax::return_num(left) 
                + enignelang_syntax::return_num(right));
            
            if(!temp.empty()) {
                return temp;
            } else
                return "0";
        } else if(!right.empty()) {
            if(const char data = right.front(); data == '\'' || data == '"') {
                return std::string(1, data) + right + right.substr(1, right.size() - 1); 
            }

            // num + string
        }
    }  else {
        if(!enignelang_syntax::is_valid_number(right)) {
            // string + string
            return left.substr(0, left.size() - 1) + right.substr(1, right.size() - 1);
        } else if(!left.empty()) {
            if(const char data = left.back(); data == '\'' || data == '"') {
                return left.substr(0, left.size() - 1) + right + std::string(1, data); 
            }

            // string + num
        }
    }

    return "";
}

std::string enignelang_intptr::sub(const std::string &left, const std::string &right) noexcept {
    if(left.empty()) return right;
    if(right.empty()) return left;

    if(enignelang_syntax::is_valid_number(left)) {
        if(enignelang_syntax::is_valid_number(right)) {
            std::string temp = "";

            if(long double val = enignelang_syntax::return_num(left) - enignelang_syntax::return_num(right);
                val < 0) {
                temp = std::to_string(val);
            } else {
                temp = std::to_string(val);
            }

            if(!temp.empty())
                return temp;
            else
                return "0";
        } else {
            // num + string
        }
    }  else {
        if(!enignelang_syntax::is_valid_number(right)) {
            // string + string
            return left + right;
        } else {
            // string + num
        }
    }

    return "";
}

std::string enignelang_intptr::div(const std::string &left, const std::string &right) noexcept {
    if(left.empty()) return right;
    if(right.empty()) return left;

    if(enignelang_syntax::is_valid_number(left)) {
        if(auto val = enignelang_syntax::return_num(right);
            enignelang_syntax::is_valid_number(right)) {
            return std::to_string(enignelang_syntax::return_num(left) / ((val != 0) ? val : -1));
        } else {
            // num + string
        }
    }  else {
        if(!enignelang_syntax::is_valid_number(right)) {
            // string + string
            return left + right;
        } else {
            // string + num
        }
    }

    return "";
}

std::string enignelang_intptr::mul(const std::string &left, const std::string &right) noexcept {
    if(left.empty()) return right;
    if(right.empty()) return left;

    if(enignelang_syntax::is_valid_number(left)) {
        if(enignelang_syntax::is_valid_number(right)) {
            return std::to_string(enignelang_syntax::return_num(left) * enignelang_syntax::return_num(right));
        } else {
            // num + string
        }
    }  else {
        if(!enignelang_syntax::is_valid_number(right)) {
            // string + string
            return left + right;
        } else {
            // string + num
        }
    }

    return "";
}

std::string enignelang_intptr::mod(const std::string &left, const std::string &right) noexcept {
    if(left.empty()) return right;
    if(right.empty()) return left;
    
    if(enignelang_syntax::is_valid_number(left)) {
        if(enignelang_syntax::is_valid_number(right)) {
            return std::to_string(std::fmod(enignelang_syntax::return_num(left), enignelang_syntax::return_num(right)));
        } else {
            // num + string
        }
    }  else {
        if(!enignelang_syntax::is_valid_number(right)) {
            // string + string
            return std::to_string(left.length() % right.length());
        } else {
            return std::to_string(std::fmod(left.length(), enignelang_syntax::return_num(right)));
        }
    }

    return "";
}

void enignelang_intptr::walk(enignelang_ast* node,
                             enignelang_ast* from,
                             enignelang_syntax::enignelang_tokens type,
                             std::vector<std::string> func_args) noexcept {
    if(node == nullptr) return;

    switch(node->node_type) {
        case enignelang_syntax::Print: {
            for(auto data: node->other) {
                if(data == nullptr) continue;
                ret: switch(data->node_type) {
                    case enignelang_syntax::Constant: {
                        if(!data->node_current.empty() && ((data->node_current.front() == '\'' && data->node_current.back() == '\'')
                            || (data->node_current.front() == '"' && data->node_current.back() == '"'))) {
                            data->node_current = data->node_current.substr(1, 
                                                      data->node_current.length() - 2);
                        }

                        std::cout << this->remove_hints(data->node_current);
                        break;
                    }

                    case enignelang_syntax::BinComp:
                    case enignelang_syntax::BinOp: {
                        std::cout << this->remove_hints(this->handle_expr(data));
                        break;
                    }

                    case enignelang_syntax::FunctionVariant: {
                        if(!data->name.empty()) {
                            if(auto index = enignelang_syntax::return_num(data->name); index < from->other.size()) {
                                data->node_current = from->other[index]->node_current;
                            } else {
                                data->node_current = "";
                            }
                            
                            data->node_type = enignelang_syntax::Constant;
                            goto ret;
                        } else {
                            // $(5 + 2 - 3 + ...)
                        }

                        break;
                    }

                    // array
                    case enignelang_syntax::LeftBPr: {
                        this->expand(data);
                        break;
                    }

                    case enignelang_syntax::VariantLit: {
                         for(auto& val: this->global_variants) {
                            if(val->name == data->name) {                                
                                if(!val->other.empty()) {
                                    if(auto& __val__ = val->other[0]; __val__->node_type == enignelang_syntax::Constant) {
                                        std::cout << this->remove_hints(__val__->node_current);
                                    } else if(__val__->node_type == enignelang_syntax::LeftBPr) {
                                        this->expand(__val__);    
                                    } else {
                                        std::cout << this->remove_hints(this->handle_expr(__val__));
                                    }
                                }
                                break;
                            }
                        }

                        break;
                    }

                    case enignelang_syntax::FunctionCall: {
                        for(auto& Val : data->other) {
                            if(Val == nullptr) continue;
                            if(Val->node_type == enignelang_syntax::Argument) {
                                if(Val->node_current.empty()) {
                                    for(auto& var : Val->other) {
                                        if(var == nullptr) continue;
                                    }
                                }
                            } else if(Val->node_type == enignelang_syntax::FunctionCall) {
                                auto callback = [this, Val](enignelang_ast* node) {
                                    if(this->parser.is_fn(node->name)) {
                                        for(auto val : this->main_structure->other) {
                                            if(val->node_type == enignelang_syntax::Function &&
                                                val->name == node->name) {
                                                this->jump = new enignelang_ast();
                                                this->jump->other.assign(Val->other.begin(), Val->other.end());
                                
                                                for(auto block: val->other) {
                                                    if(block->node_type == enignelang_syntax::Return) {        
                                                        Val->node_type = enignelang_syntax::Constant;

                                                        if(block->other.empty())
                                                            Val->node_current = "";
                                                        else
                                                            Val->node_current = this->handle_expr(block->other[0]);
                                                        break;
                                                    }

                                                    this->walk(block, val, block->node_type, {});
                                                }
                                                break;
                                            }
                                        }
                                    }

                                    //for(auto& cb: Val->other)
                                    // ...
                                };

                                callback(Val);
                            } 
                        }

                        for(auto& val: this->main_structure->other) {
                            if(val == nullptr) continue;
                            if(val->node_type == enignelang_syntax::Function && val->name == data->name) {
                                this->jump = new enignelang_ast();
                                this->jump->other.assign(data->other.begin(), data->other.end());
                                boolean _return_control_ = false;
                                enignelang_ast* last = nullptr;

                                for(auto block: val->other) {
                                    if(block == nullptr) continue;
                                    if(block->node_type == enignelang_syntax::Return) {
                                        if(!block->other.empty())
                                            std::cout << this->remove_hints(this->handle_expr(block->other[0]));
                                        
                                        break;
                                    } else if(block->node_type == enignelang_syntax::If) {
                                        if(!this->jump->other.empty()) {
                                            enignelang_ast* add = new enignelang_ast();
                                            add->name = "_return_control_";
                                            add->node_current = "1";
                                            last = add;
                                            this->jump->other.push_back(std::forward<enignelang_ast*>(add));
                                        }
                                    }

                                    walk(block, data, block->node_type, {});

                                    if(last != nullptr && last->name == "_return_control_ok_") {
                                        std::cout << this->remove_hints(last->node_current);
                                        break;
                                    }
                                }

                                break;
                            }
                        }

                        break;
                    }


                    default: {
                        if(!data->other.empty() || data->node_type >= enignelang_syntax::PathExists)
                            std::cout << this->remove_hints(this->handle_expr(data));
                        break;
                    }
                }
            }

            break;
        }

        case enignelang_syntax::Return: {
            if(this->jump != nullptr && !this->jump->other.empty()) {
                if(auto& last = this->jump->other.back(); last->name == "_return_control_") {
                    last->node_current = this->handle_expr(node->other[0]);
                    last->name = "_return_control_ok_";
                }
            } break;
        }

        case enignelang_syntax::FunctionCall: {
            for(auto& Val : node->other) {
                if(Val == nullptr) continue;
                if(Val->node_type == enignelang_syntax::Argument) {
                    if(Val->node_current.empty()) {
                        for(auto& var : Val->other) {
                            if(var == nullptr) continue;
                        }
                    }
                } else if(Val->node_type == enignelang_syntax::FunctionCall) {
                    auto callback = [this, Val](enignelang_ast* node) {
                        if(this->parser.is_fn(node->name)) {
                            for(auto val : this->main_structure->other) {
                                if(val->node_type == enignelang_syntax::Function &&
                                    val->name == node->name) {
                                    this->jump = new enignelang_ast();
                                    this->jump->other.assign(Val->other.begin(), Val->other.end());
                                    
                                    for(auto block: val->other) {
                                        if(block->node_type == enignelang_syntax::Return) {        
                                            Val->node_type = enignelang_syntax::Constant;
                                            
                                            if(block->other.empty())
                                                Val->node_current = "";
                                            else
                                                Val->node_current = this->handle_expr(block->other[0]);
                                            break;
                                        } this->walk(block, val, block->node_type, {});
                                    } break;
                                }
                            }
                        }

                        //for(auto& cb: Val->other)
                        // ...
                    }; callback(Val);
                } 
            }

                        
            if(node->index_of_fn < this->main_structure->other.size()) {
                if(auto fn = this->main_structure->other[node->index_of_fn];
                    fn != nullptr &&
                    fn->node_type == enignelang_syntax::Function && (fn->name == node->name)) {
                    
                    this->jump = new enignelang_ast();
                    this->jump->other.assign(node->other.begin(), node->other.end());
                    
                    for(auto& block: fn->other) {
                        if(block == nullptr) continue;
                        if(block->node_type == enignelang_syntax::Return) { return; }
                        if(block->node_type == enignelang_syntax::FunctionCall && block->name == node->name) {
                            if(++this->recursion_fn_call == 255) {
                                std::cout << "note: recursion limit exceed\n";
                                this->recursion_fn_call = 0;
                                return;
                            }
                        }

                        walk(block, node, block->node_type, node->func_args);
                    }
                }
            }
            break;
        }

        case enignelang_syntax::If: {
            // this->jump = new enignelang_ast();
            // for(auto& val : from->other) {
            //     std::cout << val->node_id << '\n';
            // }
            // this->jump->other.assign(from->other.begin(), from->other.end());
            
            if(node->node_current == "equal_to") {
                if(this->handle_expr(node->node_l)
                     == this->handle_expr(node->node_r)) {
                    
                    for(auto& val: node->other) {
                        if(val != nullptr && val->node_type == enignelang_syntax::Break) {
                            return;
                        }

                        walk(val, node, val->node_type, val->func_args);
                    }
                } else if(!node->statement_list.empty()) {
                    for(auto& else_elif_node: node->statement_list) {
                        if(else_elif_node->node_type == enignelang_syntax::Elif) {
                            if(else_elif_node->node_current == "equal_to") {
                                
                                if(this->handle_expr(else_elif_node->node_l)
                                    == this->handle_expr(else_elif_node->node_r)) {
                                    for(auto& val: else_elif_node->other) {
                                        if(val->node_type == enignelang_syntax::Break) {
                                            return;
                                        }
                                        walk(val, else_elif_node, val->node_type, val->func_args);
                                    }

                                    break;
                                }
                            } else if(else_elif_node->node_current == "not_equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    != this->handle_expr(else_elif_node->node_r)) {
                                    for(auto& val: else_elif_node->other) {
                                        if(val->node_type == enignelang_syntax::Break) {
                                            return;
                                        }
                                        walk(val, else_elif_node, val->node_type, val->func_args);
                                    }

                                    break;
                                }
                            }
                        } else if(else_elif_node->node_type == enignelang_syntax::Else) {
                            for(auto& val: else_elif_node->other) {
                                if(val->node_type == enignelang_syntax::Break) {
                                    return;
                                }

                                walk(val, else_elif_node, val->node_type, val->func_args);
                            }
                        }
                    }
                }
            } else if(node->node_current == "not_equal_to") {
                if(this->handle_expr(node->node_l) 
                    != this->handle_expr(node->node_r)) {
                    for(auto& val: node->other) {
                        if(val->node_type == enignelang_syntax::Break) {
                            return;
                        }
                        walk(val, node, val->node_type, val->func_args);
                    }
                } else if(!node->statement_list.empty()) {
                    for(auto& else_elif_node: node->statement_list) {
                        if(else_elif_node->node_type == enignelang_syntax::Elif) {
                            if(else_elif_node->node_current == "equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    == this->handle_expr(else_elif_node->node_r)) {
                                    for(auto& val: else_elif_node->other) {
                                        if(val->node_type == enignelang_syntax::Break) {
                                            return;
                                        }
                                        walk(val, else_elif_node, val->node_type, val->func_args);
                                    }

                                    break;
                                }
                            } else if(else_elif_node->node_current == "not_equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    != this->handle_expr(else_elif_node->node_r)) {
                                    for(auto& val: else_elif_node->other) {
                                        if(val->node_type == enignelang_syntax::Break) {
                                            return;
                                        }

                                        walk(val, else_elif_node, val->node_type, val->func_args);
                                    }

                                    break;
                                }
                            }
                        } else if(else_elif_node->node_type == enignelang_syntax::Else) {
                            for(auto& val: else_elif_node->other) {
                                if(val->node_type == enignelang_syntax::Break) {
                                    return;
                                }
                                walk(val, else_elif_node, val->node_type, val->func_args);
                            }
                        }
                    }
                }
            }

            break;
        }

        case enignelang_syntax::LoopIf: {
            this->jump = new enignelang_ast();
            this->jump->other.assign(from->other.begin(), from->other.end());
            if(node->node_current == "equal_to") {
                if(this->handle_expr(node->node_l)
                     == this->handle_expr(node->node_r)) {
                    while(this->handle_expr(node->node_l)
                        == this->handle_expr(node->node_r)) {
                        for(auto& val: node->other) {
                            if(val->node_type == enignelang_syntax::Break) {
                                return;
                            }

                            walk(val, node, val->node_type, val->func_args);
                        }
                    }
                } else if(!node->statement_list.empty()) {
                    for(auto& else_elif_node: node->statement_list) {
                        if(else_elif_node->node_type == enignelang_syntax::Elif) {
                            if(else_elif_node->node_current == "equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    == this->handle_expr(else_elif_node->node_r)) {
                                    
                                    while(this->handle_expr(else_elif_node->node_l)
                                    == this->handle_expr(else_elif_node->node_r)) {
                                        for(auto& val: else_elif_node->other) {
                                            if(val->node_type == enignelang_syntax::Break) {
                                                return;
                                            }

                                            walk(val, else_elif_node, val->node_type, val->func_args);
                                        }
                                    }

                                    break;
                                }
                            } else if(else_elif_node->node_current == "not_equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    != this->handle_expr(else_elif_node->node_r)) {
                                    while(this->handle_expr(else_elif_node->node_l)
                                    != this->handle_expr(else_elif_node->node_r)) {
                                        for(auto& val: else_elif_node->other) {
                                            if(val->node_type == enignelang_syntax::Break) {
                                                return;
                                            }
                                            
                                            walk(val, else_elif_node, val->node_type, val->func_args);
                                        }
                                    }

                                    break;
                                }
                            }
                        } else if(else_elif_node->node_type == enignelang_syntax::Else) {
                            while(true) {
                                for(auto& val: else_elif_node->other) {
                                    if(val->node_type == enignelang_syntax::Break) {
                                        return;
                                    }

                                    walk(val, else_elif_node, val->node_type, val->func_args);
                                }
                            }
                        }
                    }
                }
            } else if(node->node_current == "not_equal_to") {
                if(this->handle_expr(node->node_l) 
                    != this->handle_expr(node->node_r)) {
                    while(this->handle_expr(node->node_l) 
                    != this->handle_expr(node->node_r)) {
                        for(auto& val: node->other) {
                            if(val->node_type == enignelang_syntax::Break) {
                                return;
                            }

                            walk(val, node, val->node_type, val->func_args);
                        }
                    }
                } else if(!node->statement_list.empty()) {
                    // TODO: implement else if
                    for(auto& else_elif_node: node->statement_list) {
                        if(else_elif_node->node_type == enignelang_syntax::Elif) {
                            if(else_elif_node->node_current == "equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    == this->handle_expr(else_elif_node->node_r)) {
                                    while(this->handle_expr(else_elif_node->node_l)
                                    == this->handle_expr(else_elif_node->node_r)) {
                                        for(auto& val: else_elif_node->other) {
                                            if(val->node_type == enignelang_syntax::Break) {
                                                return;
                                            }

                                            walk(val, else_elif_node, val->node_type, val->func_args);
                                        }
                                    }

                                    break;
                                }
                            } else if(else_elif_node->node_current == "not_equal_to") {
                                if(this->handle_expr(else_elif_node->node_l)
                                    != this->handle_expr(else_elif_node->node_r)) {
                                    while(this->handle_expr(else_elif_node->node_l)
                                    != this->handle_expr(else_elif_node->node_r)) {
                                        for(auto& val: else_elif_node->other) {
                                            if(val->node_type == enignelang_syntax::Break) {
                                                return;
                                            }
                                            walk(val, else_elif_node, val->node_type, val->func_args);
                                        }
                                    }

                                    break;
                                }
                            }
                        } else if(else_elif_node->node_type == enignelang_syntax::Else) {
                            while(true) {
                                for(auto& val: else_elif_node->other) {
                                    if(val->node_type == enignelang_syntax::Break) {
                                        return;
                                    }
                                    walk(val, else_elif_node, val->node_type, val->func_args);
                                }
                            }
                        }
                    }
                }
            }

            break;
        }

        case enignelang_syntax::Exec: {
            for(auto& data: node->other) {
                switch(data->node_type) {
                    case enignelang_syntax::Constant: {
                        std::system(data->node_current.c_str());
                        break;
                    }

                    case enignelang_syntax::BinOp: {
                        std::system(this->handle_expr(data).c_str());
                        break;
                    }

                    case enignelang_syntax::VariantLit: {
                        for(auto& val: this->global_variants) {
                            if(val->name == data->name) {
                                std::system(this->remove_hints(this->handle_expr(val->other.back())).c_str());
                                break;
                            }
                        }
                        break;
                    }

                    default: {
                        break;
                    }
                }
            }

            break;
        }

        case enignelang_syntax::Exit: {
            if(node->other.empty())
                std::exit(1);
            
            std::exit(0);
        }

        case enignelang_syntax::CharInput: {
            ::getchar();
            break;
        }
        
        case enignelang_syntax::Variant: {
            for(auto& data: this->global_variants) {
                if(data->name == node->name) {
                    enignelang_ast* test = node->other.back();

                    test = this->handle_var(test, node->name);
                    enignelang_ast* _val = new enignelang_ast("=", node->name, enignelang_syntax::Constant);
                    _val->node_current = this->handle_expr(test);
                    data->other.clear();
                    data->other.push_back(_val);
                    
                    return;
                }
            }


            enignelang_ast* variant = new enignelang_ast(node->name,
                                                         "variant",
                                                         enignelang_syntax::Variant);
            variant->other.assign(node->other.begin(), node->other.end());

            this->global_variants.push_back(variant);

            break;
        }

        default: {
            break;
        }
    }
}

void enignelang_intptr::start() noexcept {
    for(; index < this->main_structure->other.size(); ++index) {
        if(auto val = this->main_structure->other[index]->node_type; val == enignelang_syntax::Function) {
            continue;
        } else if(val == enignelang_syntax::Return) {
            return;
        }

        this->walk(this->main_structure->other[index], 
                    this->main_structure->other[::abs(((index == 0) ? 0 : index - 1))], 
                    this->main_structure->other[index]->node_type,
                    this->main_structure->other[index]->func_args);
    }
}