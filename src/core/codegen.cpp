#include <fstream>
#include <map>

#include "core/codegen.hpp"
#include "core/tokenise.hpp"
#include "utils/error.hpp"

std::ostream& operator<<(std::ostream& os, const token_type_e type) {
  switch (type) {
    case token_type_e::type_exit:
      return os << "type_exit";
    case token_type_e::type_let:
      return os << "type_let";
    case token_type_e::type_identifier:
      return os << "type_identifier";
    case token_type_e::type_equal:
      return os << "type_equal";
    case token_type_e::type_int_lit:
      return os << "type_int_lit";
    case token_type_e::type_add:
      return os << "type_add";
    case token_type_e::type_sub:
      return os << "type_sub";
    case token_type_e::type_mul:
      return os << "type_mul";
    case token_type_e::type_div:
      return os << "type_div";
    case token_type_e::type_open_paren:
      return os << "type_open_paren";
    case token_type_e::type_close_paren:
      return os << "type_close_paren";
    case token_type_e::type_semi:
      return os << "type_semi";
    case token_type_e::type_space:
      return os << "type_space";
    case token_type_e::type_EOF:
      return os << "type_EOF";
    default:
      return os << "unknown_token_type";
  }
}

// Global symbol table to store variable names and their memory locations
std::map<std::string, std::string> symbol_table;
int variable_count = 0;

void gen_binary_op(const ast_node_t& node, std::ofstream& asm_file) {
  gen_node_code(*node.child_node_1, asm_file);
  asm_file << "    push rdi" << std::endl;  // Save left operand on the stack

  gen_node_code(*node.child_node_2, asm_file);
  asm_file << "    pop rax" << std::endl;  // Restore left operand from stack

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
      asm_file << "    mov rax, rdi" << std::endl;
      asm_file << "    xor rdx, rdx" << std::endl;
      asm_file << "    div rax" << std::endl;
      asm_file << "    mov rdi, rax" << std::endl;
      break;
    default:
      asm_file << "    ; unknown binary operator" << std::endl;
  }
}

void gen_code_for_ast(const std::vector<ast_node_t>& ast,
                      std::ofstream& asm_file) {
  asm_file << "format ELF64" << std::endl;

  // Process variable declarations to populate the symbol table
  process_variable_declarations(const_cast<std::vector<ast_node_t>&>(ast));

  asm_file << "section '.data' writeable" << std::endl;

  // Declare variables in the data section
  for (const auto& pair : symbol_table) {
    asm_file << "    " << pair.second << " dq 0" << std::endl;
    asm_file << "    " << pair.second << "_len = $ - " << pair.second
             << std::endl;
  }

  asm_file << "section '.text' executable" << std::endl << std::endl;
  asm_file << "public _start" << std::endl;
  asm_file << "_start:" << std::endl;

  // Generate code for all nodes
  for (const auto& node : ast) {
    gen_node_code(node, asm_file);
  }

  asm_file << "    syscall" << std::endl;
}


void push_var_on_stack(const ast_node_t& node, std::ofstream& asm_file) {
  switch (node.type) {
    case token_type_e::type_let: {
      // Check if we have a valid identifier node
      if (node.child_node_1 &&
          node.child_node_1->type == token_type_e::type_identifier) {
        std::string identifier = node.child_node_1->string_value;

        // Check if the identifier exists in the symbol table
        if (symbol_table.count(identifier) > 0) {
          std::string var_name = symbol_table[identifier];

          // Generate code for the expression (will put result in rdi)
          if (node.child_node_2) {
            gen_node_code(*node.child_node_2, asm_file);

            // Store the result in the named location
            asm_file << "    mov [" << var_name << "], rdi" << std::endl;
            asm_file << "    ; Variable '" << identifier
                     << "' assigned value in rdi" << std::endl;
          }
        } else {
          error_msg("Variable '" + identifier + "' not declared");
        }
      } else {
        error_msg("Invalid variable declaration: missing identifier");
      }
      break;
    }

    default:
      info_msg("In codegen found other token than let:");
      std::cout << node.type;
  }
}

void gen_node_code(const ast_node_t& node, std::ofstream& asm_file) {
  switch (node.type) {
    case token_type_e::type_exit:
      info_msg("Encountered exit token, writing to output asm file");

      if (node.child_node_1) {
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
    case token_type_e::type_let:
      push_var_on_stack(node, asm_file);
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
    case token_type_e::type_identifier:
      info_msg("Found identifier");
      break;

    case token_type_e::type_space:
    case token_type_e::type_EOF:
      info_msg("Encountered space/EOF token, writing to output asm file");
      break;

    default:
      error_msg("Encountered unknown token type in codegen");
  }
}

void process_variable_declarations(std::vector<ast_node_t>& ast) {
  for (auto& node : ast) {
    if (node.type == token_type_e::type_let && node.child_node_1 &&
        node.child_node_1->type == token_type_e::type_identifier) {
      std::string identifier = node.child_node_1->string_value;
      std::string var_name = "var_" + identifier;
      symbol_table[identifier] = var_name;
    }
  }
}