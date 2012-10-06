#pragma once

#include "../GringoOutputProcessor.h"

namespace threeCol {

class GringoOutputProcessor : public ::GringoOutputProcessor
{
public:
	virtual void printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);

	typedef std::pair<long, Clasp::SymbolTable::key_type> LongAndSymbolTableKey;
	typedef std::vector<LongAndSymbolTableKey> LongAndSymbolTableKeyVec;
	const LongAndSymbolTableKeyVec& getR() const { return r; }
	const LongAndSymbolTableKeyVec& getG() const { return g; }
	const LongAndSymbolTableKeyVec& getB() const { return b; }
	const LongAndSymbolTableKeyVec& getChosenChildTuple() const { return chosenChildTuple; }

private:
	LongAndSymbolTableKeyVec r; // Holds pairs of "arg" and the key in the symbol table which is mapped to the clasp variable corresponding to "r(arg)" (different keys may be mapped to the same variables due to clasp internals)
	LongAndSymbolTableKeyVec g;
	LongAndSymbolTableKeyVec b;
	LongAndSymbolTableKeyVec chosenChildTuple;
};

} // namespace threeCol
