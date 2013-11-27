/*{{{
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
//}}}
#include <stdexcept>

#include "Driver.h"
#include "Terms.h"
#include "../Hypergraph.h"

namespace parser {

Driver::Driver(const std::string& input, const Predicates& hyperedgePredicateNames)
	: input(input)
	, hyperedgePredicateNames(hyperedgePredicateNames)
{
}

Driver::~Driver()
{
}

Hypergraph Driver::parse()
{
	Hypergraph hypergraph;
	scan_begin();
	::yy::Parser parser(*this, hypergraph);
	int res = parser.parse();
	scan_end();
	if(res != 0)
		throw std::runtime_error("Parse error");
	return hypergraph;
}

void Driver::error(const yy::location& l, const std::string& m)
{
	std::ostringstream ss;
	ss << "Parse error." << std::endl << l << ": " << m;
	throw std::runtime_error(ss.str());
}

void Driver::processFact(Hypergraph& hypergraph, const std::string& predicate, const Terms* arguments)
{
	if(hyperedgePredicateNames.find(predicate) != hyperedgePredicateNames.end()) {
		Hypergraph::Edge hyperedge;

		if(arguments) {
			for(const auto* term : arguments->getTerms()) {
				hypergraph.addVertex(*term);
				hyperedge.insert(*term);
			}
		}

		hypergraph.addEdge(hyperedge);
	}
}

} // namespace parser
