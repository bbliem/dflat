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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "parser/Driver.h"
#include "parser/Terms.h"
#include "Problem.h"

Problem::Problem(const std::string& input, const std::set<std::string>& hyperedgePredicateNames)
	: input(input), hyperedgePredicateNames(hyperedgePredicateNames)
{
}

void Problem::parsedFact(const std::string& predicate, const parser::Terms* arguments)
{
	if(hyperedgePredicateNames.find(predicate) != hyperedgePredicateNames.end()) {
		sharp::VertexSet hyperedge;

		if(arguments) {
			foreach(const std::string* arg, arguments->getTerms()) {
				sharp::Vertex v = storeVertexName(*arg);
				vertices.insert(v);
				hyperedge.insert(v);
			}
		}

		hyperedges.insert(hyperedge);
	}
}

void Problem::parse()
{
	parser::Driver driver(*this, input);
	driver.parse();
}

void Problem::preprocess()
{
}

sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	return createHypergraphFromSets(vertices, hyperedges);
}
