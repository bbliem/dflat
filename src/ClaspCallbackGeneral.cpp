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
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getExtendAtoms())
				extendAtoms[it.first] = symTab[it.second].lit;
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
			foreach(const PredecessorData::value_type& it, predecessorData) {
				foreach(const TopLevelItemsToRow::value_type& it2, it.second) {
					algorithm.addRowToTable(table, it2.second);
				}
			}
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

	Row::ExtensionPointerTuple childRows;
	childRows.reserve(numChildNodes);

	foreach(const LongToLiteral::value_type& it, extendAtoms) {
		if(s.isTrue(it.second)) {
			childRows.push_back(reinterpret_cast<const Row*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childRows.size() == numChildNodes)
				break;
#endif
		}
	}

#ifndef DISABLE_ANSWER_SET_CHECKS
	if(childRows.size() > 0 && childRows.size() != numChildNodes)
		throw std::runtime_error("Number of extended rows non-zero and not equal to number of child nodes");
#endif

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
			path[atom.level].insert(atom.value);
		}
	}
	// A path does not have to use all levels, but up to the highest used level it must be connected.
	path.resize(highestLevel+1);

	assert(!path.empty());
	Row*& rowPtr = predecessorData[childRows][path.front()];
	if(!rowPtr) {
		rowPtr = new Row(path.front());

		if(childRows.empty())
			rowPtr->setCount(1);
		else
			rowPtr->addExtensionPointerTuple(childRows);
	}
	else
		rowPtr->addSubItemsPath(path.begin(), path.end());

	Row& row = *rowPtr;

	foreach(const LongToLiteral::value_type& it, countAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(!childRows.empty())
				throw std::runtime_error("Counts present although extension pointers are used");
			if(row.getCount() > 1 && row.getCount() != it.first)
				throw std::runtime_error("Different count for same top-level items");
#endif
			row.setCount(it.first);
#ifdef DISABLE_ANSWER_SET_CHECKS
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, currentCostAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(row.getCurrentCost() != 0 && row.getCurrentCost() != it.first)
				throw std::runtime_error("Different current cost for same top-level items");
#endif
			row.setCurrentCost(it.first);
#ifdef DISABLE_ANSWER_SET_CHECKS
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(row.getCost() != 0 && row.getCost() != it.first)
				throw std::runtime_error("Different cost for same top-level items");
#endif
			row.setCost(it.first);
#ifdef DISABLE_ANSWER_SET_CHECKS
			break;
#endif
		}
	}

//#ifndef DISABLE_ANSWER_SET_CHECKS
// TODO: Check if for each child node we have an extended row (or none at all)
//#endif
}
