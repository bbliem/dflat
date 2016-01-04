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
#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#include "String.h"

class Instance
{
public:
	typedef String Vertex;
	typedef std::vector<Vertex> Edge;
	typedef std::set<Edge> Edges;
	// Key: Edge predicate; value: One tuple of arguments
	typedef std::unordered_map<std::string, Edges> EdgeFacts;
	typedef std::vector<std::string> NonEdgeFacts;

	const Edges& getEdgeFactsOfPredicate(const std::string& predicate) const
	{
		const EdgeFacts::const_iterator it = edgeFacts.find(predicate);
		return it == edgeFacts.end() ? emptyEdgeSet : it->second;
	}
	const EdgeFacts& getEdgeFacts() const { return edgeFacts; }
	const NonEdgeFacts& getNonEdgeFacts() const { return nonEdgeFacts; }

	void addEdgeFact(const std::string& predicate, const Edge& e) { edgeFacts[predicate].insert(e); }
	void addNonEdgeFact(const std::string& fact) { nonEdgeFacts.push_back(fact); }

private:
	EdgeFacts edgeFacts;
	Edges emptyEdgeSet; // always empty, to be returned by getEdgeFactsOfPredicate() for unknown predicate
	NonEdgeFacts nonEdgeFacts;
};
