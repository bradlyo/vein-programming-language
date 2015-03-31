#ifndef __ANY_TYPE_H__
#define __ANY_TYPE_H__
#include "AST.h"

class AnyType {
private:
	std::vector<uint64_t> arrays;

	AnyType() { }
public:
	int arrays_size() const { return arrays.size(); }

	std::string type; 
	bool isSigned();
	int numPointers = 0; 
	int absoluteNumPtrs();

	std::string string(bool no_brackets = false);

	Type *getType();

	static AnyType *Create(Type *t);
	AnyType* clone();

	AnyType(std::string *type, int numPointers, std::vector<BaseVal *> *arrays);
};

#endif