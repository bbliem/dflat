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

#include "GringoOutputProcessor.h"

namespace solver { namespace asp { namespace trees {

GringoOutputProcessor::GringoOutputProcessor(const ChildItemTrees& childItemTrees)
	: ::solver::asp::GringoOutputProcessor(childItemTrees)
{
}

const GringoOutputProcessor::ItemAtomInfos& GringoOutputProcessor::getItemAtomInfos() const
{
	return itemAtomInfos;
}

const GringoOutputProcessor::ExtendAtomInfos& GringoOutputProcessor::getExtendAtomInfos() const
{
	return extendAtomInfos;
}

const GringoOutputProcessor::CountAtomInfos& GringoOutputProcessor::getCountAtomInfos() const
{
	return countAtomInfos;
}

const GringoOutputProcessor::CurrentCostAtomInfos& GringoOutputProcessor::getCurrentCostAtomInfos() const
{
	return currentCostAtomInfos;
}

const GringoOutputProcessor::CostAtomInfos& GringoOutputProcessor::getCostAtomInfos() const
{
	return costAtomInfos;
}

const GringoOutputProcessor::LengthAtomInfos& GringoOutputProcessor::getLengthAtomInfos() const
{
	return lengthAtomInfos;
}

void GringoOutputProcessor::storeAtom(const std::string& name, ValVec::const_iterator firstArg, uint32_t arity, Clasp::SymbolTable::key_type symbolTableKey)
{
	// Store the atom together with its symbol table key and extracted arguments
	if(name == "item") {
		assert(arity == 2);
		std::vector<std::string> arguments = getArguments(firstArg, arity);
		itemAtomInfos.emplace_back(ItemAtomInfo{ItemAtomArguments{static_cast<unsigned int>(std::stoi(arguments[0])), std::move(arguments[1])}, symbolTableKey});
	} else if(name == "extend") {
		assert(arity == 2);
		const std::vector<std::string> arguments = getArguments(firstArg, arity);
		const unsigned int level = std::stol(arguments[0]);
		const std::string extended = std::move(arguments[1]);
		// (Decomposition) child node number is before the first '_' (and after the leading 'n')
		// '_' then separates item tree child indices
		size_t underscorePos = extended.find('_');
		unsigned int decompositionChildId = std::stoi(std::string(extended, 1, underscorePos-1));

		const ItemTree* current = childItemTrees.at(decompositionChildId).get();
		while(underscorePos != std::string::npos) {
			const size_t lastUnderscorePos = underscorePos;
			underscorePos = extended.find('_', underscorePos+1);
			unsigned int childNumber = std::stoi(std::string(extended, lastUnderscorePos+1, underscorePos));

			current = &current->getChild(childNumber);
		}
		// XXX Instead of the previous loop which runs through all levels, it could be beneficial to assign a globally unique ID to each item tree node and then use a lookup-table. (The globally unique ID could be either an integer, as is already the case in the Decomposition class, but it could also be a string like the one we are already using.)

		extendAtomInfos.emplace_back(ExtendAtomInfo{{level, current->getRoot()}, symbolTableKey});
	} else if(name == "count") {
		assert(arity == 1);
		// TODO mpz_class?
		countAtomInfos.emplace_back(CountAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(firstArg, arity).front()))}, symbolTableKey});
	} else if(name == "currentCost") {
		assert(arity == 1);
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo{{std::stol(getArguments(firstArg, arity).front())}, symbolTableKey});
	} else if(name == "cost") {
		assert(arity == 1);
		costAtomInfos.emplace_back(CostAtomInfo{{std::stol(getArguments(firstArg, arity).front())}, symbolTableKey});
	} else if(name == "length") {
		assert(arity == 1);
		lengthAtomInfos.emplace_back(LengthAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(firstArg, arity).front()))}, symbolTableKey});
	}
}

}}} // namespace solver::asp::trees
