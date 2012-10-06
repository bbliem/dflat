#pragma once

#include "../GringoOutputProcessor.h"

namespace sat {

class GringoOutputProcessor : public ::GringoOutputProcessor
{
public:
	virtual void printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);

	typedef std::pair<long, Clasp::SymbolTable::key_type> LongAndSymbolTableKey;
	typedef std::vector<LongAndSymbolTableKey> LongAndSymbolTableKeyVec;
	const LongAndSymbolTableKeyVec& getMAtom() const { return mAtom; }
	const LongAndSymbolTableKeyVec& getMClause() const { return mClause; }
	const LongAndSymbolTableKeyVec& getChosenChildTuple() const { return chosenChildTuple; }

private:
	LongAndSymbolTableKeyVec mAtom; // Holds pairs of "arg" and the key in the symbol table which is mapped to the clasp variable corresponding to "mAtom(arg)" (different keys may be mapped to the same variables due to clasp internals)
	LongAndSymbolTableKeyVec mClause;
	LongAndSymbolTableKeyVec chosenChildTuple;
};

} // namespace sat
