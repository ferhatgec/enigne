// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//

#include "../include/enigne_lang_ast.hpp"

void enignelang_ast::clear() noexcept {
    this->node_id = "global_node";
    this->node_l = nullptr;
    this->node_r = nullptr;
    this->other.clear();
    this->node_current = nullptr;
}