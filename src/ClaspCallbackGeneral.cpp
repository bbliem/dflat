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
//#define foreach BOOST_FOREACH // XXX: Strange: After a Boost upgrade, this led to "error: 'boost::BOOST_FOREACH' has not been declared". Moving it down helps...

#include "ClaspCallbackGeneral.h"
#include "GringoOutputProcessor.h"
#include "RowGeneral.h"

#define foreach BOOST_FOREACH

void ClaspCallbackGeneral::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallbackGeneral::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve) {
		if(e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			foreach(const GringoOutputProcessor::ItemAtom& it, gringoOutput.getItemAtoms())
				itemAtoms.push_back(ItemAtom(it.level, it.value, symTab[it.symbolTableKey].lit));
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildRowAtoms())
				chosenChildRowAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildRowLAtoms()) // XXX: Obsolete
				chosenChildRowLAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildRowRAtoms()) // XXX: Obsolete
				chosenChildRowRAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCurrentCostAtoms())
				currentCostAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCostAtoms())
				costAtoms[it.first] = symTab[it.second].lit;
#ifdef PRINT_MODELS
			std::cout << std::endl;
#endif
		}
		else if(e == Clasp::ClaspFacade::event_state_exit)
			pathCollection.fillTable(table, algorithm);
	}
}

void ClaspCallbackGeneral::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

#ifdef PRINT_MODELS
	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated << ": ";
	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		if(s.isTrue(it->second.lit) && !it->second.name.empty())
			std::cout << it->second.name.c_str() << ' ';
	}
	std::cout << std::endl;
#endif

	std::vector<const sharp::Table::value_type*> childRowsAndPlans;
	childRowsAndPlans.reserve(numChildNodes);
	unsigned currentCost = 0;
	unsigned cost = 0;

	foreach(const LongToLiteral::value_type& it, chosenChildRowAtoms) {
		if(s.isTrue(it.second)) {
			childRowsAndPlans.push_back(reinterpret_cast<const sharp::Table::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childRowsAndPlans.size() == numChildNodes)
				break;
#endif
		}
	}

	// XXX: Obsolete
	foreach(const LongToLiteral::value_type& it, chosenChildRowLAtoms) {
		if(s.isTrue(it.second)) {
			childRowsAndPlans.push_back(reinterpret_cast<const sharp::Table::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childRowsAndPlans.size() == numChildNodes)
				break;
#else
			if(childRowsAndPlans.size() != 1)
				throw std::runtime_error("You may only use chosenChildRow/1 if you use neither chosenChildRowL/1 nor chosenChildRowR/1.");
#endif
		}
	}

	// XXX: Obsolete
	foreach(const LongToLiteral::value_type& it, chosenChildRowRAtoms) {
		if(s.isTrue(it.second)) {
			childRowsAndPlans.push_back(reinterpret_cast<const sharp::Table::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childRowsAndPlans.size() == numChildNodes)
				break;
#else
			if(childRowsAndPlans.size() != 2)
				throw std::runtime_error("You may only use chosenChildRow/1 if you use neither chosenChildRowL/1 nor chosenChildRowR/1.");
#endif
		}
	}

#ifndef DISABLE_ANSWER_SET_CHECKS
	if(childRowsAndPlans.size() > 0 && childRowsAndPlans.size() != numChildNodes)
		throw std::runtime_error("Number of chosen child rows not equal to number of child nodes");
#endif

	foreach(const LongToLiteral::value_type& it, currentCostAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(currentCost != 0)
				throw std::runtime_error("Multiple current costs");
#endif
			currentCost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(cost != 0)
				throw std::runtime_error("Multiple costs");
#endif
			cost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

//#ifndef DISABLE_ANSWER_SET_CHECKS
// TODO: Check if for each child node we have a chosenChildRow (or none at all)
//#endif

	Path path(numLevels);
	unsigned int highestLevel = 0; // Highest level of an item set encountered so far
	foreach(ItemAtom& atom, itemAtoms) {
		if(s.isTrue(atom.literal)) {
			highestLevel = std::max(highestLevel, atom.level);
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(atom.level >= numLevels) {
				std::ostringstream err;
				err << "item predicate uses invalid level " << atom.level;
				throw std::runtime_error(err.str());
			}
#endif
			path[atom.level].push_back(atom.value);
		}
	}
	// A path does not have to use all levels, but up to the highest used level it must be connected.
	path.resize(highestLevel+1);
	pathCollection.insert(path, childRowsAndPlans, currentCost, cost);
}

inline void ClaspCallbackGeneral::PathCollection::insert(const Path& path, const std::vector<const TableRow*>& predecessors, unsigned currentCost, unsigned cost)
{
	assert(!path.empty());
	TopLevelItemsToRowData& rowDataMap = predecessorData[predecessors];
	const Row::Items& topLevelItems = path.front();
	RowData& rowData = rowDataMap[topLevelItems];

	rowData.paths.push_back(path);
#ifndef DISABLE_ANSWER_SET_CHECKS
	if(rowData.currentCost != 0 && rowData.currentCost != currentCost)
		throw std::runtime_error("Different current cost for same top-level items");
	if(rowData.cost != 0 && rowData.cost != cost)
		throw std::runtime_error("Different cost for same top-level items");
#endif
	rowData.currentCost = currentCost;
	rowData.cost = cost;
}

inline void ClaspCallbackGeneral::PathCollection::fillTable(sharp::Table& table, const Algorithm& algorithm) const
{
	// For all (pairs of) predecessors, build new rows from our collected paths
	foreach(const PredecessorData::value_type& it, predecessorData) {
		const TableRows& predecessors = it.first;
		foreach(const TopLevelItemsToRowData::value_type& it2, it.second) {
			const RowData& rowData = it2.second;

			RowGeneral& newRow = *new RowGeneral;
			newRow.currentCost = rowData.currentCost;
			newRow.cost = rowData.cost;

			foreach(const Path& path, rowData.paths) {
				assert(path.front() == it2.first); // top-level items must coincide
				newRow.tree.addPath(path.begin(), path.end());
				assert(newRow.tree.children.size() == 1); // each row may only have one top-level item set
			}

//			std::vector<const sharp::Plan*> plans;
//			plans.reserve(predecessors.size());
//			foreach(const TableRow* row, predecessors)
//				plans.push_back(row->second);
//			algorithm.addRowToTable(table, &newRow, algorithm.getPlanFactory().join(newRow, plans));

			sharp::Plan* plan;
			if(predecessors.empty())
				plan = algorithm.getPlanFactory().leaf(newRow);
			else if(predecessors.size() == 1)
				plan = algorithm.getPlanFactory().join(newRow, predecessors[0]->second);
			else {
				plan = algorithm.getPlanFactory().join(newRow, predecessors[0]->second, predecessors[1]->second);
				for(unsigned i = 2; i < predecessors.size(); ++i)
					plan = algorithm.getPlanFactory().join(newRow, plan, predecessors[i]->second);
			}
			algorithm.addRowToTable(table, &newRow, plan);
		}
	}
}
