/*{{{
Copyright 2012-2014, Bernhard Bliem
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

#include "../clasp/GringoOutputProcessor.h"

namespace solver { namespace lazy_clasp {

class GringoOutputProcessor : public ::solver::clasp::GringoOutputProcessor
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

	typedef std::vector<ItemAtomInfo>           ItemAtomInfos;
	typedef std::vector<AuxItemAtomInfo>        AuxItemAtomInfos;
	typedef std::vector<CurrentCostAtomInfo>    CurrentCostAtomInfos;
	typedef std::vector<CostAtomInfo>           CostAtomInfos;

	GringoOutputProcessor(Clasp::Asp::LogicProgram& out);

	const ItemAtomInfos&        getItemAtomInfos()           const;
	const AuxItemAtomInfos&     getAuxItemAtomInfos()        const;
	const CurrentCostAtomInfos& getCurrentCostAtomInfos()    const;
	const CostAtomInfos&        getCostAtomInfos()           const;

protected:
	virtual void storeAtom(unsigned int atomUid, Gringo::Value v) override;

	ItemAtomInfos           itemAtomInfos;
	AuxItemAtomInfos        auxItemAtomInfos;
	CurrentCostAtomInfos    currentCostAtomInfos;
	CostAtomInfos           costAtomInfos;
};

}} // namespace solver::lazy_clasp
