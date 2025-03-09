#include <fstream>
#include <map>

#include "core/codegen.hpp"
#include "core/tokenise.hpp"
#include "utils/error.hpp"

code_gen_ctx_t::code_gen_ctx_t(std::ofstream& asmFile, std::map<std::string, std::string>& symbolTable,
                               std::map<std::string, ast_node_t*>& functionTable)
    : asm_file(asmFile), symbol_table(symbolTable), function_table(functionTable) {}
  
std::string code_gen_ctx_t::generate_label(const std::string& base_name) {
    static int label_count = 0;
    return base_name + "_" + std::to_string(label_count++);
}

void code_gen_ctx_t::access_variable(const std::string& var_name) {
    if (current_function) {
      // Check if it's a parameter
      for (size_t i = 0; i < current_function->parameters.size(); ++i) {
          if (current_function->parameters[i] == var_name) {
              int offset = -((i + 1) * 8);
              asm_file << "    mov rdi, [rbp" << offset << "]" << std::endl;
              return;
          }
      }

      // Check if it's a local variable
      auto it = current_function->local_symbols.find(var_name);
      if (it != current_function->local_symbols.end()) {
          int offset = -(current_function->parameters.size() + std::stoi(it->second) + 1) * 8;
          asm_file << "    mov rdi, [rbp" << offset << "]" << std::endl;
          return;
      }
    }

    
    // If not local or parameter, assume global
    if (symbol_table.count(var_name) > 0) {
        asm_file << "    mov rdi, [" << symbol_table[var_name] << "]" << std::endl;
    } else {
        error_msg("Undefined variable: {}", var_name);
    }
}

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

void gen_binary_op(const ast_node_t& node, code_gen_ctx_t& ctx) {
    gen_node_code(*node.child_node_1, ctx);
    ctx.asm_file << "    push rdi" << std::endl;  // Save left operand on the stack

    gen_node_code(*node.child_node_2, ctx);
    ctx.asm_file << "    pop rax" << std::endl;  // Restore left operand from stack

    switch (node.type) {
        case token_type_e::type_add:
            ctx.asm_file << "    add rdi, rax" << std::endl;
            break;
        case token_type_e::type_sub:
            ctx.asm_file << "    sub rax, rdi" << std::endl;
            ctx.asm_file << "    mov rdi, rax" << std::endl;
            break;
        case token_type_e::type_mul:
            ctx.asm_file << "    imul rdi, rax" << std::endl;
            break;
        case token_type_e::type_div:
            ctx.asm_file << "    mov rax, rdi" << std::endl;
            ctx.asm_file << "    xor rdx, rdx" << std::endl;
            ctx.asm_file << "    div rsi" << std::endl;
            ctx.asm_file << "    mov rdi, rax" << std::endl;
            break;
        default:
            ctx.asm_file << "    ; unknown binary operator" << std::endl;
    }
}

// Process all nodes in the AST to find variable declarations
void process_variable_declarations(std::vector<ast_node_t>& ast, code_gen_ctx_t& ctx) {
    for (auto& node : ast) {
        process_node_declarations(node, ctx);
    }
}

void process_function_declarations(std::vector<ast_node_t> &ast, code_gen_ctx_t& ctx) {
    for (auto& node : ast) {
        if (node.type == token_type_e::type_fn) {
            ctx.function_table[node.string_value] = &node;
        }
    }
}

void gen_function_code(const ast_node_t& node, code_gen_ctx_t& ctx) {
    // Save the previous current_function
    ast_node_t* previous_function = ctx.current_function;
    
    // Set this as the current function
    ctx.current_function = const_cast<ast_node_t*>(&node);
    
    // Generate function label
    std::string function_name = "func_" + node.string_value;
    ctx.asm_file << function_name << ":" << std::endl;
    
    // Prologue
    ctx.asm_file << "    push rbp" << std::endl;
    ctx.asm_file << "    mov rbp, rsp" << std::endl;
    
    // Allocate space for local variables if needed
    int local_vars_count = node.local_symbols.size();
    if (local_vars_count > 0) {
        ctx.asm_file << "    sub rsp, " << (local_vars_count * 8) << std::endl;
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
        ctx.asm_file << "    mov [rbp" << offset << "], " << reg << std::endl;
    }
    
    // Generate code for function body
    for (const auto& stmt : node.body) {
        gen_node_code(stmt, ctx);
    }
    
    // Epilogue
    ctx.asm_file << "    mov rsp, rbp" << std::endl;
    ctx.asm_file << "    pop rbp" << std::endl;
    ctx.asm_file << "    ret" << std::endl;
    
    // Restore the previous current_function
    ctx.current_function = previous_function;
}

