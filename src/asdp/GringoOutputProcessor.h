#pragma once

#include "../GringoOutputProcessor.h"

#include <sharp/main>

namespace asdp {

class GringoOutputProcessor : public ::GringoOutputProcessor
{
public:
	GringoOutputProcessor(const sharp::Problem&);

	virtual void printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);

	// Arguments of the map/3 predicate (Level, vertex and the string it is assigned)
	struct MapAtom {
		unsigned int level;
		std::string vertex;
		std::string value;
		Clasp::SymbolTable::key_type symbolTableKey;

		MapAtom(unsigned int level, const std::string& vertex, const std::string& value, Clasp::SymbolTable::key_type symbolTableKey)
			: level(level), vertex(vertex), value(value), symbolTableKey(symbolTableKey)
		{}
	};
	const std::vector<MapAtom>& getMapAtoms() const { return mapAtoms; }

	typedef std::map<long, Clasp::SymbolTable::key_type> LongToSymbolTableKey;
	const LongToSymbolTableKey& getChosenChildTupleAtoms() const { return chosenChildTupleAtoms; }

private:
	const sharp::Problem& problem;

	std::vector<MapAtom> mapAtoms; // Holds pairs of 1) pairs ("arg0","arg1","arg2") and 2) the key in the symbol table which is mapped to the clasp variable corresponding to "map(arg0,arg1,arg2)" (different keys may be mapped to the same variables due to clasp internals)
	LongToSymbolTableKey chosenChildTupleAtoms; // Maps addresses of entries in the TupleSet corresponding to a child tuple (with solution) to the symbol table key of "chosenChildTuple(address)"

#ifndef NDEBUG
	unsigned int mapArity;
#endif
};

} // namespace asdp
