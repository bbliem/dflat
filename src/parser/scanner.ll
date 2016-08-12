/*
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

%{
#include <sstream>
#include "../../src/parser/Scanner.h"

#undef YY_DECL
#define YY_DECL int parser::Scanner::yylex(Parser::semantic_type* const lval, Parser::location_type* loc)

typedef parser::Parser::token token;

#define yyterminate() return token::END

// Update location on matching
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
%}

%option c++ noyywrap batch nounput yyclass="parser::Scanner"

identifier    [a-z][a-zA-Z_0-9]*
number        -?[0-9]+
qstring       \"[^"]*\"
blank         [ \t]
comment       %.*

%%

%{ // Code executed at the beginning of yylex
	yylval = lval;
%}

{blank}+   ;
{comment}  ;
[\n]+      loc->lines(yyleng);

[(),.] return Parser::token_type(yytext[0]);

{identifier} {
	yylval->build<std::string>(yytext);
	return token::IDENTIFIER;
}

{number} {
	yylval->build<std::string>(yytext);
	return token::NUMBER;
}

{qstring} {
	yylval->build<std::string>(yytext);
	return token::QSTRING;
}


. {
	std::ostringstream ss;
	ss << "Parse error." << std::endl << *loc << ": Invalid character";
	throw std::runtime_error(ss.str());
}
%%
