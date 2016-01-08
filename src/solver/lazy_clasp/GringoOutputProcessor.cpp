/*{{{
Copyright 2012-2016, Bernhard Bliem
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

namespace solver { namespace lazy_clasp {

GringoOutputProcessor::GringoOutputProcessor(Clasp::Asp::LogicProgram& out)
	: asp_utils::GringoOutputProcessor(out)
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

const GringoOutputProcessor::CurrentCostAtomInfos& GringoOutputProcessor::getCurrentCostAtomInfos() const
{
	return currentCostAtomInfos;
}

const GringoOutputProcessor::CostAtomInfos& GringoOutputProcessor::getCostAtomInfos() const
{
	return costAtomInfos;
}

void GringoOutputProcessor::storeAtom(unsigned int atomUid, Gringo::Value v)
{
	// Store the atom together with its symbol table key and extracted arguments
	const std::string& predicate = *v.name();
	if(predicate == "item") {
		ASP_CHECK(v.args().size() == 1, "'item' predicate does not have arity 1");
		std::ostringstream argument;
		v.args().front().print(argument);
		itemAtomInfos.emplace_back(ItemAtomInfo{ItemAtomArguments{argument.str()}, atomUid});
	} else if(predicate == "auxItem") {
		ASP_CHECK(v.args().size() == 1, "'auxItem' predicate does not have arity 1");
		std::ostringstream argument;
		v.args().front().print(argument);
		auxItemAtomInfos.emplace_back(AuxItemAtomInfo{AuxItemAtomArguments{argument.str()}, atomUid});
	} else if(predicate == "currentCost") {
		ASP_CHECK(v.args().size() == 1, "'currentCost' predicate does not have arity 1");
		std::ostringstream argument;
		v.args().front().print(argument);
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo{{std::stol(argument.str())}, atomUid});
	} else if(predicate == "cost") {
		ASP_CHECK(v.args().size() == 1, "'cost' predicate does not have arity 1");
		std::ostringstream argument;
		v.args().front().print(argument);
		costAtomInfos.emplace_back(CostAtomInfo{{std::stol(argument.str())}, atomUid});
	}
}

}} // namespace solver::lazy_clasp
