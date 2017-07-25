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

%skeleton "lalr1.cc"
%require "3.0"

// Create parser header file for the scanner
%defines
%define api.namespace {parser}
%define parser_class_name {Parser}

%code requires
{
	#include <sstream>
	class Instance;
	namespace parser {
		class Driver;
		class Scanner;
	}
}

%parse-param { parser::Scanner& scanner }
%parse-param { parser::Driver& driver }
%parse-param { Instance& instance }
%locations
%error-verbose

%code
{
#include "../src/parser/Driver.h"

#undef yylex
#define yylex scanner.yylex
}

%define api.value.type variant
%define parse.assert

%token          END        0 "end of file"
%token <std::string> IDENTIFIER "identifier"
%token <std::string> NUMBER     "number"
%token <std::string> QSTRING    "qstring"
%type  <std::string> simpleterm term function
%type  <std::vector<std::string>> terms

%%

%start facts;

simpleterm: "identifier" { $$ = $1; }
          | "number"     { $$ = $1; }
          | "qstring"    { $$ = $1; }
          ;

term: simpleterm { $$ = $1; }
    | function   { $$ = $1; }
    ;

terms: term           { $$.push_back($1); }
     | terms ',' term {
                        $1.push_back($3);
                        $$ = $1;
                      }
     ;

function: "identifier" '(' terms ')' {
                                       std::ostringstream ss;
                                       char sep = '(';
                                       for(const std::string& arg : $3) {
                                           ss << sep << arg;
                                           sep = ',';
                                       }
                                       ss << ')';
                                       $$ = ss.str();
                                     }
        ;

fact: "identifier" '.'               {
                                       driver.processFact(instance, $1, {});
                                     }
    | "identifier" '(' terms ')' '.' {
                                       driver.processFact(instance, $1, $3);
                                     }
    ;

facts: /* empty */
     | facts fact
     ;

%%

void parser::Parser::error(const location_type& l, const std::string& m)
{
	std::ostringstream ss;
	ss << "Parse error." << std::endl << l << ": " << m;
	throw std::runtime_error(ss.str());
}
