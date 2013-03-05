/*
Copyright 2012-2013, Bernhard Bliem
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

#include <sharp/main>

class Problem : public sharp::Problem
{
public:
	Problem(const std::string& input, const std::set<std::string>& hyperedgePredicateNames);

	typedef std::pair<std::string, std::vector<std::string> > Fact; // Predicate name and (optional) arguments

	// To be used by the parser. Do not call directly.
	void parsedFact(const Fact&);

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	const std::string& input;
	const std::set<std::string>& hyperedgePredicateNames;
	sharp::VertexSet vertices;
	sharp::HyperedgeSet hyperedges;
};