void gen_function_call(const ast_node_t& node, code_gen_ctx_t& ctx) {
    // Save caller-saved registers
    ctx.asm_file << "    push rdi" << std::endl;
    ctx.asm_file << "    push rsi" << std::endl;
    ctx.asm_file << "    push rdx" << std::endl;
    ctx.asm_file << "    push rcx" << std::endl;
    ctx.asm_file << "    push r8" << std::endl;
    ctx.asm_file << "    push r9" << std::endl;
    
    // Calculate and push arguments in reverse order so we can pop them into the right registers
    for (int i = node.arguments.size() - 1; i >= 0; i--) {
        gen_node_code(node.arguments[i], ctx);
        ctx.asm_file << "    push rdi" << std::endl;  // Push each argument result onto the stack
    }
    
    // Pop arguments into appropriate registers in the correct order
    for (size_t i = 0; i < node.arguments.size(); i++) {
        std::string reg;
        switch(i) {
            case 0: reg = "rdi"; break;
            case 1: reg = "rsi"; break;
            case 2: reg = "rdx"; break;
            case 3: reg = "rcx"; break;
            case 4: reg = "r8"; break;
            case 5: reg = "r9"; break;
            default:
                error_msg("More than 6 arguments are not supported yet");
                return;
        }
        
        ctx.asm_file << "    pop " << reg << std::endl;
    }
    
    // Call the function
    std::string function_name = "func_" + node.string_value;
    ctx.asm_file << "    call " << function_name << std::endl;
    
    // Restore caller-saved registers (in reverse order)
    ctx.asm_file << "    pop r9" << std::endl;
    ctx.asm_file << "    pop r8" << std::endl;
    ctx.asm_file << "    pop rcx" << std::endl;
    ctx.asm_file << "    pop rdx" << std::endl;
    ctx.asm_file << "    pop rsi" << std::endl;
    ctx.asm_file << "    pop rdi" << std::endl;
    
    // Function result is in rax, move it to rdi
    ctx.asm_file << "    mov rdi, rax" << std::endl;
}

// Process a single node recursively for variable declarations
void process_node_declarations(ast_node_t& node, code_gen_ctx_t& ctx) {
    if (node.type == token_type_e::type_let && node.child_node_1 &&
            node.child_node_1->type == token_type_e::type_identifier) {
        std::string identifier = node.child_node_1->string_value;
        std::string var_name = "var_" + identifier;
        ctx.symbol_table[identifier] = var_name;
    } else if (node.type == token_type_e::type_if) {
        // Process the condition
        if (node.child_node_1) {
            process_node_declarations(*node.child_node_1, ctx);
        }
        
        // Process the 'then' block
        if (node.child_node_2 && node.child_node_2->type == token_type_e::type_block) {
            for (auto& stmt : node.child_node_2->statements) {
                process_node_declarations(stmt, ctx);
            }
        }

        // Process the 'else' block or 'else-if'
        if (node.child_node_3) {
            if (node.child_node_3->type == token_type_e::type_block) {
                for (auto& stmt : node.child_node_3->statements) {
                    process_node_declarations(stmt, ctx);
                }
            } else if (node.child_node_3->type == token_type_e::type_if) {
                process_node_declarations(*node.child_node_3, ctx);
            }
        }
    } else if (node.type == token_type_e::type_block) {
        // Process all statements in a block
        for (auto& stmt : node.statements) {
            process_node_declarations(stmt, ctx);
        }
    }
    
    // Recursively process child nodes if they exist
    if (node.child_node_1) {
        process_node_declarations(*node.child_node_1, ctx);
    }
    if (node.child_node_2) {
        process_node_declarations(*node.child_node_2, ctx);
    }
    if (node.child_node_3) {
        process_node_declarations(*node.child_node_3, ctx);
    }
}

