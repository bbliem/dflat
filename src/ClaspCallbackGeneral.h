/*
Copyright 2012, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

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

#include <clasp/clasp_facade.h>

#include "Algorithm.h"
#include "Row.h"

class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ClaspCallbackGeneral : public Clasp::ClaspFacade::Callback
{
public:
	ClaspCallbackGeneral(const Algorithm& algorithm, sharp::Table& table, const GringoOutputProcessor& gringoOutput, unsigned int numChildNodes, unsigned int level)
		: algorithm(algorithm), table(table), gringoOutput(gringoOutput), numChildNodes(numChildNodes), numLevels(level)
	{
		assert(numLevels > 0);
	}

	// Called if the current configuration contains unsafe/unreasonable options
	virtual void warning(const char* msg);

	// Called on entering/exiting a state
	virtual void state(Clasp::ClaspFacade::Event, Clasp::ClaspFacade&);

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f);

private:
	const Algorithm& algorithm;
	sharp::Table& table;
	const GringoOutputProcessor& gringoOutput;
	const unsigned int numChildNodes;
	const unsigned int numLevels;

	// cf. GringoOutputProcessor.h
	struct ItemAtom {
		unsigned int level;
		std::string value;
		Clasp::Literal literal;
		ItemAtom(unsigned int level, const std::string& value, Clasp::Literal literal)
			: level(level), value(value), literal(literal)
		{}
	};
	std::vector<ItemAtom> itemAtoms;
	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral extendAtoms;
	LongToLiteral countAtoms;
	LongToLiteral currentCostAtoms;
	LongToLiteral costAtoms;

	// Because one table row can be constituted of multiple AS's, we cannot insert a new row upon arrival of a new AS but must rather collect all AS data until the solve state is finished.
	// By "path" we denote a path from root to leaf in Row::Tree. Each AS characterizes exactly one path.
	typedef std::vector<Row::Items> Path;
	typedef std::map<Row::Items, Row*> TopLevelItemsToRow; // Maps an item set to a row with that item set on the top level
	typedef std::map<Row::ExtensionPointerTuple, TopLevelItemsToRow> PredecessorData;

	PredecessorData predecessorData;
};
