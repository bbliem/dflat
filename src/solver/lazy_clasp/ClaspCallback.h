/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include "../../asp_utils/ClaspCallback.h"
#include "GringoOutputProcessor.h"

namespace solver { namespace lazy_clasp {

class Solver;

// TODO actually this class does not really need to be a clasp callback
class ClaspCallback : public asp_utils::ClaspCallback
{
public:
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::ItemAtomArguments> ItemAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::AuxItemAtomArguments> AuxItemAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CurrentCostAtomArguments> CurrentCostAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CostAtomArguments> CostAtomInfo;

	ClaspCallback(const GringoOutputProcessor&, const Application&);

	void setItemTree(ItemTreePtr&& it);
	void setExtendedRows(const ItemTreeNode::ExtensionPointerTuple& e) { extendedRows = &e; }
	// onModel only inserts rows whose cost is below c
	void setCostBound(long c) { costBound = c; }
	// Results in itemTree->getChildren().end() if merging occurred for the last model, otherwise yields the row created by the last model
	ItemTree::Children::const_iterator getNewestRow() const;

	virtual bool onModel(const Clasp::Solver&, const Clasp::Model&) override;
	virtual void prepare(const Clasp::SymbolTable&) override;

private:
	std::vector<ItemAtomInfo>        itemAtomInfos;
	std::vector<AuxItemAtomInfo>     auxItemAtomInfos;
//	std::vector<CurrentCostAtomInfo> currentCostAtomInfos;
//	std::vector<CostAtomInfo>        costAtomInfos;

	const GringoOutputProcessor& gringoOutput;
	ItemTree::Children::const_iterator newestRow;
	const ItemTreeNode::ExtensionPointerTuple* extendedRows; // The predecessor rows that have given rise to the current call of the (lazy) ASP solver
	long costBound;
};

}} // namespace solver::lazy_clasp
