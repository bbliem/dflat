/*{{{
Copyright 2016, Bernhard Bliem
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
#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif
#include <set>
#include "parser.hpp"
#include "location.hh"

namespace parser {

class Scanner : public yyFlexLexer
{
public:
	typedef std::set<std::string> Predicates;

	Scanner(std::istream* in)
		: yyFlexLexer(in)
	{
		loc = new Parser::location_type;
	}

	virtual ~Scanner()
	{
		delete loc;
	}

	virtual int yylex(Parser::semantic_type* const lval, Parser::location_type* loc);

private:
	Parser::semantic_type* yylval;
	Parser::location_type* loc;
};

} // namespace parser
