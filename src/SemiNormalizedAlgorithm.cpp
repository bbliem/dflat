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
#include "Row.h"

using sharp::Table;

namespace {
	void insertEmptyLeaves(sharp::Hypertree* root)
	{
		if(root->getChildren()->empty())
			root->insChild(new sharp::ExtendedHypertree(sharp::VertexSet()));
		else {
			foreach(sharp::Hypertree* child, *root->getChildren())
				insertEmptyLeaves(child);
		}
	}
}

SemiNormalizedAlgorithm::SemiNormalizedAlgorithm(sharp::Problem& problem, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram, sharp::NormalizationType normalizationType, bool ignoreOptimization, unsigned int level)
	: Algorithm(problem, instanceFacts, normalizationType, ignoreOptimization, level), exchangeNodeProgram(exchangeNodeProgram), joinNodeProgram(joinNodeProgram)
{
	assert(normalizationType == sharp::DefaultNormalization || normalizationType == sharp::SemiNormalization);
}

const char* SemiNormalizedAlgorithm::getUserProgram(const sharp::ExtendedHypertree& node)
{
	return node.getType() == sharp::Branch ? joinNodeProgram : exchangeNodeProgram;
}

Table* SemiNormalizedAlgorithm::computeTable(const sharp::ExtendedHypertree& node, const std::vector<Table*>& childTables)
{
	assert(node.getChildren()->size() == childTables.size());

	switch(node.getType()) {
		case sharp::Leaf:
			{
			assert(node.getVertices().empty());
			// We return an empty dummy child row
			Table* table = new Table;
			table->insert(new Row(Row::Items()));
			return table;
			}

		case sharp::Branch:
			if(joinNodeProgram)
				return Algorithm::computeTable(node, childTables);
			else
				return defaultJoin(node, childTables);

		case sharp::Permutation:
			return Algorithm::computeTable(node, childTables);

		default:
			break;
	}
	assert(false);
	return 0;
}

sharp::ExtendedHypertree* SemiNormalizedAlgorithm::prepareHypertreeDecomposition(sharp::ExtendedHypertree* root)
{
	// Insert empty leaves
	// FIXME: This is a workaround until SHARP lets us insert empty leaves in the normalization
	insertEmptyLeaves(root);
	return Algorithm::prepareHypertreeDecomposition(root);
}

Table* SemiNormalizedAlgorithm::defaultJoin(const sharp::ExtendedHypertree& node, const std::vector<sharp::Table*>& childTables) const
{
	// Default join implementation (used when no join node program is specified)
	Table* newTable = new Table;

	assert(node.getType() == sharp::Branch && node.getChildren()->size() == 2 && childTables.size() == 2);
	const Table& childTableLeft = *childTables[0];
	const Table& childTableRight = *childTables[1];

	// Tables are ordered, use sort merge join algorithm
	Table::const_iterator lit = childTableLeft.begin();
	Table::const_iterator rit = childTableRight.begin();
#define ROW(X) (*dynamic_cast<const Row*>(*X)) // FIXME: Think of something better
	while(lit != childTableLeft.end() && rit != childTableRight.end()) {
		while(!ROW(lit).matches(ROW(rit))) {
			// Advance iterator pointing to smaller value
			if(ROW(lit) < ROW(rit)) {
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
		Table::const_iterator mark = rit;
joinLitWithAllPartners:
		do {
			sharp::Row* r = ROW(lit).join(ROW(rit));
			addRowToTable(*newTable, r);
			++rit;
		} while(rit != childTableRight.end() && ROW(lit).matches(ROW(rit)));

		// lit and rit don't join anymore. Advance lit. If it joins with mark, reset rit to mark.
		++lit;
		if(lit == childTableLeft.end())
			break;

		if(ROW(lit).matches(ROW(mark))) {
			rit = mark;
			goto joinLitWithAllPartners; // Ha!
		}
	}
endJoin:
	return newTable;
}
