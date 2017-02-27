#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <stdexcept>
#include <sstream>
#include <list>

#include <cstring>
#include <cstdlib>

#include "interpreter.hpp"

#ifndef NULL
#define NULL ((void*)0)
#endif

extern std::string instructionAsString(Instruction i);

CellTypeException::CellTypeException(std::string msg) : std::runtime_error(msg) {}
ExecutionOutOfBoundsError::ExecutionOutOfBoundsError(std::string msg) : std::runtime_error(msg) {}
UnknownFunctionError::UnknownFunctionError(std::string msg) : std::runtime_error(msg) {}
NotImplementedError::NotImplementedError(std::string msg) : std::runtime_error(msg) {}

Cell::Cell() : type(INT32) { int32 = 0; }
Cell::Cell(int i) : type(INT32) { int32 = i; }
Cell::Cell(Cell *other) : type(ADDRESS) { address = other; }
Cell::Cell(char *s) : type(ZSTRING) { string = s; }
Cell::Cell(Instruction inst) : type(INSTRUCTION) { instruction = inst; }
Cell::Cell(CodeBlock *block) : type(PROCEDURE) { procedure = block; }
Cell::Cell(Object *obj) : type(OBJECT) { object = obj; }

Cell::Cell(const Cell &other)
: type(other.type)
{
	switch (other.type)
	{
		case INT32: int32 = other.int32; break;
		case INSTRUCTION: instruction = other.instruction; break;
		case ZSTRING: string = other.string; break;
		case PROCEDURE: procedure = other.procedure; break;
		case OBJECT: object = other.object; break;
		case ADDRESS: 
		default: address = other.address; break;
	}
}

std::string Cell::typeAsString(CellType t)
{
	switch (t)
	{
		case INT32: return std::string("@integer");
		case INSTRUCTION: return std::string("@instruction");
		case ZSTRING: return std::string("@string");
		case PROCEDURE: return std::string("@procedure");
		case OBJECT: return std::string("@object");
		case ADDRESS: 
		default:  return std::string("@pointer");
	}
}

std::string Cell::toString() const
{
	std::stringstream output;
	switch (this->type)
	{
		case INT32: {
			output << "$" << int32;
			break;
		}
		case ZSTRING: {
			output << '"' << std::string(string) << '"';
			break;
		}
		case ADDRESS: {
			output << "&(" << address->toString() << ")";
			break;
		}
		case INSTRUCTION: {
			output << "<" << instructionAsString(instruction) << ">";
			break;
		}
		case PROCEDURE: {
			output << "procedure(" << procedure->toString() << ")";
			break;
		}
		case OBJECT: {
			output << object->toString();
			break;
		}
		default: {
			output << std::hex << (void*)address;
			break;
		}
	}
	return output.str();
}

bool Cell::operator==(const Cell &other) const
{
	if (type != other.type) return false;
	switch (type)
	{
		case INT32: return this->int32 == other.int32;
		case ZSTRING: return strcmp(this->string, other.string) == 0;
		case ADDRESS: return this->address == other.address;
		case INSTRUCTION: return this->instruction == other.instruction;
		case PROCEDURE: return this->procedure == other.procedure;
		case OBJECT: return this->object == other.object;
		default: return false;
	}
}
bool Cell::operator<(const Cell &other) const
{
	if (this->type < other.type) return true;
	else if (this->type == other.type)
	{
		switch (type)
		{
			case INT32: return this->int32 < other.int32;
			case ZSTRING: return strcmp(this->string, other.string) < 0;
			case ADDRESS: return this->address < other.address;
			case INSTRUCTION: return this->instruction < other.instruction;
			case PROCEDURE: return this->procedure < other.procedure;
			case OBJECT: return this->object < other.object;
			default: return false;
		}
	}
	else return false;
}

void Cell::assert_type(CellType t, std::string message)
{
	if (type != t)
	{
		std::string received = Cell::typeAsString(this->type);
		std::string expected = Cell::typeAsString(t);
		std::stringstream output;
		output << "Illegal operand type from " << message << " - expected " << expected << " but received " << received;
		throw CellTypeException(output.str());
	}
}

StackFrame::StackFrame(const CodeBlock *cblock, Object *ctx, Cell *raddr) : code(cblock), context(ctx), location_pointer(raddr)  {}

StackFrame::StackFrame(const StackFrame &other) : code(other.code), context(other.context), location_pointer(other.location_pointer)  {}

Cell *StackFrame::begin() const
{
	return code->text;
}
Cell *StackFrame::current() const
{
	return location_pointer;
}
Cell *StackFrame::end() const
{
	return code->text + code->size;
}
std::string StackFrame::toString() const
{
	std::stringstream output;
	output << "<" << begin() << "|" << current() << ">";
	output << code->toString();
	return output.str();
}


CodeBlock::CodeBlock(unsigned int s, Cell *txt) : size(s), text(txt) {}

std::string CodeBlock::toString() const
{
	std::stringstream output;
	output << "size=" << size << ", [";
	for (unsigned int i=0; i<size; ++i)
	{
		if (i>0) output << " ";
		output << text[i].toString();
	}
	output << "]";
	return output.str();
}

