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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Problem.h"

namespace qi = boost::spirit::qi;

namespace {
	// Skipper for comments and whitespace
	template <typename Iterator>
	struct SkipperGrammar : qi::grammar<Iterator>
	{
		SkipperGrammar() : SkipperGrammar::base_type(start)
		{
			start = qi::space | ('%' >> *(qi::char_ - qi::eol));
		}
		qi::rule<Iterator> start;
	};

	// Parses a set of facts
	template <typename Iterator>
	struct InstanceGrammar : qi::grammar<Iterator, SkipperGrammar<Iterator> >
	{
		typedef SkipperGrammar<Iterator> Skipper;

		InstanceGrammar(Problem& problem) : InstanceGrammar::base_type(start)
		{
			identifier = qi::char_("a-z") >> *qi::char_("A-Za-z0-9_");
//			fact = identifier >> -( qi::lit('(') >> (identifier % ',') >> ')' ) >> '.';
			argument = identifier | (*qi::char_("0-9"));
			fact = identifier >> -( qi::lit('(') >> (argument % ',') >> ')' ) >> '.';
			start = *fact[boost::bind(&Problem::parsedFact, &problem, _1)];
		}

		qi::rule<Iterator, std::string()> identifier; // Note the absence of the skipper
		qi::rule<Iterator, std::string()> argument; // Note the absence of the skipper
		qi::rule<Iterator, Skipper, Problem::Fact()> fact;
		qi::rule<Iterator, Skipper> start;
	};
}




Problem::Problem(const std::string& input, const std::set<std::string>& hyperedgePredicateNames)
	: input(input), hyperedgePredicateNames(hyperedgePredicateNames)
{
}

void Problem::parsedFact(const Problem::Fact& f)
{
	if(hyperedgePredicateNames.find(f.first) != hyperedgePredicateNames.end()) {
		sharp::VertexSet hyperedge;

		foreach(const std::string& arg, f.second) {
			sharp::Vertex v = storeVertexName(arg);
			vertices.insert(v);
			hyperedge.insert(v);
		}

		hyperedges.insert(hyperedge);
	}
}

void Problem::parse()
{
//	SkipperGrammar<boost::spirit::istream_iterator> skipper;
//	InstanceGrammar<boost::spirit::istream_iterator> instanceParser("foobar"); // TODO
//
//	input.unsetf(std::ios::skipws);
//	boost::spirit::istream_iterator it(input);
//	boost::spirit::istream_iterator end;

	SkipperGrammar<std::string::const_iterator> skipper;
	InstanceGrammar<std::string::const_iterator> instanceParser(*this);

	std::string::const_iterator it = input.begin();
	std::string::const_iterator end = input.end();

	bool result = qi::phrase_parse(
			it,
			end,
			instanceParser,
			skipper
			);

	if(!result || it != end)
		throw std::runtime_error("Parse error");
}

void Problem::preprocess()
{
}

sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	return createHypergraphFromSets(vertices, hyperedges);
}
