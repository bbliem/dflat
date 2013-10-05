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

GringoOutputProcessor::GringoOutputProcessor()
	: ::solver::asp::GringoOutputProcessor()
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
		std::string argument = getArguments(firstArg, arity).front();
		// Child node number is before the first '_' (and after the leading 'n')
		// Row number is after the first '_'
		unsigned int underscorePos = argument.find('_');
		unsigned int childId = std::stoi(std::string(argument, 1, underscorePos-1));
		unsigned int rowNumber = std::stoi(std::string(argument, underscorePos + 1));

		extendAtomInfos.emplace_back(ExtendAtomInfo{{childId, rowNumber}, symbolTableKey});
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
