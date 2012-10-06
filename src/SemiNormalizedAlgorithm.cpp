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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "SemiNormalizedAlgorithm.h"
#include "Tuple.h"

using sharp::TupleTable;

SemiNormalizedAlgorithm::SemiNormalizedAlgorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram, sharp::NormalizationType normalizationType, bool ignoreOptimization, unsigned int level)
	: Algorithm(problem, planFactory, instanceFacts, normalizationType, ignoreOptimization, level), exchangeNodeProgram(exchangeNodeProgram), joinNodeProgram(joinNodeProgram)
{
	assert(normalizationType == sharp::DefaultNormalization || normalizationType == sharp::SemiNormalization);
}

void SemiNormalizedAlgorithm::declareBag(std::ostream& out, const sharp::ExtendedHypertree& node)
{
	foreach(sharp::Vertex v, node.getVertices())
		out << "current(" << problem.getVertexName(v) << ")." << std::endl;
	foreach(sharp::Vertex v, node.getIntroducedVertices())
		out << "introduced(" << problem.getVertexName(v) << ")." << std::endl;
	foreach(sharp::Vertex v, node.getRemovedVertices())
		out << "removed(" << problem.getVertexName(v) << ")." << std::endl;

	if(node.isRoot())
		out << "root." << std::endl;
}

void SemiNormalizedAlgorithm::declareChildTables(std::ostream& out, const sharp::ExtendedHypertree& node, const std::vector<TupleTable*>& childTables)
{
	if(node.getType() == sharp::Branch) {
		assert(childTables.size() == 2);
		foreach(const TupleTable::value_type& tupleAndSolution, *childTables[0])
			dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(out, tupleAndSolution, "childTupleL");
		foreach(const TupleTable::value_type& tupleAndSolution, *childTables[1])
			dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(out, tupleAndSolution, "childTupleR");
	} else {
		assert(childTables.size() == 1);
		foreach(const TupleTable::value_type& tupleAndSolution, *childTables[0])
			dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(out, tupleAndSolution);
	}
}

const char* SemiNormalizedAlgorithm::getUserProgram(const sharp::ExtendedHypertree& node)
{
	return node.getType() == sharp::Branch ? joinNodeProgram : exchangeNodeProgram;
}

TupleTable* SemiNormalizedAlgorithm::computeTable(const sharp::ExtendedHypertree& node, const std::vector<TupleTable*>& childTables)
{
	assert((node.getChildren()->size() == 0 && childTables.size() == 1) || node.getChildren()->size() == childTables.size());

	if(node.getType() != sharp::Branch || joinNodeProgram)
		return Algorithm::computeTable(node, childTables);

	// Default join implementation (used when no join node program is specified)
	TupleTable* newTable = new TupleTable;

	assert(node.getType() == sharp::Branch && node.getChildren()->size() == 2 && childTables.size() == 2);
	const TupleTable& childTableLeft = *childTables[0];
	const TupleTable& childTableRight = *childTables[1];

	// TupleTables are ordered, use sort merge join algorithm
	TupleTable::const_iterator lit = childTableLeft.begin();
	TupleTable::const_iterator rit = childTableRight.begin();
#define TUP(X) (*dynamic_cast<const Tuple*>(X->first)) // FIXME: Think of something better
	while(lit != childTableLeft.end() && rit != childTableRight.end()) {
		while(!TUP(lit).matches(TUP(rit))) {
			// Advance iterator pointing to smaller value
			if(TUP(lit) < TUP(rit)) {
				++lit;
				if(lit == childTableLeft.end())
					goto endJoin;
			} else {
				++rit;
				if(rit == childTableRight.end())
					goto endJoin;
			}
		}

		// Now lit and rit join
		// Remember position of rit and advance rit until no more match
		TupleTable::const_iterator mark = rit;
joinLitWithAllPartners:
		do {
			sharp::Tuple* t = TUP(lit).join(TUP(rit));
			sharp::Plan* p = planFactory.join(*t, lit->second, rit->second);
			addRowToTupleTable(*newTable, t, p);
			++rit;
		} while(rit != childTableRight.end() && TUP(lit).matches(TUP(rit)));

		// lit and rit don't join anymore. Advance lit. If it joins with mark, reset rit to mark.
		++lit;
		if(lit == childTableLeft.end())
			break;

		if(TUP(lit).matches(TUP(mark))) {
			rit = mark;
			goto joinLitWithAllPartners; // Ha!
		}
	}
endJoin:
	return newTable;
}
