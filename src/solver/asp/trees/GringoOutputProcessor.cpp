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

namespace solver { namespace asp { namespace trees {

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

const GringoOutputProcessor::LengthAtomInfos& GringoOutputProcessor::getLengthAtomInfos() const
{
	return lengthAtomInfos;
}

const GringoOutputProcessor::OrAtomInfos& GringoOutputProcessor::getOrAtomInfos() const
{
	return orAtomInfos;
}

const GringoOutputProcessor::AndAtomInfos& GringoOutputProcessor::getAndAtomInfos() const
{
	return andAtomInfos;
}

const Clasp::SymbolTable::key_type* GringoOutputProcessor::getAcceptAtomKey() const
{
	return acceptAtomKey.get();
}

const Clasp::SymbolTable::key_type* GringoOutputProcessor::getRejectAtomKey() const
{
	return rejectAtomKey.get();
}

void GringoOutputProcessor::storeAtom(const std::string& name, ValVec::const_iterator firstArg, uint32_t arity, Clasp::SymbolTable::key_type symbolTableKey)
{
	// Store the atom together with its symbol table key and extracted arguments
	if(name == "item") {
		ASP_CHECK(arity == 2, "'item' predicate does not have arity 2");
		std::vector<std::string> arguments = getArguments(firstArg, arity);
		itemAtomInfos.emplace_back(ItemAtomInfo{ItemAtomArguments{static_cast<unsigned int>(std::stoi(arguments[0])), std::move(arguments[1])}, symbolTableKey});
	} else if(name == "auxItem") {
		ASP_CHECK(arity == 2, "'auxItem' predicate does not have arity 2");
		std::vector<std::string> arguments = getArguments(firstArg, arity);
		auxItemAtomInfos.emplace_back(AuxItemAtomInfo{AuxItemAtomArguments{static_cast<unsigned int>(std::stoi(arguments[0])), std::move(arguments[1])}, symbolTableKey});
	} else if(name == "extend") {
		ASP_CHECK(arity == 2, "'extend' predicate does not have arity 2");
		const std::vector<std::string> arguments = getArguments(firstArg, arity);
		const unsigned int level = std::stol(arguments[0]);
		const std::string extended = std::move(arguments[1]);
		// (Decomposition) child node number is before the first '_' (and after the leading 'n')
		// '_' then separates item tree child indices
		size_t underscorePos = extended.find('_');
		const unsigned int decompositionChildId = std::stoi(std::string(extended, 1, underscorePos-1));
		ASP_CHECK(childItemTrees.find(decompositionChildId) != childItemTrees.end(), "Extension pointer refers to nonexistent decomposition child");

		const ItemTree* current = childItemTrees.at(decompositionChildId).get();
		while(underscorePos != std::string::npos) {
			assert(current);
			const size_t lastUnderscorePos = underscorePos;
			underscorePos = extended.find('_', underscorePos+1);
			unsigned int childNumber = std::stoi(std::string(extended, lastUnderscorePos+1, underscorePos));
			ASP_CHECK(childNumber < current->getChildren().size(), "Extension pointer refers to nonexistent item tree node");

			current = &current->getChild(childNumber);
		}
		// XXX Instead of the previous loop which runs through all levels, it could be beneficial to assign a globally unique ID to each item tree node and then use a lookup-table. (The globally unique ID could be either an integer, as is already the case in the Decomposition class, but it could also be a string like the one we are already using.)

		extendAtomInfos.emplace_back(ExtendAtomInfo{{level, decompositionChildId, current->getRoot()}, symbolTableKey});
	} else if(name == "currentCost") {
		ASP_CHECK(arity == 1, "'currentCost' predicate does not have arity 1");
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo{{std::stol(getArguments(firstArg, arity).front())}, symbolTableKey});
	} else if(name == "cost") {
		ASP_CHECK(arity == 1, "'cost' predicate does not have arity 1");
		costAtomInfos.emplace_back(CostAtomInfo{{std::stol(getArguments(firstArg, arity).front())}, symbolTableKey});
	} else if(name == "length") {
		ASP_CHECK(arity == 1, "'length' predicate does not have arity 1");
		lengthAtomInfos.emplace_back(LengthAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(firstArg, arity).front()))}, symbolTableKey});
	} else if(name == "or") {
		ASP_CHECK(arity == 1, "'or' predicate does not have arity 1");
		orAtomInfos.emplace_back(OrAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(firstArg, arity).front()))}, symbolTableKey});
	} else if(name == "and") {
		ASP_CHECK(arity == 1, "'and' predicate does not have arity 1");
		andAtomInfos.emplace_back(AndAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(firstArg, arity).front()))}, symbolTableKey});
	} else if(name == "accept") {
		ASP_CHECK(arity == 0, "'accept' predicate does not have arity 0");
		assert(!acceptAtomKey);
		acceptAtomKey.reset(new Clasp::SymbolTable::key_type(symbolTableKey));
	} else if(name == "reject") {
		ASP_CHECK(arity == 0, "'reject' predicate does not have arity 0");
		assert(!rejectAtomKey);
		rejectAtomKey.reset(new Clasp::SymbolTable::key_type(symbolTableKey));
	}
}

}}} // namespace solver::asp::trees
