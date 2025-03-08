#include <fstream>
#include <map>

#include "core/codegen.hpp"
#include "core/tokenise.hpp"
#include "utils/error.hpp"




// Global symbol table to store variable names and their memory locations
std::map<std::string, std::string> symbol_table;
int variable_count = 0;

int get_stack_offset(const ast_node_t& func_node, const std::string& var_name) {
    // Find parameter position
    for (size_t i = 0; i < func_node.parameters.size(); i++) {
        if (func_node.parameters[i] == var_name) {
            // Parameters are at [rbp-8], [rbp-16], etc.
            return -((i + 1) * 8);
        }
    }
    
    // Check local variables
    auto it = func_node.local_symbols.find(var_name);
    if (it != func_node.local_symbols.end()) {
        // Local variables start after parameters
        int offset = -(func_node.parameters.size() + std::stoi(it->second) + 1) * 8;
        return offset;
    }
    
    return 0; // Not found (will be a global variable)
}

// Modify how variables are accessed inside functions
void access_variable(const ast_node_t& node, const ast_node_t* current_function, 
                    std::ofstream& asm_file) {
    std::string var_name = node.string_value;
    
    if (current_function) {
        // Check if it's a local variable or parameter
        int offset = get_stack_offset(*current_function, var_name);
        if (offset != 0) {
            // Local variable or parameter
            asm_file << "    mov rdi, [rbp" << offset << "]" << std::endl;
            return;
        }
    }
    
    // Global variable
    if (symbol_table.count(var_name) > 0) {
        std::string global_var_name = symbol_table[var_name];
        asm_file << "    mov rdi, [" << global_var_name << "]" << std::endl;
    } else {
        error_msg("Undefined variable: {}", var_name);
    }
}

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
      asm_file << "    div rsi" << std::endl;
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



std::map<std::string, std::shared_ptr<ast_node_t>> function_table;
void process_function_declarations(std::vector<ast_node_t> &ast) {
    info_msg("Processing function declarations...");
    for (auto& node : ast) {
        info_msg("Node type: {}", token_type_to_string(node.type));
        if (node.type == token_type_e::type_fn) {
            info_msg("Found function: {}", node.string_value);
            function_table[node.string_value] = std::shared_ptr<ast_node_t>(&node, [](ast_node_t*){});
        }
    }
    info_msg("Function table size: {}", function_table.size());
}

void gen_function_code(const ast_node_t& node, std::ofstream& asm_file) {
    // Generate function label
    std::string function_name = "func_" + node.string_value;
    asm_file << function_name << ":" << std::endl;
    
    // Prologue
    asm_file << "    push rbp" << std::endl;
    asm_file << "    mov rbp, rsp" << std::endl;
    
    // Allocate space for local variables if needed
    int local_vars_count = node.local_symbols.size();
    if (local_vars_count > 0) {
        asm_file << "    sub rsp, " << (local_vars_count * 8) << std::endl;
    }
    
    // Store parameters in the stack
    for (size_t i = 0; i < node.parameters.size(); i++) {
        // Parameters are passed in registers: rdi, rsi, rdx, rcx, r8, r9
        std::string reg;
        switch(i) {
            case 0: reg = "rdi"; break;
            case 1: reg = "rsi"; break;
            case 2: reg = "rdx"; break;
            case 3: reg = "rcx"; break;
            case 4: reg = "r8"; break;
            case 5: reg = "r9"; break;
            default:
                error_msg("More than 6 parameters are not supported yet");
                return;
        }
        
        // Store parameter in its stack position
        int offset = -(i + 1) * 8;
        asm_file << "    mov [rbp" << offset << "], " << reg << std::endl;
    }
    
    // Generate code for function body
    for (const auto& stmt : node.body) {
        gen_node_code(stmt, asm_file);
    }
    
    // Epilogue
    asm_file << "    mov rsp, rbp" << std::endl;
    asm_file << "    pop rbp" << std::endl;
    asm_file << "    ret" << std::endl;
}

void gen_function_call(const ast_node_t& node, std::ofstream& asm_file) {
    // Save caller-saved registers
    asm_file << "    push rdi" << std::endl;
    asm_file << "    push rsi" << std::endl;
    asm_file << "    push rdx" << std::endl;
    asm_file << "    push rcx" << std::endl;
    asm_file << "    push r8" << std::endl;
    asm_file << "    push r9" << std::endl;
    
    // Generate code for arguments and move them to appropriate registers
    for (size_t i = 0; i < node.arguments.size(); i++) {
        gen_node_code(node.arguments[i], asm_file);
        
        // Move result from rdi to the appropriate register
        std::string reg;
        switch(i) {
            case 0: reg = "rdi"; break; // First arg stays in rdi
            case 1: reg = "rsi"; break;
            case 2: reg = "rdx"; break;
            case 3: reg = "rcx"; break;
            case 4: reg = "r8"; break;
            case 5: reg = "r9"; break;
            default:
                error_msg("More than 6 arguments are not supported yet");
                return;
        }
        
        if (i > 0) {
            asm_file << "    mov " << reg << ", rdi" << std::endl;
        }
    }
    
    // Call the function
    std::string function_name = "func_" + node.string_value;
    asm_file << "    call " << function_name << std::endl;
    
    // Restore caller-saved registers (in reverse order)
    asm_file << "    pop r9" << std::endl;
    asm_file << "    pop r8" << std::endl;
    asm_file << "    pop rcx" << std::endl;
    asm_file << "    pop rdx" << std::endl;
    asm_file << "    pop rsi" << std::endl;
    asm_file << "    pop rdi" << std::endl;
    
    // Function result is in rax, move it to rdi
    asm_file << "    mov rdi, rax" << std::endl;
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
  
  // Process variable and function declarations
  process_variable_declarations(const_cast<std::vector<ast_node_t>&>(ast));
  process_function_declarations(const_cast<std::vector<ast_node_t>&>(ast));

  asm_file << "section '.data' writeable" << std::endl;
  for (const auto& pair : symbol_table) {
    asm_file << "    " << pair.second << " dq 0" << std::endl;
    asm_file << "    " << pair.second << "_len = $ - " << pair.second
             << std::endl;
  }

  asm_file << "section '.text' executable" << std::endl << std::endl;
  
  // Generate code for functions
  for (const auto& pair : function_table) {
    gen_function_code(*pair.second, asm_file);
    asm_file << std::endl;
  }
  
  // Generate main code
  asm_file << "public _start" << std::endl;
  asm_file << "_start:" << std::endl;
  for (const auto& node : ast) {
    // Skip function definitions in the main code path
    if (node.type != token_type_e::type_fn) {
      gen_node_code(node, asm_file);
    }
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
    case token_type_e::type_fn:
      // Function definitions are handled separately
      break;
      
    case token_type_e::type_call:
      gen_function_call(node, asm_file);
      break;
    case token_type_e::type_return:
            if (node.child_node_1) {
                gen_node_code(*node.child_node_1, asm_file);
                // Move the result from rdi to rax for return value
                asm_file << "    mov rax, rdi" << std::endl;
            }
            // Generate function epilogue
            asm_file << "    mov rsp, rbp" << std::endl;
            asm_file << "    pop rbp" << std::endl;
            asm_file << "    ret" << std::endl;
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

