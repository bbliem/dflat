/*
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

#include "ClaspCallbackNP.h"
#include "GringoOutputProcessor.h"
#include "ItemTreeBranchLookupTable.h"

namespace solver { namespace asp {

ClaspCallbackNP::ClaspCallbackNP(const GringoOutputProcessor& gringoOutput, const std::vector<ItemTreeBranchLookupTable>& itemTreeBranchLookupTables)
	: itemTree({{}})
	, gringoOutput(gringoOutput)
	, itemTreeBranchLookupTables(itemTreeBranchLookupTables)
{
}

ItemTree&& ClaspCallbackNP::getItemTree()
{
	return std::move(itemTree);
}

void ClaspCallbackNP::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallbackNP::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve) {
		if(e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			for(const GringoOutputProcessor::ItemAtom& it : gringoOutput.getItemAtoms()) {
#ifndef DISABLE_ANSWER_SET_CHECKS
				if(it.level != 0)
					throw std::runtime_error("item predicate uses invalid level");
#endif
				itemAtoms.emplace_back(it.value, symTab[it.symbolTableKey].lit);
			}

			for(const GringoOutputProcessor::ExtendAtom& it : gringoOutput.getExtendAtoms()) {
				assert(it.level == 0);
				extendAtoms[it.extended] = symTab[it.symbolTableKey].lit;
			}
			for(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it : gringoOutput.getCountAtoms())
				countAtoms[it.first] = symTab[it.second].lit;
			for(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it : gringoOutput.getCurrentCostAtoms())
				currentCostAtoms[it.first] = symTab[it.second].lit;
			for(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it : gringoOutput.getCostAtoms())
				costAtoms[it.first] = symTab[it.second].lit;
#ifdef PRINT_MODELS
			std::cout << std::endl;
#endif
		}
	}
}

void ClaspCallbackNP::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

#ifdef PRINT_MODELS
	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated-1 << ": ";
	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		if(s.isTrue(it->second.lit) && !it->second.name.empty())
			std::cout << it->second.name.c_str() << ' ';
	}
	std::cout << std::endl;
#endif

	ItemTreeNode::Items items;
	for(ItemAtom& atom : itemAtoms) {
		if(s.isTrue(atom.literal))
			items.insert(atom.value);
	}

	ItemTreeNode::ExtensionPointerTuple childRows;
	childRows.reserve(itemTreeBranchLookupTables.size());

	for(const StringToLiteral::value_type& it : extendAtoms) {
		if(s.isTrue(it.second)) {
			// Child node number is before the first '_' (and after the initial 'r')
			// Row number is after the first '_'
			unsigned int underscorePos = it.first.find('_');
			unsigned int tableNumber = std::stoi(std::string(it.first, 1, underscorePos-1));
			unsigned int rowNumber = std::stoi(std::string(it.first, underscorePos + 1));
			// TODO: Instead of the following assertions, throw assertions if invalid extension pointers are given. Also, add a check that ensures that if extension pointers are given, exactly one is given for each child table.
			assert(tableNumber < itemTreeBranchLookupTables.size());
			assert(rowNumber < itemTreeBranchLookupTables[tableNumber].getBranches().size());
			childRows.push_back(&itemTreeBranchLookupTables[tableNumber][rowNumber].getRoot());
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childRows.size() == itemTreeBranchLookupTables.size())
				break;
#endif
		}
	}

#ifndef DISABLE_ANSWER_SET_CHECKS
	if(childRows.size() > 0 && childRows.size() != itemTreeBranchLookupTables.size())
		throw std::runtime_error("Number of extended rows non-zero and not equal to number of child nodes");
#endif

	itemTree.addChildAndMerge(ItemTreePtr(new ItemTree({std::move(items), {std::move(childRows)}})));
}

}} // namespace solver::asp
