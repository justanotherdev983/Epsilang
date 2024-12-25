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

void gen_node_code(const ast_node_t &node, std::ofstream &asm_file) {
    switch (node.type) {
    case token_type_e::type_exit:
        std::cout << "Encountered exit token, writing to output asm file" << std::endl;
        asm_file << "    mov rax, 60" << std::endl; // syscall: exit

        break;

    case token_type_e::type_int_lit:
        // Only emit if not part of an expression
        if (!node.child_node_1 && !node.child_node_2) {
            std::cout << "Encountered int_lit token, writing to output asm file" << std::endl;
            asm_file << "    mov rdi, " << node.int_value << std::endl;
        }
        break;

    case token_type_e::type_add:
        std::cout << "Encountered add token, writing to output asm file" << std::endl;
        asm_file << "    mov rdx, " << node.child_node_1->int_value << std::endl;
        asm_file << "    add rdx, " << node.child_node_2->int_value << std::endl;
        asm_file << "    mov rdi, rdx" << std::endl;
        break;

    case token_type_e::type_sub:
        std::cout << "Encountered sub token, writing to output asm file" << std::endl;
        asm_file << "    mov rdx, " << node.child_node_1->int_value << std::endl;
        asm_file << "    sub rdx, " << node.child_node_2->int_value << std::endl;
        asm_file << "    mov rdi, rdx" << std::endl;
        break;

    case token_type_e::type_mul:
        std::cout << "Encountered mul token, writing to output asm file" << std::endl;
        asm_file << "    mov rdx, " << node.child_node_1->int_value << std::endl;
        asm_file << "    imul rdx, " << node.child_node_2->int_value << std::endl;
        asm_file << "    mov rdi, rdx" << std::endl;
        break;

    case token_type_e::type_div:
        std::cout << "Encountered div token, writing to output asm file" << std::endl;

        // Save rax (no problem)
        asm_file << "    push rax" << std::endl;
        asm_file << "    mov rax, " << node.child_node_1->int_value << std::endl;
        asm_file << "    xor rdx, rdx" << std::endl;
        asm_file << "    mov rdi, " << node.child_node_2->int_value << std::endl;
        asm_file << "    div rdi" << std::endl;
        asm_file << "    mov rdi, rax" << std::endl;
        asm_file << "    pop rax" << std::endl;
        break;

    case token_type_e::type_semi:
        std::cout << "Encountered semi token, writing to output asm file" << std::endl;
        asm_file << "    ; Semicolon encountered" << std::endl;
        break;

    case token_type_e::type_space:
    case token_type_e::type_EOF:
        std::cout << "Encountered space/EOF token, continuing" << std::endl;
        break;

    default:
        std::cerr << "Error: Unexpected token type during code generation!" << std::endl;
    }
}
