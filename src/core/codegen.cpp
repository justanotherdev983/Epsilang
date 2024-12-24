#include "core/codegen.hpp"

#include <fstream>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const token_type_e type) {
    switch (type) {
    case token_type_e::type_exit: return os << "type_exit";
    case token_type_e::type_int_lit: return os << "type_int_lit";
    case token_type_e::type_semi: return os << "type_semi";
    case token_type_e::type_space: return os << "type_space";
    case token_type_e::type_EOF: return os << "type_EOF";
    default: return os << "unknown_token_type";
    }
}


void gen_code_for_ast(const std::vector<ast_node_t>& ast, std::ofstream &asm_file)
{
    asm_file << "global _start" << std::endl;
    asm_file << "_start:" << std::endl;

    for (const auto& node : ast) {
        gen_node_code(node, asm_file);
    }
    asm_file << "    syscall" << std::endl;
}

void gen_node_code(const ast_node_t &node, std::ofstream &asm_file)
{
    switch (node.type)
    {
    case token_type_e::type_exit:
        std::cout << "Encountered exit token, writing to output asm file" << std::endl;
        asm_file << "    mov rax, 60" << std::endl;
        asm_file << "    mov rdi, 0" << std::endl;

        break;
    case token_type_e::type_int_lit:
        std::cout << "Encountered int_lit token, writing to output asm file" << std::endl;
        std::cout << "int value: " << node.int_value << std::endl;
        asm_file << "    mov rdi, " << node.int_value << std::endl;
        break;

    case token_type_e::type_semi:
        std::cout << "Encountered semi token, writing to output asm file" << std::endl;
        asm_file << "    ; Semicolon encountered" << std::endl;
        break;
    case token_type_e::type_space:
    case token_type_e::type_EOF:
        std::cout << "Encountered space/EOF token, continue-ing" << std::endl;
        break;
    default:
        std::cout << "Encountered unexpected token" << std::endl;
    }

    if (node.child_node_1)
    {
        std::cout << "Encountered child node 1, recalling function" << std::endl;
        gen_node_code(*node.child_node_1, asm_file);
    }
    if (node.child_node_2)
    {
        std::cout << "Encountered child node 2, recalling function" << std::endl;
        gen_node_code(*node.child_node_2, asm_file);
    }
}