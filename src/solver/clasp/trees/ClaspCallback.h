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

#pragma once
//}}}
#include "../../../asp_utils/ClaspCallback.h"
#include "../../../UncompressedItemTree.h"
#include "GringoOutputProcessor.h"

class Decomposition;

namespace solver { namespace clasp { namespace trees {

class ClaspCallback : public asp_utils::ClaspCallback
{
public:
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::ExtendAtomArguments> ExtendAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::ItemAtomArguments> ItemAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::AuxItemAtomArguments> AuxItemAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CurrentCostAtomArguments> CurrentCostAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CostAtomArguments> CostAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::LengthAtomArguments> LengthAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::OrAtomArguments> OrAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::AndAtomArguments> AndAtomInfo;

	ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, const Application&, const Decomposition&);

	virtual bool onModel(const Clasp::Solver&, const Clasp::Model&) override;
	virtual void prepare(const Clasp::Asp::LogicProgram&) override;
	virtual ItemTreePtr finalize(bool pruneUndefined, bool pruneRejecting) override;

private:
	UncompressedItemTreePtr uncompressedItemTree;

	std::vector<ItemAtomInfo>           itemAtomInfos;
	std::vector<AuxItemAtomInfo>        auxItemAtomInfos;
	std::vector<ExtendAtomInfo>         extendAtomInfos;
	std::vector<CurrentCostAtomInfo>    currentCostAtomInfos;
	std::vector<CostAtomInfo>           costAtomInfos;
	std::vector<LengthAtomInfo>         lengthAtomInfos;
	std::vector<OrAtomInfo>             orAtomInfos;
	std::vector<AndAtomInfo>            andAtomInfos;

	std::unique_ptr<Clasp::Literal> acceptLiteral;
	std::unique_ptr<Clasp::Literal> rejectLiteral;

	const GringoOutputProcessor& gringoOutput;
	const ChildItemTrees& childItemTrees;
	const Decomposition& decomposition;
};

}}} // namespace solver::clasp::trees
