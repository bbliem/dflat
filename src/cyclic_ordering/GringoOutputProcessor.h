#pragma once

#include "../GringoOutputProcessor.h"

#include <sharp/main>

namespace cyclic_ordering {

class GringoOutputProcessor : public ::GringoOutputProcessor
{
public:
	virtual void printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);

	typedef std::pair<sharp::Vertex,unsigned> VertexAndNumber; // Vertex and the number it is assigned
	typedef std::pair<VertexAndNumber,Clasp::SymbolTable::key_type> MappingAndSymbolTableKey;
	typedef std::vector<MappingAndSymbolTableKey> MappingAndSymbolTableKeyVec;
	const MappingAndSymbolTableKeyVec& getMap() const { return map; }
	unsigned int getN() const { return n; }

private:
	MappingAndSymbolTableKeyVec map; // Holds pairs of 1) pairs ("arg1","arg2") and 2) the key in the symbol table which is mapped to the clasp variable corresponding to "map(arg1,arg2)" (different keys may be mapped to the same variables due to clasp internals)
	unsigned int n;
};

} // namespace cyclic_ordering
