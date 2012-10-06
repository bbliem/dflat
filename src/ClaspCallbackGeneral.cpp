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

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
//#define foreach BOOST_FOREACH // XXX: Strange: After a Boost upgrade, this led to "error: 'boost::BOOST_FOREACH' has not been declared". Moving it down helps...

#include "ClaspCallbackGeneral.h"
#include "GringoOutputProcessor.h"
#include "Row.h"

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
			foreach(const GringoOutputProcessor::ExtendAtom& it, gringoOutput.getExtendAtoms())
				extendAtoms.push_back(ExtendAtom(it.level, it.extended, symTab[it.symbolTableKey].lit));
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getLevelsAtoms())
				levelsAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCountAtoms())
				countAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCurrentCostAtoms())
				currentCostAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCostAtoms())
				costAtoms[it.first] = symTab[it.second].lit;
#ifdef PRINT_MODELS
			std::cout << std::endl;
#endif
		}
		else if(e == Clasp::ClaspFacade::event_state_exit) {
			// We need to associate rows to indices
			// XXX: Unnecessarily inefficient
			std::vector<std::vector<sharp::Row*> > childTablesVec;
			childTablesVec.reserve(childTables.size());
			foreach(sharp::Table* t, childTables)
				childTablesVec.push_back(std::vector<sharp::Row*>(t->begin(), t->end()));

			foreach(const Tree::Children::value_type& child, tree.children) {
				const ExtendArguments& predecessors = child.first.first;
				const Row::Items& topLevelItems = child.first.second;
				Row* row = new Row(Row::Tree(topLevelItems, child.second.mergeChildren()));

				if(predecessors.empty())
					row->setCount(1);
				else {
					Row::ExtensionPointerTuple extensionPointers;
					extensionPointers.reserve(predecessors.size());
					foreach(const string& predecessor, predecessors) {
						// Child table number is before, row number is after the first '_'
						const unsigned int underscorePos = predecessor.find('_');
						unsigned int tableNumber = boost::lexical_cast<unsigned int>(std::string(predecessor, 1, underscorePos-1)); // predecessor starts with 'r'
						unsigned int rowNumber = boost::lexical_cast<unsigned int>(std::string(predecessor, underscorePos + 1));
						// TODO: Instead of the following assertions, throw assertions if invalid extension pointers are given. Also, add a check that ensures that if extension pointers are given, exactly one is given for each child table.
						assert(tableNumber < childTablesVec.size());
						assert(rowNumber < childTablesVec[tableNumber].size());
						extensionPointers.push_back(dynamic_cast<const Row*>(childTablesVec[tableNumber][rowNumber]));
					}
					row->addExtensionPointerTuple(extensionPointers);
				}

				if(child.second.hasCount)
					row->setCount(child.second.count);
				if(child.second.hasCurrentCost)
					row->setCurrentCost(child.second.currentCost);
				if(child.second.hasCost)
					row->setCost(child.second.cost);

				algorithm.addRowToTable(table, row);
			}
			// Tell each row its table index
			unsigned int i = 0;
			foreach(sharp::Row* row, table)
				dynamic_cast<Row*>(row)->setIndex(i++);
		}
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

	unsigned int numLevels = 0;
	foreach(const LongToLiteral::value_type& it, levelsAtoms) {
		if(s.isTrue(it.second)) {
			numLevels = it.first;
			break;
		}
	}
#ifndef DISABLE_ANSWER_SET_CHECKS
	if(numLevels == 0)
		throw std::runtime_error("Number of levels > 0 must be specified");
#endif

	Path path(numLevels);

	foreach(ExtendAtom& atom, extendAtoms) {
		if(s.isTrue(atom.literal)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(atom.level >= numLevels)
				throw std::runtime_error("'extend' atom with too high level");
#endif
			path[atom.level].first.insert(atom.extended);
		}
	}
	foreach(ItemAtom& atom, itemAtoms) {
		if(s.isTrue(atom.literal)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(atom.level >= numLevels)
				throw std::runtime_error("'item' atom with too high level");
#endif
			path[atom.level].second.insert(atom.value);
		}
	}

	bool hasCount = false, hasCurrentCost = false, hasCost = false;
	long count = 0, currentCost = 0, cost = 0;

	foreach(const LongToLiteral::value_type& it, countAtoms) {
		if(s.isTrue(it.second)) {
			hasCount = true;
			count = it.first;
			break;
		}
	}

	foreach(const LongToLiteral::value_type& it, currentCostAtoms) {
		if(s.isTrue(it.second)) {
			hasCurrentCost = true;
			currentCost = it.first;
			break;
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
			hasCost = true;
			cost = it.first;
			break;
		}
	}

	// Insert path into tree
	tree.insert(path.begin(), path.end(), hasCount, count, hasCurrentCost, currentCost, hasCost, cost);
}

void ClaspCallbackGeneral::Tree::insert(Path::iterator pathBegin, Path::iterator pathEnd, bool hasCount, long count, bool hasCurrentCost, long currentCost, bool hasCost, long cost)
{
	if(pathBegin == pathEnd)
		return;

	Tree& child = children[*pathBegin];
	child.hasCount = hasCount;
	child.count = count;
	child.hasCurrentCost = hasCurrentCost;
	child.currentCost = currentCost;
	child.hasCost = hasCost;
	child.cost = cost;
	child.insert(++pathBegin, pathEnd, hasCount, count, hasCurrentCost, currentCost, hasCost, cost);
}

Row::Tree::Children ClaspCallbackGeneral::Tree::mergeChildren() const
{
	Row::Tree::Children result;
	foreach(const Children::value_type& child, children)
		result.insert(Row::Tree(child.first.second, child.second.mergeChildren())); // This eliminates duplicates, i.e., merges equal subtrees
	return result;
}
