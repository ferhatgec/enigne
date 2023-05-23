// MIT License
//
// Copyright (c) 2022-2023 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_intptr.hpp"
#include "../include/modules/enigne_lang_chars.hpp"
#include "../include/modules/enigne_lang_fs.hpp"
#include "../include/modules/enigne_lang_system.hpp"
#include "../include/modules/enigne_lang_math.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

const std::string true_str =
        std::to_string(static_cast<long double>(1));

const std::string false_str =
        std::to_string(static_cast<long double>(0));

unsigned index = 0;

void enignelang_intptr::expand(enignelang_ast* node) noexcept {
    if(node == nullptr) return;

    std::cout << "[";

    for(auto& val: node->other) {
        val = this->copy_array_elements(val);

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

std::string enignelang_intptr::handle_expr(enignelang_ast* expr) noexcept {
    if(expr == nullptr) return "";
    
    expr = this->copy_array_elements(expr);

    if(expr->node_type == enignelang_syntax::BinOp) {
        if((expr->node_l != nullptr && expr->node_l->node_type == enignelang_syntax::LeftBPr)
            || (expr->node_r != nullptr && expr->node_r->node_type == enignelang_syntax::LeftBPr)) {
            std::cout << "note: do not try to add constants directly to array\n";
        }

        enignelang_ast* left = expr->node_l, *right = expr->node_r;
        std::size_t left_indx = 0, right_indx = 0;

        // if((left != nullptr && left->node_type == enignelang_syntax::BinOp)) {
        //     this->handle_expr(left);
        // }

        // if((right != nullptr && right->node_type ==enignelang_syntax::BinOp)) {
        //     this->handle_expr(right);
        // }

        if((left != nullptr && (left->node_id == "variant_literal"
                                || left->node_id == "constant"))) {
            for(auto& var: this->global_variants) {
                if(var->name == left->name) {
                    left = var->other.back();

                    break;
                }
                ++left_indx;
            }
        }


        if((right != nullptr && right->node_id == "variant_literal"
                                || left->node_id == "constant")) {
            for(auto& var: this->global_variants) {
                if(var->name == right->name) {
                    right = var->other.back();
                    break;
                }
                ++right_indx;
            }
        }
        
        // array + variant
        if((left != nullptr && left->node_type == enignelang_syntax::LeftBPr)
            && right != nullptr && right->node_type != enignelang_syntax::LeftBPr) {
            for(auto& val: this->global_variants) {
                if(val->name == expr->which_node) {
                    this->global_variants[left_indx]->other.back()->other.push_back(right);
                    
                    if(expr->which_node != this->global_variants[left_indx]->name) {
                        enignelang_ast* copy_node = this->global_variants[left_indx];
                        val->other.back()->other.clear();
                        
                        for(auto& data: copy_node->other.back()->other) {
                            val->other.back()->other.push_back(data);
                        }

                       this->global_variants[left_indx]->other.back()->other.pop_back();
                    }

                    break; 
                }
            }

            expr->which_node.clear();

            return "";
        }
        // variant + array
        else if((right != nullptr && right->node_type == enignelang_syntax::LeftBPr)
            && left != nullptr && left->node_type != enignelang_syntax::LeftBPr) {
            // FIXME: Add array to variant as 
            // string literal or integer depending on variant's type.

            return "";
        }
        // array + array
        else if((left != nullptr && left->node_type == enignelang_syntax::LeftBPr)
            && right != nullptr && right->node_type == enignelang_syntax::LeftBPr) {
            for(auto& val: this->global_variants) {
                if(val->name == expr->which_node) {
                    for(auto& val: right->other)
                        this->global_variants[left_indx]->other.back()->other.push_back(val);
                    
                    if(expr->which_node != this->global_variants[left_indx]->name) {
                        enignelang_ast* copy_node = this->global_variants[left_indx];
                        val->other.back()->other.clear();
                        
                        for(auto& data: copy_node->other.back()->other) {
                            val->other.back()->other.push_back(data);
                        }

                        for(std::size_t _indx = 0; _indx < right->other.size(); ++_indx)                        
                            this->global_variants[left_indx]->other.back()->other.pop_back();
                    }

                    break; 
                }
            }

            expr->which_node.clear();

            return "";
        }

        auto left_val = this->handle_expr(expr->node_l);
        auto right_val = this->handle_expr(expr->node_r);

        if(expr->node_current.empty()) return "";

        switch(expr->node_current.front()) {
            case '+': {
                this->callback_method(enignelang_syntax::Plus, expr);
                return this->add(left_val, right_val);
            }

            case '-': {
                this->callback_method(enignelang_syntax::Minus, expr);
                return this->sub(left_val, right_val);
            }

            case '/': {
                this->callback_method(enignelang_syntax::Div, expr);
                return this->div(left_val, right_val);
            }

            case '*': {
                this->callback_method(enignelang_syntax::Ast, expr);
                return this->mul(left_val, right_val);
            }

            case '%': {
                this->callback_method(enignelang_syntax::Mod, expr);
                return this->mod(left_val, right_val);
            }
        }
    } else if(expr->node_type == enignelang_syntax::BinComp) {
        this->callback_method(expr->node_type, expr);

        auto left_val = this->handle_expr(expr->node_l);
        auto right_val = this->handle_expr(expr->node_r);

        if(expr->node_current.empty()) return "";
        if(expr->node_current == "equal_to") {
            return (left_val == right_val) ? true_str : false_str;
        } else if(expr->node_current == "not_equal_to") {
            return (left_val != right_val) ? true_str : false_str;
        } else if(expr->node_current == "or") {
            return ((left_val == true_str) || (right_val == true_str)) ? true_str : false_str;
        } else if(expr->node_current == "and") {
            return ((left_val == true_str) && (right_val == true_str)) ? true_str : false_str;
        } else if(expr->node_current == "is_in") {
            if(left_val == right_val) {
                return true_str;
            }

            const std::string left_val_hints = this->remove_hints(left_val);


            if(expr->node_r->node_type == enignelang_syntax::LeftBPr) {
                for(auto& val: expr->node_r->other) {
                    if((left_val_hints == this->remove_hints(this->handle_expr(val)))
                        || (expr->node_l == val)) {
                        return true_str;
                    }
                }
            } else if(expr->node_r->node_type == enignelang_syntax::VariantLit) {
                for(auto& val: this->global_variants) {
                    if(val->name == expr->node_r->name) {
                        if(val->other.empty())
                            return false_str;

                        if(val->other.back()->node_type == enignelang_syntax::LeftBPr) {
                            for(auto& data: val->other.back()->other) {
                                if((left_val_hints == this->remove_hints(this->handle_expr(data)))
                                    || (expr->node_l == data)) {
                                    return true_str;
                                }
                            }
                        }

                        return false_str;
                    }
                }
            }


            return false_str;
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
        this->callback_method(expr->node_type, expr);

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
        this->callback_method(expr->node_type, expr);

        std::string __data__ = "[";

        for(auto& val: expr->other) {
            if(val == nullptr) continue;
            
            const std::string data = this->handle_expr(val);
            __data__.append(data + ", ");
        }

        return __data__ + "]";
    } else if(expr->node_type == enignelang_syntax::Element) {
        this->callback_method(expr->node_type, expr);

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
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return false_str;
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::is_file(val)));
        }
    } else if(expr->node_type == enignelang_syntax::IsDir) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return false_str;
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::is_dir(val)));
        }
    } else if(expr->node_type == enignelang_syntax::IsSymlink) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return false_str;
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::is_symlink(val)));
        }
    } else if(expr->node_type == enignelang_syntax::PathExists) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return false_str;
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            return std::to_string(static_cast<long double>(enignelang_fs::path_exists(val)));
        }
    } else if(expr->node_type == enignelang_syntax::ReadFile) {
        this->callback_method(expr->node_type, expr);

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
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "";
        } else {
            auto val = this->remove_hints(this->handle_expr(expr->other[0]));
            
            return enignelang_system::output(val);
        }
    } else if(expr->node_type == enignelang_syntax::Exit) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty())
            std::exit(1);
        
        std::exit(0);
    } else if(expr->node_type == enignelang_syntax::Length) {
        this->callback_method(expr->node_type, expr);

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
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::abs(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Ceil) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::ceil(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Floor) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::floor(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Logarithm) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.size() < 2) {
            return "0";
        } 
        
        return enignelang_math::log(
            this->remove_hints(this->handle_expr(expr->other[0])), this->remove_hints(this->handle_expr(expr->other[1])));
    } else if(expr->node_type == enignelang_syntax::SquareRoot) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "0";
        } 
        
        return enignelang_math::sqrt(
            this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::Pi) {
        this->callback_method(expr->node_type, expr);

        return enignelang_math::pi();    
    } else if(expr->node_type == enignelang_syntax::Euler) {
        this->callback_method(expr->node_type, expr);
        
        return enignelang_math::e();
    } else if(expr->node_type == enignelang_syntax::StartsWith) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::starts_with(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::EndsWith) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::ends_with(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::ToUpper) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::to_upper(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::ToLower) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "";
        } 
        
        return enignelang_chars::to_lower(this->remove_hints(this->handle_expr(expr->other[0])));
    } else if(expr->node_type == enignelang_syntax::CharInput) {
        this->callback_method(expr->node_type, expr);

        return enignelang_system::char_input();
    } else if(expr->node_type == enignelang_syntax::ToString) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return "\"\"";
        }

        return "\"" + this->remove_hints(this->handle_expr(expr->other[0])) + "\"";
    } else if(expr->node_type == enignelang_syntax::ToInt) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return false_str;
        }

        std::string _temp_str = this->remove_hints(this->handle_expr(expr->other[0]));
        _temp_str = std::to_string(static_cast<std::int64_t>(enignelang_syntax::return_num(_temp_str)));

        return std::to_string(enignelang_syntax::return_num(_temp_str));
    } else if(expr->node_type == enignelang_syntax::TypeOf) {
        this->callback_method(expr->node_type, expr);

        if(expr->other.empty()) {
            return false_str; 
        }

        const std::string _result = this->handle_expr(expr->other[0]);

        switch(expr->other[0]->node_type) {
            case enignelang_syntax::Constant: {
                if(!expr->other[0]->node_current.empty() 
                    && (expr->other[0]->node_current.front() == '"' 
                        || expr->other[0]->node_current.front() == '\'')) {
                    return true_str; 
                }
                
                return std::to_string(static_cast<long double>(2));
            }

            case enignelang_syntax::LeftBPr: {
                return std::to_string(static_cast<long double>(3));
            }

            default: {
                if(!_result.empty()) {
                    switch(_result.front()) {
                        case '[':
                            return std::to_string(static_cast<long double>(3));

                        case '"':
                        case '\'':
                            return true_str;

                        default:
                            return std::to_string(static_cast<long double>(2));              
                    }
                }
            }
        }

        return false_str;
    }

    return "";
}

