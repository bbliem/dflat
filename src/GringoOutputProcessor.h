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
	const LongToSymbolTableKey& getChosenChildTupleLAtoms() const { return chosenChildTupleLAtoms; }
	const LongToSymbolTableKey& getChosenChildTupleRAtoms() const { return chosenChildTupleRAtoms; }
	const LongToSymbolTableKey& getCurrentCostAtoms() const { return currentCostAtoms; }
	const LongToSymbolTableKey& getCostAtoms() const { return costAtoms; }

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

	Clasp::ProgramBuilder *b_;
	BoolVec  atomUnnamed_;
	uint32_t lastUnnamed_;

private:
	std::vector<MapAtom> mapAtoms; // Holds pairs of 1) pairs ("arg0","arg1","arg2") and 2) the key in the symbol table which is mapped to the clasp variable corresponding to "map(arg0,arg1,arg2)" (different keys may be mapped to the same variables due to clasp internals)
	LongToSymbolTableKey chosenChildTupleAtoms; // Maps addresses of entries in the TupleSet corresponding to a child tuple (with solution) to the symbol table key of "chosenChildTuple(address)"
	LongToSymbolTableKey chosenChildTupleLAtoms; // same for chosenChildTupleL(address)
	LongToSymbolTableKey chosenChildTupleRAtoms; // same for chosenChildTupleR(address)
	LongToSymbolTableKey currentCostAtoms;
	LongToSymbolTableKey costAtoms;

	void storeChildTupleAtom(const std::string& name, const AtomRef& atom, LongToSymbolTableKey& store);
	void storeCostAtom(const std::string& name, const AtomRef& atom, LongToSymbolTableKey& store);

#ifndef NDEBUG
	unsigned int mapArity;
#endif

};
