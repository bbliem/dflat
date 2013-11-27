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

#pragma once
//}}}
#include <set>

#include "parser.hpp"

#define YY_DECL                                \
 yy::Parser::token_type                        \
 yylex(yy::Parser::semantic_type* yylval,      \
       yy::Parser::location_type* yylloc,      \
       parser::Driver& driver)
YY_DECL;

namespace parser {

class Driver
{
public:
	typedef std::set<std::string> Predicates;

	Driver(const std::string& input, const Predicates& hyperedgePredicateNames);
	~Driver();
	void scan_begin();
	void scan_end();
	Hypergraph parse();
	void error(const yy::location& l, const std::string& m);
	void processFact(Hypergraph& hypergraph, const std::string& predicate, const Terms* arguments = 0);

private:
	const std::string& input;
	const Predicates& hyperedgePredicateNames;
};

} // namespace parser
