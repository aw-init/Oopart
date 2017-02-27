#include <iostream>
#include <iomanip>
#include <list>

#include "interpreter.hpp"
#include "instructions.hpp"

int main(int argc, char **argv)
{
	RuntimeMachine machine;
	char *name = machine.create_string("put_5");

	const unsigned int MAIN_SIZE = 10;
	Cell code[MAIN_SIZE] = {
		
		Cell(compile_procedure),
		Cell(3),
		Cell(load_immediate),
		Cell(17),
		Cell(return_from_function),

		Cell(load_immediate),
		Cell(name),

		Cell(create_empty_object),

		Cell(set_object_attribute),

		Cell(exit_program)
	};
	CodeBlock main_func(MAIN_SIZE, code);


	Cell top = machine.execute(&main_func);
	std::cout << "Result " << top.toString() << std::endl;

	machine.reset();
	machine.collect_garbage();

	return 0;
}
