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

%skeleton "lalr1.cc"
%require "2.7.12"

%define parser_class_name {Parser}

%code requires
{
	#include <sstream>
	class Hypergraph;
	namespace parser {
		class Driver;
		class Terms;
	}
}

%parse-param { parser::Driver& driver }
%parse-param { Hypergraph& hypergraph }
%lex-param   { parser::Driver& driver }
%locations
%error-verbose

%code
{
	#include "../../src/parser/Driver.h"
	#include "../../src/parser/Terms.h"
}

%union
{
	std::string* string;
	parser::Terms* terms;
};

%token          END        0 "end of file"
%token <string> IDENTIFIER "identifier"
%token <string> NUMBER     "number"
%token <string> QSTRING    "qstring"
%type  <string> simpleterm term function
%type  <terms> terms

%destructor { delete $$; } "identifier" "number" "qstring" simpleterm term function terms

%%

%start facts;

simpleterm: "identifier" { $$ = $1; }
          | "number"     { $$ = $1; }
          | "qstring"    { $$ = $1; }
          ;

term: simpleterm { $$ = $1; }
    | function   { $$ = $1; }
    ;

terms: term           { $$ = new parser::Terms($1); }
     | terms ',' term {
                        $1->push_back($3);
                        $$ = $1;
                      }
     ;

function: "identifier" '(' terms ')' {
                                       std::ostringstream ss;
                                       ss << *$1 << '(' << *$3 << ')';
                                       $$ = new std::string(ss.str());
                                       delete $1;
                                       delete $3;
                                     }
        ;

fact: "identifier" '.'               {
                                       driver.processFact(hypergraph, *$1);
                                       delete $1;
                                     }
    | "identifier" '(' terms ')' '.' {
                                       driver.processFact(hypergraph, *$1, $3);
                                       delete $1;
                                       delete $3;
                                     }
    ;

facts: /* empty */
     | facts fact
     ;

%%

void yy::Parser::error(const yy::Parser::location_type& l, const std::string& m)
{
	driver.error(l, m);
}
