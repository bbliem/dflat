/*{{{
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

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
//}}}
#include <gringo/gringo.h>
#include <gringo/lparseconverter.h>
#include <clasp/program_builder.h>
#include <unordered_map>

#include "../../ItemTree.h"

#ifdef DISABLE_CHECKS
#	define ASP_CHECK(cond, error)
#else
#	define ASP_CHECK(cond, error)\
	if((cond) == false)                 \
		throw std::runtime_error(error);
#endif

namespace solver { namespace asp {

class GringoOutputProcessor : public LparseConverter
{
public:
	template<typename T>
	struct AtomInfo {
		T arguments;
		Clasp::SymbolTable::key_type symbolTableKey;
	};

	// Key: Global ID of child node; value: the child node's item tree
	typedef std::unordered_map<unsigned int, ItemTreePtr> ChildItemTrees;

	GringoOutputProcessor(const ChildItemTrees& childItemTrees);

	virtual void initialize();
	virtual void setProgramBuilder(Clasp::ProgramBuilder* api) { b_ = api; }
	virtual const SymbolMap &symbolMap(uint32_t domId) const;
	virtual ValRng vals(Domain *dom, uint32_t offset) const;

protected:
	std::vector<std::string> getArguments(ValVec::const_iterator firstArg, uint32_t arity) const;
	virtual void storeAtom(const std::string& name, ValVec::const_iterator firstArg, uint32_t arity, Clasp::SymbolTable::key_type symbolTableKey) = 0;

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

	const ChildItemTrees& childItemTrees;
	Clasp::ProgramBuilder *b_;
	typedef std::vector<bool> BoolVec;
	BoolVec atomUnnamed_;
	uint32_t lastUnnamed_;
};

}} // namespace solver::asp
