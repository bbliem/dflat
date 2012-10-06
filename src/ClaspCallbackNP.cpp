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

#include "ClaspCallbackNP.h"
#include "GringoOutputProcessor.h"
#include "RowNP.h"

#define foreach BOOST_FOREACH

void ClaspCallbackNP::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallbackNP::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve && e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			foreach(const GringoOutputProcessor::ItemAtom& it, gringoOutput.getItemAtoms()) {
#ifndef DISABLE_ANSWER_SET_CHECKS
				if(it.level != 0)
					throw std::runtime_error("item predicate uses invalid level");
#endif
				itemAtoms.push_back(ItemAtom(it.value, symTab[it.symbolTableKey].lit));
			}

			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildRowAtoms())
				extendAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCurrentCostAtoms())
				currentCostAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCostAtoms())
				costAtoms[it.first] = symTab[it.second].lit;
#ifdef PRINT_MODELS
 			std::cout << std::endl;
#endif
	}
}

void ClaspCallbackNP::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
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

	RowNP& newRow = *new RowNP;

	std::vector<const sharp::Table::value_type*> childRowsAndPlans;
	childRowsAndPlans.reserve(numChildNodes);

	foreach(const LongToLiteral::value_type& it, extendAtoms) {
		if(s.isTrue(it.second)) {
			childRowsAndPlans.push_back(reinterpret_cast<const sharp::Table::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childRowsAndPlans.size() == numChildNodes)
				break;
#endif
		}
	}

#ifndef DISABLE_ANSWER_SET_CHECKS
	if(childRowsAndPlans.size() > 0 && childRowsAndPlans.size() != numChildNodes)
		throw std::runtime_error("Number of extended rows non-zero and not equal to number of child nodes");
#endif

	foreach(const LongToLiteral::value_type& it, currentCostAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(newRow.currentCost != 0)
				throw std::runtime_error("Multiple current costs");
#endif
			newRow.currentCost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(newRow.cost != 0)
				throw std::runtime_error("Multiple costs");
#endif
			newRow.cost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	foreach(ItemAtom& atom, itemAtoms) {
		if(s.isTrue(atom.literal))
			newRow.items.push_back(atom.value);
	}

	sharp::Plan* plan;
	if(childRowsAndPlans.empty())
		plan = algorithm.getPlanFactory().leaf(newRow);
	else if(childRowsAndPlans.size() == 1)
		plan = algorithm.getPlanFactory().join(newRow, childRowsAndPlans[0]->second);
	else {
		plan = algorithm.getPlanFactory().join(newRow, childRowsAndPlans[0]->second, childRowsAndPlans[1]->second);
		for(unsigned i = 2; i < childRowsAndPlans.size(); ++i)
			plan = algorithm.getPlanFactory().join(newRow, plan, childRowsAndPlans[i]->second);
	}
	algorithm.addRowToTable(table, &newRow, plan);
}
