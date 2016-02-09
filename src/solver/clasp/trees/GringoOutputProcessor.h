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
#include <memory>

#include "../../../asp_utils.h"
#include "../../../asp_utils/GringoOutputProcessor.h"

namespace solver { namespace clasp { namespace trees {

class GringoOutputProcessor : public asp_utils::GringoOutputProcessor
{
public:
	struct ExtendAtomArguments {
		unsigned int level;
		unsigned int decompositionNodeId;
		std::weak_ptr<ItemTreeNode> extendedNode;
	};
	typedef asp_utils::GringoAtomInfo<ExtendAtomArguments> ExtendAtomInfo;

	struct ItemAtomArguments {
		unsigned int level;
		String item;
	};
	typedef asp_utils::GringoAtomInfo<ItemAtomArguments> ItemAtomInfo;

	struct AuxItemAtomArguments {
		unsigned int level;
		String item;
	};
	typedef asp_utils::GringoAtomInfo<AuxItemAtomArguments> AuxItemAtomInfo;

	struct CurrentCostAtomArguments {
		long currentCost;
	};
	typedef asp_utils::GringoAtomInfo<CurrentCostAtomArguments> CurrentCostAtomInfo;

	struct CostAtomArguments {
		long cost;
	};
	typedef asp_utils::GringoAtomInfo<CostAtomArguments> CostAtomInfo;

	struct LengthAtomArguments {
		unsigned int length;
	};
	typedef asp_utils::GringoAtomInfo<LengthAtomArguments> LengthAtomInfo;

	struct OrAtomArguments {
		unsigned int level;
	};
	typedef asp_utils::GringoAtomInfo<OrAtomArguments> OrAtomInfo;

	struct AndAtomArguments {
		unsigned int level;
	};
	typedef asp_utils::GringoAtomInfo<AndAtomArguments> AndAtomInfo;

	typedef std::vector<ItemAtomInfo>           ItemAtomInfos;
	typedef std::vector<AuxItemAtomInfo>        AuxItemAtomInfos;
	typedef std::vector<ExtendAtomInfo>         ExtendAtomInfos;
	typedef std::vector<CurrentCostAtomInfo>    CurrentCostAtomInfos;
	typedef std::vector<CostAtomInfo>           CostAtomInfos;
	typedef std::vector<LengthAtomInfo>         LengthAtomInfos;
	typedef std::vector<OrAtomInfo>             OrAtomInfos;
	typedef std::vector<AndAtomInfo>            AndAtomInfos;

	GringoOutputProcessor(Clasp::Asp::LogicProgram& out, const ChildItemTrees& childItemTrees);

	const ItemAtomInfos&           getItemAtomInfos()           const;
	const AuxItemAtomInfos&        getAuxItemAtomInfos()        const;
	const ExtendAtomInfos&         getExtendAtomInfos()         const;
	const CurrentCostAtomInfos&    getCurrentCostAtomInfos()    const;
	const CostAtomInfos&           getCostAtomInfos()           const;
	const LengthAtomInfos&         getLengthAtomInfos()         const;
	const OrAtomInfos&             getOrAtomInfos()             const;
	const AndAtomInfos&            getAndAtomInfos()            const;

	const Clasp::Var* getAcceptAtomKey() const;
	const Clasp::Var* getRejectAtomKey() const;

protected:
	virtual void storeAtom(unsigned int atomUid, Gringo::Value v) override;

	ItemAtomInfos           itemAtomInfos;
	AuxItemAtomInfos        auxItemAtomInfos;
	ExtendAtomInfos         extendAtomInfos;
	CurrentCostAtomInfos    currentCostAtomInfos;
	CostAtomInfos           costAtomInfos;
	LengthAtomInfos         lengthAtomInfos;
	OrAtomInfos             orAtomInfos;
	AndAtomInfos            andAtomInfos;

	std::unique_ptr<Clasp::Var> acceptAtomKey;
	std::unique_ptr<Clasp::Var> rejectAtomKey;

	const ChildItemTrees& childItemTrees;
};

}}} // namespace solver::clasp::trees
