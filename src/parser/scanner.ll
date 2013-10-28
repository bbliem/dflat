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

%{
	#include "../../src/parser/Driver.h"
	#include "parser.hpp"
	#define yyterminate() return token::END
	// Silence a Clang warning about yyinput() being unused
	#define YY_NO_INPUT
%}

%option noyywrap nounput batch

identifier    [a-z][a-zA-Z_0-9]*
number        -?[0-9]+
qstring       \"[^"]*\"
blank         [ \t]
comment       %.*

%{
	#define YY_USER_ACTION yylloc->columns(yyleng);
%}

%%

%{
	yylloc->step();
%}

{blank}+   yylloc->step();
{comment}  yylloc->step();
[\n]+      yylloc->lines(yyleng); yylloc->step();

%{
	typedef yy::Parser::token token;
%}

[(),.] return yy::Parser::token_type(yytext[0]);

{identifier} {
	yylval->string = new std::string(yytext);
	return token::IDENTIFIER;
}

{number} {
	yylval->string = new std::string(yytext);
	return token::NUMBER;
}

{qstring} {
	yylval->string = new std::string(yytext);
	return token::QSTRING;
}

. driver.error(*yylloc, "invalid character");

%%

namespace parser {

void Driver::scan_begin()
{
	yy_scan_string(input.c_str());
}

void Driver::scan_end()
{
	yylex_destroy();
}

} // namespace parser