enignelang_ast* enignelang_intptr::handle_var(enignelang_ast* var, 
                                             const std::string& variable_name) noexcept {
    if(var == nullptr) return var;
    
    this->callback_method(var->node_type, var);

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
                    if(!val->other.empty()) { // FIXME: Come here if something wrong 
                        var->node_type = enignelang_syntax::Constant;
                            
                        if(!val->other.empty() && 
                            val->other.back()->node_type == enignelang_syntax::Constant) {
                            var->node_current = val->other.back()->node_current;
                        } else {
                            this->handle_var(val->other.back(), variable_name);
                            var->node_current = this->handle_expr(val->other.back());
                        }
                    } // FIXME: Come here if something wrong 
                    break;
                }
            }
        }
    } else if(var->node_type == enignelang_syntax::Element) {
        var = this->copy_array_elements(var);  
    } return var;
}


enignelang_ast* enignelang_intptr::copy_array_elements(enignelang_ast* node) noexcept {
    if(node->node_type == enignelang_syntax::Element) {
        for(auto& val: this->global_variants) {
            if(val->name == node->name) {
                if(!val->other.empty() && 
                    (val->other.back() != nullptr) && val->other.back()->node_type == enignelang_syntax::LeftBPr) {
                    const std::size_t indx = static_cast<std::size_t>(
                            enignelang_syntax::return_num(this->handle_expr(node->other.back())));

                    if(indx < val->other.back()->other.size()) {
                        return val->other.back()->other[indx];
                    }
                }
            }
        }
    }

    return node;
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
            this->callback_method(node->node_type, node);
            
            for(auto data: node->other) {
                if(data == nullptr) continue;
                
                ret: switch(data->node_type) {
                    case enignelang_syntax::LeftBPr:
                    case enignelang_syntax::Constant:
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
            this->callback_method(node->node_type, node);

            if(this->jump != nullptr && !this->jump->other.empty()) {
                if(auto& last = this->jump->other.back(); last->name == "_return_control_") {
                    last->node_current = this->handle_expr(node->other[0]);
                    last->name = "_return_control_ok_";
                }
            } break;
        }

        case enignelang_syntax::FunctionCall: {
            this->callback_method(node->node_type, node);

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
            this->callback_method(node->node_type, node);

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
            this->callback_method(node->node_type, node);

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
            this->callback_method(node->node_type, node);

            std::string cmd_and_its_parameters = "";

            for(auto& data: node->other) {
                switch(data->node_type) {
                    case enignelang_syntax::Constant: {
                        cmd_and_its_parameters.append(
                            this->remove_hints(data->node_current));
                        break;
                    }

                    case enignelang_syntax::BinOp: {
                        cmd_and_its_parameters.append(this->remove_hints(this->handle_expr(data)));
                        break;
                    }

                    case enignelang_syntax::VariantLit: {
                        for(auto& val: this->global_variants) {
                            if(val->name == data->name) {
                                cmd_and_its_parameters.append(
                                    this->remove_hints(this->handle_expr(val->other.back())));
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
        
            std::system(cmd_and_its_parameters.c_str());

            break;
        }
        

        case enignelang_syntax::Exit: {
            this->callback_method(node->node_type, node);

            if(node->other.empty())
                std::exit(1);
            
            std::exit(0);
        }

        case enignelang_syntax::CharInput: {
            this->callback_method(node->node_type, node);

            ::getchar();
            break;
        }

        case enignelang_syntax::ToString:
        case enignelang_syntax::ToInt:
        case enignelang_syntax::TypeOf: {
            // nothing to do, out of scope. makes no sense
            // TODO: support '-warnings' arg for pushing warning
            // per unused variants, 'makes no sense' function calls etc.
            break;
        }

        case enignelang_syntax::Delete: {
            this->callback_method(node->node_type, node);

            if(node->other.empty())
                break;

            if(node->other.back()->node_type == enignelang_syntax::LeftBPr) {
                for(auto& val: node->other.back()->other) {
                    if(val->node_type != enignelang_syntax::Constant)
                        continue;

                    for(std::size_t i = 0; i < this->global_variants.size(); ++i) {
                        if(this->global_variants[i]->name == this->remove_hints(val->node_current)) {
                            // we don't care order of which variant is before because it's global variant.
                            // but there can be a lower complexity for accessing the elements by keeping their
                            // global variant index in node, so it's TODO. (i am lazy to do this)
                            std::swap(this->global_variants[i], this->global_variants.back());
                            this->global_variants.pop_back();

                            break;
                        }
                    }
                }
            } else if(node->other.back()->node_type == enignelang_syntax::Constant) {
                const std::string __val = this->remove_hints(node->other.back()->node_current);

                for(std::size_t i = 0; i < this->global_variants.size(); ++i) {
                    if(this->global_variants[i]->name == __val) {
                        std::swap(this->global_variants[i], this->global_variants.back());
                        this->global_variants.pop_back();

                        break;
                    }
                }
            }

            break;
        }
        
        case enignelang_syntax::Variant: {
            this->callback_method(node->node_type, node);

            for(auto& data: this->global_variants) {
                if(data->name == node->name) {
                    enignelang_ast* test = node->other.back();
                    test = this->handle_var(test, node->name);
                    test->which_node = data->name;
                    enignelang_ast* _val = new enignelang_ast("=", node->name, enignelang_syntax::Constant);
                    _val->node_current = this->handle_expr(test);
                    
                    if(!test->which_node.empty()) {
                        data->other.clear();
                        data->other.push_back(_val);
                    }
                    
                    return;
                }
            }

            enignelang_ast* variant = new enignelang_ast(node->name,
                                                         "variant",
                                                         enignelang_syntax::Variant);

            if(!node->other.empty() && node->other.back() != nullptr && 
                node->other.back()->node_type == enignelang_syntax::Element) {
                node->other.back() = this->copy_array_elements(node->other.back());
            } else if(!node->other.empty() && node->other.back() != nullptr && 
                node->other.back()->node_type == enignelang_syntax::FunctionVariant) {
                unsigned index = enignelang_syntax::return_num(node->other.back()->name); 
                if(index < this->jump->other.size()) 
                    node->other.back() = this->jump->other[index];
            }

            variant->other.assign(node->other.begin(), node->other.end());

            this->global_variants.push_back(variant);

            break;
        }

        default: {
            break;
        }
    }
}

void enignelang_intptr::include_external_script(enignelang_ast* node) noexcept {
    if(node == nullptr || node->name.empty()) return;

    std::ifstream read_stream(
        ((node->node_id == "include_script_src") ? std::filesystem::current_path().string() : "") 
        + "/" + this->remove_hints(node->name));

    if(!read_stream) return;

    enignelang __main;
    
    for(std::string temp; std::getline(read_stream, temp);
        __main.syntax.raw_file_data.append(temp + "\n"))
        ; read_stream.close();

    __main.syntax.tokenize();
    __main.file(__main.syntax.raw_file_data);
    __main.parser.parse(__main.syntax);
    
    //enignelang_intptr __data(__main.parser.ast_main);
    std::vector<enignelang_ast*> list;

    for(auto& val: __main.parser.ast_main->other) {
        if(val->node_type == enignelang_syntax::Variant 
            || val->node_type == enignelang_syntax::Function
            || val->node_type == enignelang_syntax::Tilde) {
            list.push_back(std::forward<enignelang_ast*>(val));
        }
    }

    enignelang_include_info info;

    info.info = list;
    info.name = node->name;

    include_infos.push_back(info);

    //__data.parser = __main.parser;
}

void enignelang_intptr::start() noexcept {
    for(std::size_t n = 0; n < this->main_structure->other.size(); ++n) {
        if(this->main_structure->other[n]->node_type == enignelang_syntax::Include) {
            this->include_external_script(this->main_structure->other[n]);
        }
    }

    for(auto& info: this->include_infos) {
        this->concatenate_include_infos.insert(this->concatenate_include_infos.end(),
                                                info.info.begin(),
                                                info.info.end());
    }

    this->concatenate_include_infos.insert(this->concatenate_include_infos.end(),
                                            this->main_structure->other.begin(),
                                            this->main_structure->other.end());

    this->main_structure->other.clear();
    this->main_structure->other.swap(this->concatenate_include_infos);

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