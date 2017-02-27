#ifndef instructions_hpp
#define instructions_hpp
#include "interpreter.hpp"

// load_immediate(cell) -- cell
void load_immediate(RuntimeMachine *meta);

// compile_word(size, instructions*, return) -- Procedure
void compile_procedure(RuntimeMachine *meta);

// create_empty_object -- Object
void create_empty_object(RuntimeMachine *meta);

// value key object set_object_attribute -- object
void set_object_attribute(RuntimeMachine *meta);

// key object set_object_attribute -- object
void get_object_attribute(RuntimeMachine *meta);

// int int add_int32 -- int
void add_int32(RuntimeMachine *meta);

// exit_program --
void exit_program(RuntimeMachine *meta);

// return_from_function -- 
void return_from_function(RuntimeMachine *meta);

// procedure execute_stack_procedure -- 
void execute_stack_procedure(RuntimeMachine *meta);

// key dynamic_execute_method -> self.key()
//void dynamic_execute_method(RuntimeMachine *meta);

std::string instructionAsString(Instruction i);
#endif
