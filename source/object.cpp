#include "interpreter.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <iomanip>
#include <cstring>
#include <cstdlib>


KeyNotFoundException::KeyNotFoundException(std::string msg) : std::runtime_error(msg) {}

ObjectIterator::ObjectIterator(std::list<Cell>::iterator k, std::list<Cell>::iterator v)
: key(k), value(v) {}

ObjectIterator& ObjectIterator::operator++()
{
	key++;
	value++;
	return *this;
}

bool ObjectIterator::operator==(const ObjectIterator &other)
{
	return this->key == other.key && this->value == other.value;
}
bool ObjectIterator::operator!=(const ObjectIterator &other)
{
	return !((*this) == other);
}





/* Object */

void Object::setattr(Cell key, Cell value)
{
	for (ObjectIterator iter = this->begin(); iter != this->end(); ++iter)
	{
		if (*(iter.key) == key)
		{
			*(iter.value) = value;
			return;
		}
	}
	keys.push_front(key);
	values.push_front(value);
	return;
}

unsigned int Object::size()
{
	return keys.size();
}

Cell Object::getattr(Cell key)
{
	for (ObjectIterator iter = this->begin(); iter != this->end(); ++iter)
	{
		if (*(iter.key) == key)
		{
			return *(iter.value);
		}
	}
	std::stringstream output;
	output << "Could not find key \"" << key.toString() << "\"";
	throw KeyNotFoundException(output.str());
}

ObjectIterator Object::begin()
{
	return ObjectIterator(keys.begin(), values.begin());
}

ObjectIterator Object::end()
{
	return ObjectIterator(keys.end(), values.end());
}

std::string Object::toString()
{
	std::stringstream output;
	output << "{";
	for (ObjectIterator iter = this->begin(); iter != this->end(); ++iter)
	{
		if (iter != this->begin()) output << " ";
		output << iter.key->toString();
		output << " => ";
		output << iter.value->toString();
		if (this->size() > 1) output << ";";
	}
	output << "}";
	return output.str();
}


Object* GarbageCollector::create_object()
{
	Object* obj = new Object;
	#ifdef GC_DEBUG
	std::cout << "Allocated new Object of size " << sizeof(Object) << " at " << (void*)obj << std::endl;
	#endif
	storage[Cell(obj)] = false;

	return obj;
}
char* GarbageCollector::duplicate_string(const char *other)
{
	char *dest = strdup(other);
	#ifdef GC_DEBUG
	unsigned int len = strlen(dest) + 1;
	std::cout << "Allocated new string of size " << (sizeof(char) * len) << " at " << (void*)dest << std::endl;
	#endif
	storage[Cell(dest)] = false;
	return dest;
}
CodeBlock* GarbageCollector::create_procedure(unsigned int length)
{
	Cell *text = new Cell[length];
	CodeBlock *result = new CodeBlock(length, text);

	#ifdef GC_DEBUG
	std::cout << "Allocated new Cell* of size " << (sizeof(Cell) * length) << " at " << (void*)text << std::endl;
	std::cout << "Allocated new CodeBlock of size " << sizeof(CodeBlock) << " at " << (void*)result << std::endl;
	#endif
	
	storage[Cell(result)] = false;	

	return result;
}

void gc_CellTypeException(CellType t)
{
	std::stringstream output;
	output << "Illegal operand type from GarbageCollector::sweep - expected " <<
		Cell::typeAsString(OBJECT) << " or " << 
		Cell::typeAsString(ZSTRING) << " or " << 
		Cell::typeAsString(PROCEDURE) <<
		" but received " << Cell::typeAsString(t);
	throw CellTypeException(output.str());
}
void GarbageCollector::sweep()
{
		// assumes all objects have already been marked
	std::map<Cell,bool>::iterator iter;
	for (iter=storage.begin(); iter!=storage.end();)
	{
		if (iter->second)
		{
			iter->second = false;
			iter++;
		}
		else
		{
			std::map<Cell,bool>::iterator current = iter++;
			Cell c = current->first;
			switch (c.type)
			{
				case ZSTRING: {
					
					#ifdef GC_DEBUG
					std::cout << "Collecting char* at " << (void*)c.string << std::endl;
					#endif
					free(c.string);
					break;
				}
				case PROCEDURE: {
					CodeBlock *block = c.procedure;
					#ifdef GC_DEBUG
					std::cout << "Collecting Cell* at " << (void*)block->text << std::endl;
					std::cout << "Collecting CodeBlock at " << (void*)block << std::endl;
					#endif
					delete [] block->text;
					delete block;
					break;
				}
				case OBJECT: {
					#ifdef GC_DEBUG
					std::cout << "Collecting Object at " << (void*)c.object << std::endl;
					#endif
					delete c.object;
					break;
				}
				default: gc_CellTypeException(c.type); break;
			}
			storage.erase(current);
		}
	}
}

void GarbageCollector::mark(Cell c)
{
	if (storage.count(c) > 0  && storage[c] == false)
	{
		std::cout << "~marked " << c.toString() << std::endl;
		storage[c] = true;
		if (c.type == OBJECT)
		{
			for (ObjectIterator iter = c.object->begin(); iter != c.object->end(); ++iter)
			{
				Cell key = *(iter.key);
				Cell value = *(iter.value);

				this->mark(key);
				this->mark(value);
			}			
		}
	}
}

