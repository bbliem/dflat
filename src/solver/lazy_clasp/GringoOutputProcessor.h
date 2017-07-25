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

#include "../../asp_utils.h"
#include "../../asp_utils/GringoOutputProcessor.h"

namespace solver { namespace lazy_clasp {

class GringoOutputProcessor : public asp_utils::GringoOutputProcessor
{
public:
	struct ItemAtomArguments {
		String item;
	};
	typedef asp_utils::GringoAtomInfo<ItemAtomArguments> ItemAtomInfo;

	struct AuxItemAtomArguments {
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
	
	struct CounterRemAtomArguments {
        std::string counterName;
	};
	typedef asp_utils::GringoAtomInfo<CounterRemAtomArguments> CounterRemAtomInfo;

	struct CounterIncAtomArguments {
        std::string counterName;
		long counterInc;
	};
	typedef asp_utils::GringoAtomInfo<CounterIncAtomArguments> CounterIncAtomInfo;

	struct CurrentCounterIncAtomArguments {
        std::string currentCounterName;
		long currentCounterInc;
	};
	typedef asp_utils::GringoAtomInfo<CurrentCounterIncAtomArguments> CurrentCounterIncAtomInfo;

	struct CounterAtomArguments {
        std::string counterName;
		long counter;
	};
	typedef asp_utils::GringoAtomInfo<CounterAtomArguments> CounterAtomInfo;

	struct CurrentCounterAtomArguments {
        std::string currentCounterName;
		long currentCounter;
	};
	typedef asp_utils::GringoAtomInfo<CurrentCounterAtomArguments> CurrentCounterAtomInfo;

	typedef std::vector<ItemAtomInfo>										ItemAtomInfos;
	typedef std::vector<AuxItemAtomInfo>									AuxItemAtomInfos;
	typedef std::vector<CurrentCostAtomInfo>								CurrentCostAtomInfos;
	typedef std::vector<CostAtomInfo>										CostAtomInfos;
	typedef std::vector<CounterRemAtomInfo>									CounterRemAtomInfos;
    typedef std::map<std::string,std::vector<CounterIncAtomInfo>>			AllCounterIncAtomInfos;
    typedef std::map<std::string,std::vector<CurrentCounterIncAtomInfo>>	AllCurrentCounterIncAtomInfos;
    typedef std::map<std::string,std::vector<CounterAtomInfo>>				AllCounterAtomInfos;
    typedef std::map<std::string,std::vector<CurrentCounterAtomInfo>>		AllCurrentCounterAtomInfos;

	GringoOutputProcessor(Clasp::Asp::LogicProgram& out);

	const ItemAtomInfos&					getItemAtomInfos()					const;
	const AuxItemAtomInfos&					getAuxItemAtomInfos()				const;
	const CurrentCostAtomInfos&				getCurrentCostAtomInfos()			const;
	const CostAtomInfos&					getCostAtomInfos()					const;
	const CounterRemAtomInfos&				getCounterRemAtomInfos()			const;
	const AllCounterIncAtomInfos&			getAllCounterIncAtomInfos()			const;
	const AllCurrentCounterIncAtomInfos&	getAllCurrentCounterIncAtomInfos()	const;
	const AllCounterAtomInfos&				getAllCounterAtomInfos()			const;
	const AllCurrentCounterAtomInfos&		getAllCurrentCounterAtomInfos()		const;

protected:
	virtual void storeAtom(unsigned int atomUid, Gringo::Value v) override;

	ItemAtomInfos					itemAtomInfos;
	AuxItemAtomInfos				auxItemAtomInfos;
	CurrentCostAtomInfos			currentCostAtomInfos;
	CostAtomInfos					costAtomInfos;
	CounterRemAtomInfos				counterRemAtomInfos;
	AllCounterIncAtomInfos			allCounterIncAtomInfos;
	AllCurrentCounterIncAtomInfos	allCurrentCounterIncAtomInfos;
	AllCounterAtomInfos				allCounterAtomInfos;
	AllCurrentCounterAtomInfos		allCurrentCounterAtomInfos;
};

}} // namespace solver::lazy_clasp
