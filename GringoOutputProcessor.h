// Taken from gringo (GPL), made some modifications.

#pragma once

#include <gringo/gringo.h>
#include <gringo/lparseconverter.h>
#include <clasp/program_builder.h>

class GringoOutputProcessor : public LparseConverter
{
	typedef std::vector<bool> BoolVec;
public:
	GringoOutputProcessor(bool shiftDisj = false);
	virtual void initialize();
	void setProgramBuilder(Clasp::ProgramBuilder* api) { b_ = api; }
	const SymbolMap &symbolMap(uint32_t domId) const;
	ValRng vals(Domain *dom, uint32_t offset) const;
	~GringoOutputProcessor();

	typedef std::pair<long, Clasp::SymbolTable::key_type> LongAndSymbolTableKey;
	typedef std::vector<LongAndSymbolTableKey> LongAndSymbolTableKeyVec;
	const LongAndSymbolTableKeyVec& getMAtoms() const { return mAtom; }
	const LongAndSymbolTableKeyVec& getChosenOldMAtoms() const { return chosenOldMAtom; }

protected:
	void printBasicRule(int head, const AtomVec &pos, const AtomVec &neg);
	void printConstraintRule(int head, int bound, const AtomVec &pos, const AtomVec &neg);
	void printChoiceRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg);
	void printWeightRule(int head, int bound, const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg);
	void printMinimizeRule(const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg);
	void printDisjunctiveRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg);
	void printComputeRule(int models, const AtomVec &pos, const AtomVec &neg);
	void printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);
	void printExternalTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);
	uint32_t symbol();
	void doFinalize();
protected:
	Clasp::ProgramBuilder *b_;
	BoolVec  atomUnnamed_;
	uint32_t lastUnnamed_;

private:
	LongAndSymbolTableKeyVec mAtom; // Holds pairs of "arg" and the key in the symbol table which is mapped to the clasp variable corresponding to "m(arg)" (different keys be mapped to the same variables due to clasp internals)
	LongAndSymbolTableKeyVec chosenOldMAtom;
};