Dictionary::~Dictionary()
{
	std::map<std::string, CodeBlock*>::iterator iter;
	for (iter=words.begin(); iter!=words.end(); ++iter)
	{
		CodeBlock *block = iter->second;
		delete [] block->text;
		delete block;
	}	
}
CodeBlock* Dictionary::lookup(std::string key)
{
	std::map<std::string, CodeBlock*>::iterator result = words.find(key);
	if (result == words.end())
	{
		std::stringstream output;
		output << "Could not find function " << key;
		throw UnknownFunctionError(output.str());
	}
	else
	{
		return result->second;
	}
}

void Dictionary::define(std::string key, CodeBlock* value)
{
	words.insert( std::map<std::string, CodeBlock*>::value_type(key, value) );
}

/* RuntimeMachine */

RuntimeMachine::RuntimeMachine()
: object_storage()	{
	this->global_object = new Object;
	this->reset();
}
RuntimeMachine::~RuntimeMachine() {
	delete this->global_object;
}
void RuntimeMachine::reset()
{
	argument_stack.clear();
	return_stack.clear();
	continue_execution = false;
}

StackFrame& RuntimeMachine::current_stack_frame()
{
	return return_stack.front();
}

/* internal functions used by instructions */
void RuntimeMachine::push_argument(Cell c)
{
	argument_stack.push_front(c);
}

Cell RuntimeMachine::pop_argument()
{
	Cell value = argument_stack.front();
	argument_stack.pop_front();
	return value;
}

Cell RuntimeMachine::read_byte()
{
	StackFrame &current = this->current_stack_frame();
	if ( current.location_pointer == current.end() )
	{
		throw ExecutionOutOfBoundsError(std::string("Read past code bounds"));		
	}
	else
	{
		Cell value = *(current.location_pointer);
		current.location_pointer++;
		return value;
	}
}


void RuntimeMachine::halt()
{
	continue_execution = false;
}

/* dictionary handling */
/*
	needs to re-implement dictionary methods, to abstract away name storage.
	someday this will involve a multi-namespace lookup
*/

CodeBlock* RuntimeMachine::lookup_word(std::string key)
{
	char *key_str = strdup(key.c_str());
	Cell tmpkey(key_str);

	Cell value = this->global_object->getattr(tmpkey);

	free(key_str);

	value.assert_type(PROCEDURE, std::string("RuntimeMachine::lookup_word(") + key + ")");
	return value.procedure;
}

void RuntimeMachine::define_word(std::string name, CodeBlock* code)
{
	char *namech = this->create_string(name.c_str());
	Cell key(namech);
	Cell value(code);
	this->global_object->setattr(key, value);
	//throw NotImplementedError(std::string("RuntimeMachine::define_word(") + name + ", " + code->toString() + ")");
}


/* allocate managed memory */
CodeBlock* RuntimeMachine::create_anonymous_procedure(unsigned int length)
{
	return object_storage.create_procedure(length);;
}

Object* RuntimeMachine::create_object()
{
	return object_storage.create_object();
}

char* RuntimeMachine::create_string(const char *other)
{
	return object_storage.duplicate_string(other);
}


/* execution */

void RuntimeMachine::execute_next_instruction()
{
	Cell byte = read_byte();
	if (byte.type == INSTRUCTION)
	{
		byte.instruction(this);
	}
	else if (byte.type == PROCEDURE)
	{
		call_function(current_stack_frame().context, byte.procedure);
	}
	else if (byte.type == ZSTRING)
	{
		// forth function?
		std::string key(byte.string);
		CodeBlock *code = this->lookup_word(key);
		if (code->size > 0)
		{
			call_function(current_stack_frame().context, code);
		}
		else
		{
			throw ExecutionOutOfBoundsError(std::string("Attempted execute null function"));
		}
	}
	else
	{
		std::stringstream output;
		output << "RuntimeMachine::execute_next_instruction - Illegal operand type for instruction - expected " << Cell::typeAsString(INSTRUCTION) << ", " << Cell::typeAsString(PROCEDURE) << ", or " << Cell::typeAsString(ZSTRING) << ", but received " << Cell::typeAsString(byte.type);
		throw CellTypeException(output.str());
	}
}

void RuntimeMachine::compile_next_instruction(CodeBlock *dest, int index)
{
	Cell byte = read_byte();
	if (byte.type == INSTRUCTION || byte.type == PROCEDURE)
	{
		dest->text[index] = byte;
	}
	else if (byte.type == ZSTRING)
	{
		std::string key(byte.string);
		CodeBlock *code = this->lookup_word(key);
		if (code->size > 0)
		{
			dest->text[index] = Cell(code);
		}
		else
		{
			// assume forward-declared function
			dest->text[index] = byte;
		}
	}
	else
	{
		dest->text[index] = byte;
	}
}

void RuntimeMachine::call_function(Object *new_context, const CodeBlock *code)
{
	
	StackFrame newframe(code, new_context, code->text);
	return_stack.push_front(newframe);
}

void RuntimeMachine::restore_stack_frame()
{
	return_stack.pop_front();
}

Cell RuntimeMachine::execute(const CodeBlock *block)
{
	continue_execution = true;
	StackFrame current(block, global_object, block->text);
	return_stack.push_front(current);	

	while (continue_execution)
	{
		execute_next_instruction();
	}

	if (argument_stack.size() > 0)
	{
		return argument_stack.front();
	}
	else return Cell(0);
}

void RuntimeMachine::collect_garbage()
{
	for (std::list<Cell>::iterator iter=argument_stack.begin(); iter!=argument_stack.end(); ++iter)
	{
		object_storage.mark(*iter);
	}
	object_storage.sweep();
}