void gen_code_for_ast(const std::vector<ast_node_t>& ast,
                      std::ofstream& asm_file,
                      std::map<std::string, std::string>& symbol_table) {
    std::map<std::string, ast_node_t*> function_table;
    code_gen_ctx_t ctx(asm_file, symbol_table, function_table);
    
    ctx.asm_file << "format ELF64" << std::endl;
  
    process_variable_declarations(const_cast<std::vector<ast_node_t>&>(ast), ctx);
    process_function_declarations(const_cast<std::vector<ast_node_t>&>(ast), ctx);

    ctx.asm_file << "section '.data' writeable" << std::endl;
    for (const auto& pair : ctx.symbol_table) {
        ctx.asm_file << "    " << pair.second << " dq 0" << std::endl;
        ctx.asm_file << "    " << pair.second << "_len = $ - " << pair.second
                << std::endl;
    }

    ctx.asm_file << "section '.text' executable" << std::endl << std::endl;
  
    // Generate code for functions
    for (const auto& pair : ctx.function_table) {
        gen_function_code(*pair.second, ctx);
        ctx.asm_file << std::endl;
    }
  
    // Generate main code
    ctx.asm_file << "public _start" << std::endl;
    ctx.asm_file << "_start:" << std::endl;
    for (const auto& node : ast) {
        // Skip function definitions in the main code path
        if (node.type != token_type_e::type_fn) {
            gen_node_code(node, ctx);
        }
    }

    ctx.asm_file << "    syscall" << std::endl;
}

void gen_comparison(const ast_node_t& node, code_gen_ctx_t& ctx, const std::string& label_true, const std::string& label_end) {
    // Generate code for left operand
    gen_node_code(*node.child_node_1, ctx);
    ctx.asm_file << "    push rdi" << std::endl;  // Save left operand
    
    // Generate code for right operand
    gen_node_code(*node.child_node_2, ctx);
    ctx.asm_file << "    pop rax" << std::endl;   // Restore left operand
    
    // Compare the values
    ctx.asm_file << "    cmp rax, rdi" << std::endl;
    
    // Perform the appropriate jump based on the comparison type
    switch (node.type) {
        case token_type_e::type_eq:  // Equal
            ctx.asm_file << "    je " << label_true << std::endl;
            break;
        case token_type_e::type_nq:  // Not equal
            ctx.asm_file << "    jne " << label_true << std::endl;
            break;
        case token_type_e::type_ge:  // Greater or equal
            ctx.asm_file << "    jge " << label_true << std::endl;
            break;
        case token_type_e::type_le:  // Less or equal
            ctx.asm_file << "    jle " << label_true << std::endl;
            break;
        default:
            ctx.asm_file << "    ; unknown comparison operator" << std::endl;
            break;
    }
    
    // Jump to end if condition is false
    ctx.asm_file << "    jmp " << label_end << std::endl;
    
    // Label for true condition
    ctx.asm_file << label_true << ":" << std::endl;
}

void gen_if_code(const ast_node_t& node, code_gen_ctx_t& ctx) {
    std::string label_true = ctx.generate_label("if_true");
    std::string label_false = ctx.generate_label("if_false");
    std::string label_end = ctx.generate_label("if_end");
    
    // Generate comparison code
    gen_comparison(*node.child_node_1, ctx, label_true, label_false);
    
    // Generate code for 'then' branch
    if (node.child_node_2 && node.child_node_2->type == token_type_e::type_block) {
        for (const auto& stmt : node.child_node_2->statements) {
            gen_node_code(stmt, ctx);
        }
    }
    
    ctx.asm_file << "    jmp " << label_end << std::endl;
    
    // Label for 'else' branch
    ctx.asm_file << label_false << ":" << std::endl;
    
    // Generate code for 'else' branch if it exists
    if (node.child_node_3) {
        if (node.child_node_3->type == token_type_e::type_block) {
            for (const auto& stmt : node.child_node_3->statements) {
                gen_node_code(stmt, ctx);
            }
        } else if (node.child_node_3->type == token_type_e::type_if) {
            // Handle 'else if'
            gen_if_code(*node.child_node_3, ctx);
        }
    }
    
    // End of if statement
    ctx.asm_file << label_end << ":" << std::endl;
}

