#include "instructions.hpp"
#include "interpreter.hpp"

#include <iostream>
#include <iomanip>

/* core instructions */
void load_immediate(RuntimeMachine *meta)
{
	Cell value = meta->read_byte();
	meta->push_argument(value);
}

void create_empty_object(RuntimeMachine *meta)
{
	Object* obj = meta->create_object();
	meta->push_argument(Cell(obj));
}
void add_int32(RuntimeMachine *meta)
{
	Cell lhand = meta->pop_argument();
	lhand.assert_type(INT32, "add_int32.lhand");

	Cell rhand = meta->pop_argument();
	lhand.assert_type(INT32, "add_int32.rhand");

	int i = lhand.int32 + rhand.int32;
	meta->push_argument( Cell(i) );
}

void compile_procedure(RuntimeMachine *meta)
{
	Cell size_byte = meta->read_byte();
	size_byte.assert_type(INT32, "load_anonymous_procedure.size");

	unsigned int size = static_cast<unsigned int>(size_byte.int32);

	CodeBlock *dest = meta->create_anonymous_procedure(size);

	for (unsigned int i=0; i<size; ++i)
	{
		meta->compile_next_instruction(dest, i);
	}
	meta->push_argument(Cell(dest));
}

void execute_stack_procedure(RuntimeMachine *meta)
{
	Cell code_cell = meta->pop_argument();
	code_cell.assert_type(PROCEDURE, "execute_stack_procedure.code");

	CodeBlock *code = code_cell.procedure;
	if (code->size > 0)
	{
		meta->call_function(meta->current_stack_frame().context, code);
	}
	else
	{
		throw ExecutionOutOfBoundsError(std::string("Attempted execute null function"));
	}
}

void return_from_function(RuntimeMachine *meta)
{
	meta->restore_stack_frame();
}

void exit_program(RuntimeMachine *meta)
{
	meta->halt();
}

void set_object_attribute(RuntimeMachine *meta)
{
	Cell obj_cell = meta->pop_argument();
	obj_cell.assert_type(OBJECT, "set_object_attribute.object");

	Object *obj = obj_cell.object;

	Cell key_cell = meta->pop_argument();
	Cell value_cell = meta->pop_argument();

	obj->setattr(key_cell, value_cell);

	meta->push_argument(Cell(obj));
}

void get_object_attribute(RuntimeMachine *meta)
{
	Cell obj_cell = meta->pop_argument();
	obj_cell.assert_type(OBJECT, "get_object_attribute.object");

	Object *obj = obj_cell.object;

	Cell key_cell = meta->pop_argument();

	Cell value_cell = obj->getattr(key_cell);
	meta->push_argument(value_cell);
}


std::string instructionAsString(Instruction inst)
{
	/* just don't worry about it */
	#define if_instruction(x) if (inst == x) { return std::string(#x); }
	if_instruction(add_int32)
	else if_instruction(load_immediate)
	else if_instruction(exit_program)
	else if_instruction(compile_procedure)
	else if_instruction(return_from_function)
	else if_instruction(execute_stack_procedure)
	else if_instruction(create_empty_object)
	else if_instruction(set_object_attribute)
	else return std::string("unknown_instruction");
	#undef if_instruction
}
