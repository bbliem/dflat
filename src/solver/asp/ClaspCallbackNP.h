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

#pragma once

#include <unordered_map>
#include <clasp/clasp_facade.h>

#include "../../ItemTree.h"

namespace solver { namespace asp {

class GringoOutputProcessor;
class ItemTreeBranchLookupTable;

// Gets called by clasp whenever a model has been found
class ClaspCallbackNP : public Clasp::ClaspFacade::Callback
{
public:
	typedef std::unordered_map<unsigned int, ItemTreeBranchLookupTable> MapChildIdToBranches;

	ClaspCallbackNP(const GringoOutputProcessor& gringoOutput, const MapChildIdToBranches& itemTreeBranchLookupTables);

	// Call this after all answer sets have been processed. The contained item tree is moved.
	ItemTree&& getItemTree();

	// Called if the current configuration contains unsafe/unreasonable options
	virtual void warning(const char* msg);

	// Called on entering/exiting a state
	virtual void state(Clasp::ClaspFacade::Event, Clasp::ClaspFacade&);

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f);

private:
	ItemTree itemTree;
	const GringoOutputProcessor& gringoOutput;
	const MapChildIdToBranches& itemTreeBranchLookupTables;

	// cf. GringoOutputProcessor.h
	struct ItemAtom {
		std::string value;
		Clasp::Literal literal;
		ItemAtom(const std::string& value, Clasp::Literal literal)
			: value(value), literal(literal)
		{}
	};
	std::vector<ItemAtom> itemAtoms;

	typedef std::map<std::string, Clasp::Literal> StringToLiteral;
	StringToLiteral extendAtoms;

	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral countAtoms;
	LongToLiteral currentCostAtoms;
	LongToLiteral costAtoms;
};

}} // namespace solver::asp