void push_var_on_stack(const ast_node_t& node, code_gen_ctx_t& ctx) {
    switch (node.type) {
        case token_type_e::type_let: {
            // Check if we have a valid identifier node
            if (node.child_node_1 &&
                node.child_node_1->type == token_type_e::type_identifier) {
                std::string identifier = node.child_node_1->string_value;

                // Check if the identifier exists in the symbol table
                if (ctx.symbol_table.count(identifier) > 0) {
                    std::string var_name = ctx.symbol_table[identifier];

                    // Generate code for the expression (will put result in rdi)
                    if (node.child_node_2) {
                        gen_node_code(*node.child_node_2, ctx);

                        // Store the result in the named location
                        ctx.asm_file << "    mov [" << var_name << "], rdi" << std::endl;
                        ctx.asm_file << "    ; Variable '" << identifier
                                    << "' assigned value in rdi" << std::endl;
                    }
                } else {
                    error_msg("Variable {}'", identifier, "' not declared");
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

void gen_block_code(const ast_node_t& node, code_gen_ctx_t& ctx) {
    if (node.type == token_type_e::type_block) {
        for (const auto& statement : node.statements) {
            gen_node_code(statement, ctx);
        }
    } else if (node.type == token_type_e::type_if) {
        gen_if_code(node, ctx);
    }
}

void gen_node_code(const ast_node_t& node, code_gen_ctx_t& ctx) {
    switch (node.type) {
        case token_type_e::type_exit:
            info_msg("Encountered exit token, writing to output asm file");

            if (node.child_node_1) {
                gen_node_code(*node.child_node_1, ctx);
            }
            ctx.asm_file << "    mov rax, 60; exit syscall" << std::endl;
            break;

        case token_type_e::type_int_lit:
            // Only emit if not part of an expression
            if (!node.child_node_1 && !node.child_node_2) {
                info_msg("Encountered int_lit token, writing to output asm file");
                ctx.asm_file << "    mov rdi, " << node.int_value << std::endl;
            }
            break;
        case token_type_e::type_let:
            push_var_on_stack(node, ctx);
            break;
        case token_type_e::type_add:
        case token_type_e::type_sub:
        case token_type_e::type_mul:
        case token_type_e::type_div:
            if (!node.child_node_1 || !node.child_node_2) {
                error_msg("Binary operator missing operands");
                return;
            }
            gen_binary_op(node, ctx);
            break;
            
        case token_type_e::type_if:
            gen_if_code(node, ctx);
            break;

        case token_type_e::type_block:
            gen_block_code(node, ctx);
            break;
        case token_type_e::type_fn:
            // Function definitions are handled separately
            break;
            
        case token_type_e::type_call:
            gen_function_call(node, ctx);
            break;
        case token_type_e::type_return:
            if (node.child_node_1) {
                gen_node_code(*node.child_node_1, ctx);
                // Move the result from rdi to rax for return value
                ctx.asm_file << "    mov rax, rdi" << std::endl;
            }
            // Generate function epilogue
            ctx.asm_file << "    mov rsp, rbp" << std::endl;
            ctx.asm_file << "    pop rbp" << std::endl;
            ctx.asm_file << "    ret" << std::endl;
            break;
        case token_type_e::type_semi:
            info_msg("Encountered semi token, writing to output asm file");
            ctx.asm_file << "    ; Semicolon encountered" << std::endl;
            break;
            
        case token_type_e::type_identifier:
            // Use the context's access_variable method
            ctx.access_variable(node.string_value);
            break;

        case token_type_e::type_space:
        case token_type_e::type_EOF:
            info_msg("Encountered space/EOF token, writing to output asm file");
            break;

        default:
            error_msg("Encountered unknown token type in codegen");
    }
}