/*{{{
Copyright 2012-2016, Bernhard Bliem, Marius Moldovan
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
//}}}
#include <algorithm>

#include "Solver.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace {

bool isJoinable(const Row& left, const Row& right)
{
	return left.getItems() == right.getItems();
}

RowPtr joinRows(const RowPtr& left, const RowPtr& right, bool optimize)
{
	assert(left);
	assert(right);
	if(!isJoinable(*left, *right))
	    return RowPtr();

	RowPtr result;

	// Join left and right
	Row::Items items = left->getItems();
	Row::Items auxItems;
	// Unify auxiliary items
	const auto& auxL = left->getAuxItems();
	const auto& auxR = right->getAuxItems();
	std::set_union(auxL.begin(), auxL.end(), auxR.begin(), auxR.end(), std::inserter(auxItems, auxItems.end()));

	Row::ExtensionPointers extensionPointers = {{left, right}};
	result.reset(new Row(std::move(items), std::move(auxItems), std::move(extensionPointers)));

	// Set (initial) cost of this node
	if(optimize) {
		result->setCost(left->getCost() - left->getCurrentCost() + right->getCost());
		assert(left->getCurrentCost() == right->getCurrentCost());
		result->setCurrentCost(left->getCurrentCost());
	}

	return result;
}

TablePtr joinTables(const TablePtr& left, const TablePtr& right, bool optimize)
{
	assert(left);
	assert(right);

	TablePtr result;

	// Join rows recursively
	auto lit = left->getRows().begin();
	auto rit = right->getRows().begin();
	while(lit != left->getRows().end() && rit != right->getRows().end()) {
		RowPtr childResult = joinRows(*lit, *rit, optimize);
		if(childResult) {
			// lit and rit match
			// Remember position of rit. We will later advance rit until is doesn't match with lit anymore.
			auto mark = rit;
join_lit_with_all_matches:
			// Join lit will all partners starting at rit
			do {
				result->add(std::move(childResult));
				++rit;
				if(rit == right->getRows().end())
					break;
				childResult = joinRows(*lit, *rit, optimize);
			} while(childResult);

			// lit and rit don't match anymore (or rit is past the end)
			// Advance lit. If it joins with mark, reset rit to mark.
			++lit;
			if(lit != left->getRows().end()) {
				childResult = joinRows(*lit, *mark, optimize);
				if(childResult) {
					rit = mark;
					goto join_lit_with_all_matches;
				}
			}
		}
		else {
			// lit and rit don't match
			// Advance iterator pointing to smaller value
			if((*lit)->getItems() < (*rit)->getItems())
				++lit;
			else
				++rit;
		}
	}

	return result;
}

} // anonymous namespace

namespace solver { namespace default_join {

Solver::Solver(const Decomposition& decomposition, const Application& app)
	: ::Solver(decomposition, app)
{
}

TablePtr Solver::compute()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	assert(decomposition.getChildren().size() > 1);
	// Compute tables of child nodes.
	// When at least two have been computed, join them with the result so far.
	// TODO Use a balanced join tree (with smaller "tables" further down).
	auto it = decomposition.getChildren().begin();
	TablePtr result = (*it)->getSolver().compute();
	for(++it; it != decomposition.getChildren().end(); ++it) {
		if(!result)
			return TablePtr();
		TablePtr table = (*it)->getSolver().compute();
		if(!table)
			return TablePtr();
		result = joinTables(result, table, !app.isOptimizationDisabled());
	}

	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}

}} // namespace solver::default_join
