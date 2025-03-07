#include <fstream>
#include <map>

#include "core/codegen.hpp"
#include "core/tokenise.hpp"
#include "utils/error.hpp"




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


// Process all nodes in the AST to find variable declarations
void process_variable_declarations(std::vector<ast_node_t>& ast) {
    for (auto& node : ast) {
        process_node_declarations(node);
    }
}

// Process a single node recursively for variable declarations
void process_node_declarations(ast_node_t& node) {
    if (node.type == token_type_e::type_let && node.child_node_1 &&
            node.child_node_1->type == token_type_e::type_identifier) {
        std::string identifier = node.child_node_1->string_value;
        std::string var_name = "var_" + identifier;
        symbol_table[identifier] = var_name;
    } else if (node.type == token_type_e::type_if) {
        // Process the condition
        if (node.child_node_1) {
            process_node_declarations(*node.child_node_1);
        }
        
        // Process the 'then' block
        if (node.child_node_2 && node.child_node_2->type == token_type_e::type_block) {
            for (auto& stmt : node.child_node_2->statements) {
                process_node_declarations(stmt);
            }
        }

        // Process the 'else' block or 'else-if'
        if (node.child_node_3) {
            if (node.child_node_3->type == token_type_e::type_block) {
                for (auto& stmt : node.child_node_3->statements) {
                    process_node_declarations(stmt);
                }
            } else if (node.child_node_3->type == token_type_e::type_if) {
                process_node_declarations(*node.child_node_3);
            }
        }
    } else if (node.type == token_type_e::type_block) {
        // Process all statements in a block
        for (auto& stmt : node.statements) {
            process_node_declarations(stmt);
        }
    }
    
    // Recursively process child nodes if they exist
    if (node.child_node_1) {
        process_node_declarations(*node.child_node_1);
    }
    if (node.child_node_2) {
        process_node_declarations(*node.child_node_2);
    }
    if (node.child_node_3) {
        process_node_declarations(*node.child_node_3);
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

void gen_comparison(const ast_node_t& node, std::ofstream& asm_file, const std::string& label_true, const std::string& label_end) {
    // Generate code for left operand
    gen_node_code(*node.child_node_1, asm_file);
    asm_file << "    push rdi" << std::endl;  // Save left operand
    
    // Generate code for right operand
    gen_node_code(*node.child_node_2, asm_file);
    asm_file << "    pop rax" << std::endl;   // Restore left operand
    
    // Compare the values
    asm_file << "    cmp rax, rdi" << std::endl;
    
    // Perform the appropriate jump based on the comparison type
    switch (node.type) {
        case token_type_e::type_eq:  // Equal
            asm_file << "    je " << label_true << std::endl;
            break;
        case token_type_e::type_nq:  // Not equal
            asm_file << "    jne " << label_true << std::endl;
            break;
        case token_type_e::type_ge:  // Greater or equal
            asm_file << "    jge " << label_true << std::endl;
            break;
        case token_type_e::type_le:  // Less or equal
            asm_file << "    jle " << label_true << std::endl;
            break;
        default:
            asm_file << "    ; unknown comparison operator" << std::endl;
            break;
    }
    
    // Jump to end if condition is false
    asm_file << "    jmp " << label_end << std::endl;
    
    // Label for true condition
    asm_file << label_true << ":" << std::endl;
}

void gen_if_code(const ast_node_t& node, std::ofstream& asm_file) {
    static int if_count = 0;
    int current_if = if_count++;
    
    std::string label_true = "if_true_" + std::to_string(current_if);
    std::string label_false = "if_false_" + std::to_string(current_if);
    std::string label_end = "if_end_" + std::to_string(current_if);
    
    // Generate comparison code
    gen_comparison(*node.child_node_1, asm_file, label_true, label_false);
    
    // Generate code for 'then' branch
    if (node.child_node_2 && node.child_node_2->type == token_type_e::type_block) {
        for (const auto& stmt : node.child_node_2->statements) {
            gen_node_code(stmt, asm_file);
        }
    }
    
    asm_file << "    jmp " << label_end << std::endl;
    
    // Label for 'else' branch
    asm_file << label_false << ":" << std::endl;
    
    // Generate code for 'else' branch if it exists
    if (node.child_node_3) {
        if (node.child_node_3->type == token_type_e::type_block) {
            for (const auto& stmt : node.child_node_3->statements) {
                gen_node_code(stmt, asm_file);
            }
        } else if (node.child_node_3->type == token_type_e::type_if) {
            // Handle 'else if'
            gen_if_code(*node.child_node_3, asm_file);
        }
    }
    
    // End of if statement
    asm_file << label_end << ":" << std::endl;
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
          error_msg("Variable {}'" + identifier + "' not declared");
        }
      } else {
        error_msg("Invalid variable declaration: missing identifier");
      }
      break;
    }

    default:
      info_msg("In codegen found other token than let: {}", token_type_to_string(node.type));
  }
}

void gen_block_code(const ast_node_t& node, std::ofstream& asm_file) {
    if (node.type == token_type_e::type_block) {
        for (const auto& statement : node.statements) {
            gen_node_code(statement, asm_file);
        }
    } else if (node.type == token_type_e::type_if) {
        gen_if_code(node, asm_file);
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
      
    case token_type_e::type_if:
      gen_if_code(node, asm_file);
      break;

    case token_type_e::type_block:
      gen_block_code(node, asm_file);
      break;

    case token_type_e::type_semi:
      info_msg("Encountered semi token, writing to output asm file");
      asm_file << "    ; Semicolon encountered" << std::endl;
      break;
      
    case token_type_e::type_identifier:
      // Load variable value into rdi
      if (symbol_table.count(node.string_value) > 0) {
        std::string var_name = symbol_table[node.string_value];
        asm_file << "    mov rdi, [" << var_name << "]" << std::endl;
      } else {
        error_msg("Undefined variable: {}" + node.string_value);
      }
      break;

    case token_type_e::type_space:
    case token_type_e::type_EOF:
      info_msg("Encountered space/EOF token, writing to output asm file");
      break;

    default:
      error_msg("Encountered unknown token type in codegen");
  }
}

