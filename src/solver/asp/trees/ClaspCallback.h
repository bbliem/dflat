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

#pragma once
//}}}
#include "../ClaspCallback.h"
#include "GringoOutputProcessor.h"
#include "UncompressedItemTree.h"

namespace solver { namespace asp { namespace trees {

class ClaspCallback : public ::solver::asp::ClaspCallback
{
public:
	typedef AtomInfo<GringoOutputProcessor::ItemAtomArguments>           ItemAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::AuxItemAtomArguments>        AuxItemAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::ExtendAtomArguments>         ExtendAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::CurrentCostAtomArguments>    CurrentCostAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::CostAtomArguments>           CostAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::LengthAtomArguments>         LengthAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::OrAtomArguments>             OrAtomInfo;
	typedef AtomInfo<GringoOutputProcessor::AndAtomArguments>            AndAtomInfo;

	typedef std::vector<ItemAtomInfo>           ItemAtomInfos;
	typedef std::vector<AuxItemAtomInfo>        AuxItemAtomInfos;
	typedef std::vector<ExtendAtomInfo>         ExtendAtomInfos;
	typedef std::vector<CurrentCostAtomInfo>    CurrentCostAtomInfos;
	typedef std::vector<CostAtomInfo>           CostAtomInfos;
	typedef std::vector<LengthAtomInfo>         LengthAtomInfos;
	typedef std::vector<OrAtomInfo>             OrAtomInfos;
	typedef std::vector<AndAtomInfo>            AndAtomInfos;

	ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, bool prune, const Application& app);

	// Called on entering/exiting a state
	virtual void state(Clasp::ClaspFacade::Event, Clasp::ClaspFacade&) override;

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f) override;

protected:
	UncompressedItemTreePtr uncompressedItemTree;

	const GringoOutputProcessor& gringoOutput;

	ItemAtomInfos           itemAtomInfos;
	AuxItemAtomInfos        auxItemAtomInfos;
	ExtendAtomInfos         extendAtomInfos;
	CurrentCostAtomInfos    currentCostAtomInfos;
	CostAtomInfos           costAtomInfos;
	LengthAtomInfos         lengthAtomInfos;
	OrAtomInfos             orAtomInfos;
	AndAtomInfos            andAtomInfos;

	std::unique_ptr<Clasp::Literal> acceptLiteral;
	std::unique_ptr<Clasp::Literal> rejectLiteral;

	bool prune;
};

}}} // namespace solver::asp::trees
