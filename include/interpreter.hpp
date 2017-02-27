#ifndef interpreter_hpp
#define interpreter_hpp

#include <stack>
#include <map>
#include <list>
#include <string>
#include <stdexcept>


/* forward declarations */
struct RuntimeMachine;
struct Cell;
class Object;


/* an instruction is a pointer to a function of type void -> void */
typedef void (*Instruction)(RuntimeMachine*);

/*
	TODO: Keep track of memory.
	- Storing strings in cells
	- Allocating new blocks of anonymous code.
*/


class CellTypeException : public std::runtime_error
{
	public:
	CellTypeException(std::string msg);
};

class NotImplementedError : public std::runtime_error
{
	public:
	NotImplementedError(std::string msg);
};

class ExecutionOutOfBoundsError : public std::runtime_error
{
	public:
	ExecutionOutOfBoundsError(std::string msg);
};

class UnknownFunctionError : public std::runtime_error
{
	public:
	UnknownFunctionError(std::string msg);
};

class KeyNotFoundException : public std::runtime_error
{
	public:
	KeyNotFoundException(std::string msg);
};


struct CodeBlock
{
	unsigned int size;
	Cell *text;

	CodeBlock(unsigned int s, Cell *txt);
	std::string toString() const;
};

enum CellType { INT32, ADDRESS, ZSTRING, INSTRUCTION, PROCEDURE, OBJECT };

struct Cell {
	union {
		int int32;
		Cell *address;
		Instruction instruction;
		char *string;
		CodeBlock *procedure;
		Object* object;
	};
	CellType type;
	Cell();
	Cell(const Cell &other);
	Cell(int i);
	Cell(char *s);
	Cell(Cell* other);
	Cell(Instruction inst);
	Cell(CodeBlock *code);
	Cell(Object *obj);

	bool operator==(const Cell &other) const;
	bool operator<(const Cell &other) const;
	void assert_type(CellType t, std::string msg);
	std::string toString() const;

	static std::string typeAsString(CellType t);
};


struct ObjectIterator
{
	std::list<Cell>::iterator key;
	std::list<Cell>::iterator value;

	ObjectIterator(std::list<Cell>::iterator k, std::list<Cell>::iterator v);
	ObjectIterator& operator++();
	bool operator==(const ObjectIterator &other);
	bool operator!=(const ObjectIterator &other);
};

class Object
{
	std::list<Cell> keys;
	std::list<Cell> values;

	public:
	unsigned int size();
	void setattr(Cell key, Cell value);
	Cell getattr(Cell key);

	ObjectIterator begin();
	ObjectIterator end();

	std::string toString();
};


struct StackFrame
{
	const CodeBlock *code;
	Object *context;
	Cell *location_pointer;

	StackFrame(const CodeBlock *block, Object *context, Cell *raddr);
	StackFrame(const StackFrame &other);

	Cell *begin() const;
	Cell *current() const;
	Cell *end() const;

	std::string toString() const;
};



class Dictionary
{
	std::map<std::string, CodeBlock*> words;

	public:
	~Dictionary();
	CodeBlock* lookup(std::string key);
	void define(std::string key, CodeBlock *value);
};


class GarbageCollector
{
	std::map<Cell, bool> storage;

	public:
	void mark(Cell c);
	void sweep();

	Object* create_object();
	CodeBlock* create_procedure(unsigned int);
	char* duplicate_string(const char *cpy);
	//char *create_string(char *current);
};



class RuntimeMachine
{
	/* data */
	#ifdef DEBUG
	public:
	#endif

	GarbageCollector object_storage;

	std::list<Cell> argument_stack;
	std::list<StackFrame> return_stack;

	bool continue_execution;

	Object *global_object;


	public:
	RuntimeMachine();
	~RuntimeMachine();
	Cell execute(const CodeBlock *target);

	StackFrame &current_stack_frame();

	CodeBlock* lookup_word(std::string name);
	void define_word(std::string name, CodeBlock* code);

	CodeBlock* create_anonymous_procedure(unsigned int length);
	Object* create_object();
	char* create_string(const char *other);
	

	void push_argument(Cell c);
	Cell pop_argument();
	Cell read_byte();
	void halt();

	void compile_next_instruction(CodeBlock*, int index);

	void execute_next_instruction();
	void call_function(Object *context, const CodeBlock *block);
	void restore_stack_frame();

	void collect_garbage();
	void reset();
};
#endif
