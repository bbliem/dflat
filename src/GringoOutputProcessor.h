// Taken from gringo (GPL), made some modifications.

#pragma once

#include <gringo/gringo.h>
#include <gringo/lparseconverter.h>
#include <clasp/program_builder.h>

class GringoOutputProcessor : public LparseConverter
{
	typedef std::vector<bool> BoolVec;
public:
	GringoOutputProcessor();

	virtual void initialize();
	virtual void setProgramBuilder(Clasp::ProgramBuilder* api) { b_ = api; }
	virtual const SymbolMap &symbolMap(uint32_t domId) const;
	virtual ValRng vals(Domain *dom, uint32_t offset) const;

protected:
	virtual void printBasicRule(int head, const AtomVec &pos, const AtomVec &neg);
	virtual void printConstraintRule(int head, int bound, const AtomVec &pos, const AtomVec &neg);
	virtual void printChoiceRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg);
	virtual void printWeightRule(int head, int bound, const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg);
	virtual void printMinimizeRule(const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg);
	virtual void printDisjunctiveRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg);
	virtual void printComputeRule(int models, const AtomVec &pos, const AtomVec &neg);
	virtual void printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);
	virtual void printExternalTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name);
	virtual uint32_t symbol();
	virtual void doFinalize();
protected:
	Clasp::ProgramBuilder *b_;
	BoolVec  atomUnnamed_;
	uint32_t lastUnnamed_;
};
