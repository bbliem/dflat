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
#include "Tuple.h"

class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ClaspCallbackGeneral : public Clasp::ClaspFacade::Callback
{
public:
#ifdef DISABLE_ANSWER_SET_CHECKS
	ClaspCallbackGeneral(const Algorithm& algorithm, sharp::TupleTable& tupleTable, const GringoOutputProcessor& gringoOutput, unsigned int level)
		: algorithm(algorithm), tupleTable(tupleTable), gringoOutput(gringoOutput), numLevels(level)
#else
	ClaspCallbackGeneral(const Algorithm& algorithm, sharp::TupleTable& tupleTable, const GringoOutputProcessor& gringoOutput, unsigned int level, const std::set<std::string>& currentVertices)
		: algorithm(algorithm), tupleTable(tupleTable), gringoOutput(gringoOutput), numLevels(level), currentVertices(currentVertices)
#endif
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
	sharp::TupleTable& tupleTable;
	const GringoOutputProcessor& gringoOutput;
	const unsigned int numLevels;

	// cf. GringoOutputProcessor.h
	struct MapAtom {
		unsigned int level;
		std::string vertex;
		std::string value;
		Clasp::Literal literal;
		MapAtom(unsigned int level, const std::string& vertex, const std::string& value, Clasp::Literal literal)
			: level(level), vertex(vertex), value(value), literal(literal)
		{}
	};
	std::vector<MapAtom> mapAtoms;
	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral chosenChildTupleAtoms;
	LongToLiteral chosenChildTupleLAtoms;
	LongToLiteral chosenChildTupleRAtoms;
	LongToLiteral currentCostAtoms;
	LongToLiteral costAtoms;

	// Because one tuple can be constituted of multiple AS's, we cannot insert new tuples upon arrival of a new AS but must rather collect all AS data until the solve state is finished.
	// By "path" we denote a path from root to leaf in Tuple::Tree. Each AS characterizes exactly one path.
	typedef std::vector<Tuple::Assignment> Path;

	class PathCollection
	{
	public:
		typedef sharp::TupleTable::value_type TableRow;

		void insert(const Path& path, const TableRow* leftPredecessor = 0, const TableRow* rightPredecessor = 0, unsigned currentCost = 0, unsigned cost = 0);
		void fillTupleTable(sharp::TupleTable& tupleTable, const Algorithm& algorithm) const;

	private:
		struct TupleData
		{
			std::list<Path> paths;
			unsigned int currentCost;
			unsigned int cost;
		};

		typedef std::pair<const TableRow*, const TableRow*> TableRowPair; // When using the join program, both are used; when using the exchange program, only the first is used, the other is 0.
		typedef std::map<Tuple::Assignment, TupleData> TopLevelAssignmentToTupleData; // Maps an assignment to data of tuples starting with that assignment
		typedef std::map<TableRowPair, TopLevelAssignmentToTupleData> PredecessorData;

		PredecessorData predecessorData;
	};

	PathCollection pathCollection;

#ifndef DISABLE_ANSWER_SET_CHECKS
	std::set<std::string> currentVertices; // To check if all vertices are assigned
#endif
};
