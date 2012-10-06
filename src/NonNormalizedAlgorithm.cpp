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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "NonNormalizedAlgorithm.h"
#include "Tuple.h"

using sharp::TupleTable;

NonNormalizedAlgorithm::NonNormalizedAlgorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* program, sharp::NormalizationType normalizationType, bool ignoreOptimization, unsigned int level)
	: Algorithm(problem, planFactory, instanceFacts, normalizationType, ignoreOptimization, level), program(program)
{
}

void NonNormalizedAlgorithm::declareBag(std::ostream& out, const sharp::ExtendedHypertree& node)
{
	// If there are no child nodes (i.e., this is a leaf) we provide a dummy child.
	out << "childNode(0)." << std::endl;
	for(unsigned i = 1; i < node.getChildren()->size(); ++i)
		out << "childNode(" << i << ")." << std::endl;

	foreach(sharp::Vertex v, node.getVertices())
		out << "current(" << problem.getVertexName(v) << ")." << std::endl;
	// TODO: Meaning of introduced and removed is not that clear with non-normalized TDs
//	foreach(sharp::Vertex v, node.getIntroducedVertices())
//		out << "introduced(" << problem.getVertexName(v) << ")." << std::endl;
//	foreach(sharp::Vertex v, node.getRemovedVertices())
//		out << "removed(" << problem.getVertexName(v) << ")." << std::endl;

	std::list<sharp::Hypertree*>::const_iterator it = node.getChildren()->begin();
	for(unsigned i = 0; it != node.getChildren()->end(); ++i) {
		foreach(sharp::Vertex v, dynamic_cast<sharp::ExtendedHypertree*>(*it)->getVertices())
			out << "childBag(" << i << ',' << problem.getVertexName(v) << ")." << std::endl;
		++it;
	}
	// XXX: Is this really necessary?
	out << "-introduced(X) :- childBag(_,X)." << std::endl;
	out << "introduced(X) :- current(X), not -introduced(X)." << std::endl;
	out << "removed(X) :- childBag(_,X), not current(X)." << std::endl;

	if(node.isRoot())
		out << "root." << std::endl;
}

void NonNormalizedAlgorithm::declareChildTables(std::ostream& out, const sharp::ExtendedHypertree& node, const std::vector<TupleTable*>& childTables)
{
	for(unsigned i = 0; i < childTables.size(); ++i)
		foreach(const TupleTable::value_type& tupleAndSolution, *childTables[i])
			dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(out, tupleAndSolution, i);
}

const char* NonNormalizedAlgorithm::getUserProgram(const sharp::ExtendedHypertree& node)
{
	return program;
}
