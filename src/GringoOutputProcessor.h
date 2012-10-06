/*
Copyright 2012, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

// Taken from gringo (GPL), made some modifications.
#pragma once

#include <gringo/gringo.h>
#include <gringo/lparseconverter.h>
#include <clasp/program_builder.h>

#include "Row.h"

class GringoOutputProcessor : public LparseConverter
{
	typedef std::vector<bool> BoolVec;
public:
	//! @param ignoreOptimization true iff the predicates responsible for optimization problems should be ignored (e.g., cost/1, currentCost/1)
	GringoOutputProcessor(bool ignoreOptimization = false);

	// Arguments of the item/2 predicate (the item's level and value)
	struct ItemAtom {
		unsigned int level;
		std::string value;
		Clasp::SymbolTable::key_type symbolTableKey;

		ItemAtom(unsigned int level, const std::string& value, Clasp::SymbolTable::key_type symbolTableKey)
			: level(level), value(value), symbolTableKey(symbolTableKey)
		{}
	};
	const std::vector<ItemAtom>& getItemAtoms() const { return itemAtoms; }

	// Arguments of the extend/2 predicate (the level and predecessor item set / row)
	struct ExtendAtom {
		unsigned int level;
		std::string extended;
		Clasp::SymbolTable::key_type symbolTableKey;

		ExtendAtom(unsigned int level, const std::string& extended, Clasp::SymbolTable::key_type symbolTableKey)
			: level(level), extended(extended), symbolTableKey(symbolTableKey)
		{
		}
	};
	const std::vector<ExtendAtom>& getExtendAtoms() const { return extendAtoms; }

	typedef std::map<long, Clasp::SymbolTable::key_type> LongToSymbolTableKey;
	const LongToSymbolTableKey& getCountAtoms() const { return countAtoms; }
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
	bool ignoreOptimization;
	std::vector<ItemAtom> itemAtoms;
	std::vector<ExtendAtom> extendAtoms;
	LongToSymbolTableKey countAtoms;
	LongToSymbolTableKey currentCostAtoms;
	LongToSymbolTableKey costAtoms;

	void storeNumberAtom(const AtomRef& atom, LongToSymbolTableKey& store);

#ifndef NDEBUG
	unsigned int itemArity;
#endif

};
