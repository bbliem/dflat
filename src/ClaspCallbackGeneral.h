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
#include <boost/container/map.hpp>

#include "Algorithm.h"
#include "Row.h"

class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ClaspCallbackGeneral : public Clasp::ClaspFacade::Callback
{
public:
	ClaspCallbackGeneral(const Algorithm& algorithm, sharp::Table& table, const GringoOutputProcessor& gringoOutput, const std::vector<sharp::Table*>& childTables)
		: algorithm(algorithm), table(table), gringoOutput(gringoOutput), childTables(childTables)
	{
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
	const std::vector<sharp::Table*>& childTables;

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

	// cf. GringoOutputProcessor.h
	struct ExtendAtom {
		unsigned int level;
		std::string extended;
		Clasp::Literal literal;

		ExtendAtom(unsigned int level, const std::string& extended, Clasp::Literal literal)
			: level(level), extended(extended), literal(literal)
		{
		}
	};
	std::vector<ExtendAtom> extendAtoms;

	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral levelsAtoms;
	LongToLiteral countAtoms;
	LongToLiteral currentCostAtoms;
	LongToLiteral costAtoms;

	// Because one table row can be constituted of multiple AS's, we cannot insert a new row upon arrival of a new AS but must rather collect all AS data until the solve state is finished.
	// By "path" we denote a path from root to leaf in Row::Tree. Each AS characterizes exactly one path.
	typedef std::vector<std::string> ExtendArguments;
	typedef std::pair<ExtendArguments, Row::Items> ExtendArgumentsAndItems;
	typedef std::vector<ExtendArgumentsAndItems> Path; // XXX: Check if vector isn't too inefficient because of reallocation when changing elements

	struct Tree
	{
		typedef boost::container::map<ExtendArgumentsAndItems, Tree> Children;
		Children children;
		// std::map won't work because Tree is an incomplete type at this time.
		// Cf. http://stackoverflow.com/questions/6527917/how-can-i-emulate-a-recursive-type-definition-in-c
		// Cf. http://www.boost.org/doc/libs/1_48_0/doc/html/container/containers_of_incomplete_types.html

		// XXX: Redundancy
		bool hasCount;
		long count;
		bool hasCurrentCost;
		long currentCost;
		bool hasCost;
		long cost;

		void insert(Path::iterator pathBegin, Path::iterator pathEnd, bool hasCount, long count, bool hasCurrentCost, long currentCost, bool hasCost, long cost);

		// Merge subtrees whose item sets are equal to obtain the characteristic of a row
		// XXX: Check if too much is being copied around due to storing stuff on the stack
		Row::Tree::Children mergeChildren() const;
	} tree;
};
