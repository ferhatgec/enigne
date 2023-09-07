// MIT License
//
// Copyright (c) 2022-2023 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_syntax.hpp"
#include "../libs/escafe/include/escafe.hpp"

enignelang_syntax::enignelang_tokens enignelang_syntax::match(std::string val) noexcept {
    auto result = this->tokens.find(val);
    if(result == this->tokens.end())
        return enignelang_syntax::enignelang_tokens::Undefined;

    return static_cast<enignelang_syntax::enignelang_tokens>(result->second);
}

void enignelang_syntax::add_file_data(const std::string file) noexcept {
    this->raw_file_data = file;
}



void enignelang_syntax::tokenize() noexcept {
    std::string current_token;
    bool escape_sequence = false,
         data = false,
         skip_until_newline = false;

    long long unsigned row = 1, column = 1;

    for(long long unsigned i = 0; i < this->raw_file_data.length(); i++) {
        if(this->raw_file_data[i] == '\n') {
            ++row;
            column = 0;
        } else {
            ++column;
        }


        if(this->raw_file_data[i] != '\n' && skip_until_newline)
            continue;

        switch(this->raw_file_data[i]) {
            case '\n': {
                if(skip_until_newline)
                    skip_until_newline = false;
                //else
                //    current_token.push_back('\n');

                break;
            }

            case '(':
            case ')':
            case '[':
            case ']':
            case ';':
            case ',':
            case '+':
            case '-':
            case '/':
            case '*':
            case '=':
            case ':':
            case '^':
            case '%':
            case '$':
            case '@':
            case '!':
            case '~':
            case '.':
            case ' ':{
                if(data)
                    current_token.push_back(this->raw_file_data[i]);
                else {
                    if(this->raw_file_data[i] == '.' 
                        && !current_token.empty() 
                        && (std::find_if(current_token.begin(), current_token.end(), [](unsigned char ch) { return !std::isdigit(ch); }) == current_token.end())) {
                        current_token.push_back('.');
                        continue;
                    }

                    if((this->raw_file_data[i] == '-' &&
                        this->raw_file_data[i + 1] == '>') ||
                        (this->raw_file_data[i] == '=' &&
                                (this->raw_file_data[i + 1] == '=' || this->raw_file_data[i + 1] == '!')) ||
                                (this->raw_file_data[i] == '~' && this->raw_file_data[i + 1] == '>')) {
                        this->unparsed_tokens.push_back(
                                std::make_pair(std::string(1, this->raw_file_data[i])
                                               + std::string(1, this->raw_file_data[i + 1]),
                                               std::make_pair(row + 1, column)));

                        ++i;
                        current_token.clear();
                        break;
                    }

                    this->unparsed_tokens.push_back(std::make_pair(current_token,
                                                                   std::make_pair(row, column)));

                    //if(this->raw_file_data[i] != ' ')
                    this->unparsed_tokens.push_back(
                            std::make_pair(std::basic_string<char>(1, this->raw_file_data[i]),
                                    std::make_pair(row, column + 1)));
                    current_token.clear();
                }

                break;
            }

            case '\'':
            case '"': {
                if(escape_sequence) {
                    data = true;
                    current_token.push_back(this->raw_file_data[i]);
                } else {
                    if(current_token.front() != this->raw_file_data[i]) {
                        data = true;
                        current_token.push_back(this->raw_file_data[i]);
                        break;
                    }

                    current_token.push_back(this->raw_file_data[i]);
                    data = false;
                    this->unparsed_tokens.push_back(std::make_pair(escafe::run(current_token),
                                                                   std::make_pair(row, column)));
                    current_token.clear();
                    escape_sequence = false;
                }

                break;
            }

            case '\\': {
                if(escape_sequence)
                    current_token.push_back('\\');
                else if(data)
                    escape_sequence = true;

                break;
            }

            // possible sequences
            case 'n':
            case 't':
            case 'r':
            case 'b':
            case 'x':
            case '0':
            case '1':
            case 'm':
            case 'w': {
                if(escape_sequence) {
                    current_token.push_back('\\');
                    current_token.push_back(this->raw_file_data[i]);
                    escape_sequence = false;
                } else {
                    current_token.push_back(this->raw_file_data[i]);
                }
                break;
            }

            case '#': {
                if(!escape_sequence && !data)
                    skip_until_newline = true;
                else
                    current_token.push_back('#');

                break;
            }

            default: {
                current_token.push_back(this->raw_file_data[i]);
                break;
            }
        }
    }


    if(!current_token.empty()) {
        std::cout << "error at " << current_token << "\n> there's one more \' or \" needed to close" << '\n';
        std::exit(1);
    }

    for(auto& [first, second] : this->unparsed_tokens) {
        if(first.empty() || first == " ") continue;

        auto token_type = this->match(first);

        if(token_type == enignelang_syntax::True || token_type == enignelang_syntax::False ||
            (token_type == enignelang_syntax::Undefined &&
            ((first.length() > 1 && ((first.front() == '"' && first.back() == '"'))
                                    ||
                                    (first.front() == '\'' && first.back() == '\'')) ||
            (this->is_valid_number(first))))) {
                if(token_type == enignelang_syntax::True) {
                    first = "1";
                } else if(token_type == enignelang_syntax::False) {
                    first = "0";
                }

                this->tokenized_tokens.push_back(enignelang_tokenized_tokens {
                    .token = first,
                    .token_type = enignelang_syntax::Constant,
                    .row = second.first,
                    .column = second.second
                }); continue;
        } else if(token_type == enignelang_syntax::Undefined && first.find(' ') == first.npos) {
            this->tokenized_tokens.push_back(enignelang_tokenized_tokens {
                .token = first,
                .token_type = enignelang_syntax::VariantLit,
                .row = second.first,
                .column = second.second
            }); continue;
        }

        this->tokenized_tokens.push_back(enignelang_tokenized_tokens {
            .token = first,
            .token_type = token_type,
            .row = second.first,
            .column = second.second
        });
    }
}
