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
#include <mutex>

#include "../asp/ClaspCallback.h"

namespace solver { namespace lazy_asp {

class Solver;

class ClaspCallback : public ::solver::asp::ClaspCallback
{
public:
	struct ItemAtomArguments {
		std::string item;
	};
	typedef AtomInfo<ItemAtomArguments> ItemAtomInfo;

	struct AuxItemAtomArguments {
		std::string item;
	};
	typedef AtomInfo<AuxItemAtomArguments> AuxItemAtomInfo;

	struct CurrentCostAtomArguments {
		long currentCost;
	};
	typedef AtomInfo<CurrentCostAtomArguments> CurrentCostAtomInfo;

	struct CostAtomArguments {
		long cost;
	};
	typedef AtomInfo<CostAtomArguments> CostAtomInfo;

	ClaspCallback(const Application&, Solver&, std::unique_lock<std::mutex>& lock);

//	virtual void storeAtom(unsigned int atomUid, Gringo::Value v) override;

	void setRootExtensionPointers(ItemTreeNode::ExtensionPointerTuple&&);
	void setExtendedRows(ItemTreeNode::ExtensionPointerTuple&&);
	ItemTree::Children::const_iterator getNewestRow() const;

	virtual bool onModel(const Clasp::Solver&, const Clasp::Model&) override;

private:
	std::vector<ItemAtomInfo>           itemAtomInfos;
	std::vector<AuxItemAtomInfo>        auxItemAtomInfos;
	std::vector<CurrentCostAtomInfo>    currentCostAtomInfos;
	std::vector<CostAtomInfo>           costAtomInfos;

	ItemTree::Children::const_iterator newestRow;
	Solver& solver;
	ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
	ItemTreeNode::ExtensionPointerTuple extendedRows; // The predecessor rows that have given rise to the current call of the (lazy) ASP solver

	std::unique_lock<std::mutex>& lock;
};

}} // namespace solver::lazy_asp
