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
//}}}
#include "GringoOutputProcessor.h"

namespace solver { namespace asp { namespace tables {

GringoOutputProcessor::GringoOutputProcessor(const ChildItemTrees& childItemTrees)
	: ::solver::asp::GringoOutputProcessor(childItemTrees)
{
}

const GringoOutputProcessor::ItemAtomInfos& GringoOutputProcessor::getItemAtomInfos() const
{
	return itemAtomInfos;
}

const GringoOutputProcessor::AuxItemAtomInfos& GringoOutputProcessor::getAuxItemAtomInfos() const
{
	return auxItemAtomInfos;
}

const GringoOutputProcessor::ExtendAtomInfos& GringoOutputProcessor::getExtendAtomInfos() const
{
	return extendAtomInfos;
}

const GringoOutputProcessor::CurrentCostAtomInfos& GringoOutputProcessor::getCurrentCostAtomInfos() const
{
	return currentCostAtomInfos;
}

const GringoOutputProcessor::CostAtomInfos& GringoOutputProcessor::getCostAtomInfos() const
{
	return costAtomInfos;
}

void GringoOutputProcessor::storeAtom(const std::string& name, ValVec::const_iterator firstArg, uint32_t arity, Clasp::SymbolTable::key_type symbolTableKey)
{
	// Store the atom together with its symbol table key and extracted arguments
	if(name == "item") {
		ASP_CHECK(arity == 1, "'item' predicate does not have arity 1");
		itemAtomInfos.emplace_back(ItemAtomInfo{ItemAtomArguments{getArguments(firstArg, arity).front()}, symbolTableKey});
	} else if(name == "auxItem") {
		ASP_CHECK(arity == 1, "'auxItem' predicate does not have arity 1");
		auxItemAtomInfos.emplace_back(AuxItemAtomInfo{AuxItemAtomArguments{getArguments(firstArg, arity).front()}, symbolTableKey});
	} else if(name == "extend") {
		ASP_CHECK(arity == 1, "'extend' predicate does not have arity 1");
		const std::string argument = getArguments(firstArg, arity).front();
		// Child node number is before the first '_' (and after the leading 'n')
		// Row number is after the first '_'
		const unsigned int underscorePos = argument.find('_');
		const unsigned int decompositionChildId = std::stoi(std::string(argument, 1, underscorePos-1));
		const unsigned int rowNumber = std::stoi(std::string(argument, underscorePos + 1));
		ASP_CHECK(childItemTrees.find(decompositionChildId) != childItemTrees.end(), "Extension pointer refers to nonexistent decomposition child");
		ASP_CHECK(rowNumber < childItemTrees.at(decompositionChildId)->getChildren().size(), "Extension pointer references invalid row number");
		extendAtomInfos.emplace_back(ExtendAtomInfo{{decompositionChildId, childItemTrees.at(decompositionChildId)->getChild(rowNumber).getRoot()}, symbolTableKey});
	} else if(name == "currentCost") {
		ASP_CHECK(arity == 1, "'currentCost' predicate does not have arity 1");
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo{{std::stol(getArguments(firstArg, arity).front())}, symbolTableKey});
	} else if(name == "cost") {
		ASP_CHECK(arity == 1, "'cost' predicate does not have arity 1");
		costAtomInfos.emplace_back(CostAtomInfo{{std::stol(getArguments(firstArg, arity).front())}, symbolTableKey});
	}
}

}}} // namespace solver::asp::tables
