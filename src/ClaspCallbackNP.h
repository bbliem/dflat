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

class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ClaspCallbackNP : public Clasp::ClaspFacade::Callback
{
public:
#ifdef DISABLE_ANSWER_SET_CHECKS
	ClaspCallbackNP(const Algorithm& algorithm, sharp::TupleTable& tupleTable, const GringoOutputProcessor& gringoOutput)
		: algorithm(algorithm), tupleTable(tupleTable), gringoOutput(gringoOutput)
	{}
#else
	ClaspCallbackNP(const Algorithm& algorithm, sharp::TupleTable& tupleTable, const GringoOutputProcessor& gringoOutput, const std::set<std::string>& currentVertices)
		: algorithm(algorithm), tupleTable(tupleTable), gringoOutput(gringoOutput), currentVertices(currentVertices)
	{}
#endif

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

	// cf. GringoOutputProcessor.h
	struct MapAtom {
		std::string vertex;
		std::string value;
		Clasp::Literal literal;
		MapAtom(const std::string& vertex, const std::string& value, Clasp::Literal literal)
			: vertex(vertex), value(value), literal(literal)
		{}
	};
	std::vector<MapAtom> mapAtoms;
	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral chosenChildTupleAtoms;
	LongToLiteral chosenChildTupleLAtoms;
	LongToLiteral chosenChildTupleRAtoms;
	LongToLiteral currentCostAtoms;
	LongToLiteral costAtoms;

#ifndef DISABLE_ANSWER_SET_CHECKS
	std::set<std::string> currentVertices; // To check if all vertices are assigned
#endif
};
