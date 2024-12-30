#include <fstream>

#include "core/codegen.hpp"
#include "utils/error.hpp"


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

void gen_binary_op(const ast_node_t &node, std::ofstream &asm_file) {
    gen_node_code(*node.child_node_1, asm_file);
    asm_file << "    push rdi" << std::endl;  // Save left operand on the stack


    gen_node_code(*node.child_node_2, asm_file);
    asm_file << "    pop rax" << std::endl;   // Restore left operand from stack

    switch (node.type) {
    case token_type_e::type_add:
        asm_file << "    add rdi, rax" << std::endl;
        break;
    case token_type_e::type_sub:
        asm_file << "    sub rax, rdi" << std::endl;
        asm_file << "    mov rdi, rax" << std::endl;
        break;
    case token_type_e::type_mul:
        asm_file << "    imul rdi, rax" << std::endl;
        break;
    case token_type_e::type_div:
        asm_file << "    xor rdx, rdx" << std::endl;
        asm_file << "    mov rax, rdi" << std::endl;
        asm_file << "    div rax"      << std::endl;
        asm_file << "    mov rdi, rax" << std::endl;
        break;
    default:
        asm_file << "    ; unknown binary operator" << std::endl;
    }
}

void gen_code_for_ast(const std::vector<ast_node_t>& ast, std::ofstream &asm_file)
{
    asm_file << "format ELF64" << std::endl;
    asm_file << "section '.text' executable" << std::endl << std::endl;
    asm_file << "public _start" << std::endl;
    asm_file << "_start:" << std::endl;

    for (const auto& node : ast) {
        gen_node_code(node, asm_file);
    }
    asm_file << "    syscall" << std::endl;
}

void gen_node_code(const ast_node_t &node, std::ofstream &asm_file) {
    switch (node.type) {
    case token_type_e::type_exit:
        info_msg("Encountered exit token, writing to output asm file");


        if (node.child_node_1)
        {
            gen_node_code(*node.child_node_1, asm_file);
        }
        asm_file << "    mov rax, 60; exit syscall" << std::endl;
        break;

    case token_type_e::type_int_lit:
        // Only emit if not part of an expression
        if (!node.child_node_1 && !node.child_node_2) {
            info_msg("Encountered int_lit token, writing to output asm file");
            asm_file << "    mov rdi, " << node.int_value << std::endl;
        }
        break;

    case token_type_e::type_add:
    case token_type_e::type_sub:
    case token_type_e::type_mul:
    case token_type_e::type_div:
        if (!node.child_node_1 || !node.child_node_2) {
            error_msg("Binary operator missing operands");
            return;
        }
        gen_binary_op(node, asm_file);
        break;

    case token_type_e::type_semi:
        info_msg("Encountered semi token, writing to output asm file");
        asm_file << "    ; Semicolon encountered" << std::endl;
        break;

    case token_type_e::type_space:
    case token_type_e::type_EOF:
        info_msg("Encountered space/EOF token, writing to output asm file");
        break;

    default:
        error_msg("Encountered unknown token type in codegen");
    }
}
