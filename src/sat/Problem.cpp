#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"

namespace sat {

namespace qi = boost::spirit::qi;

using std::vector;
using std::string;
using std::pair;
using sharp::Vertex;

namespace {
	void insertPos(Problem::Instance& instance, const std::string& clause, const std::string& atom)
	{
		instance[clause].first.insert(atom);
	}
	void insertNeg(Problem::Instance& instance, const std::string& clause, const std::string& atom)
	{
		instance[clause].second.insert(atom);
	}

	template <typename Iterator>
	struct InstanceGrammar : qi::grammar<Iterator, Problem::Instance()>
	{
		InstanceGrammar() : InstanceGrammar::base_type(start)
		{
			identifier = qi::char_("a-z") >> *qi::char_("A-Za-z0-9_");

			pos = "pos" >> qi::lit('(') >> identifier >> ',' >> identifier >> ')' >> '.';
			neg = "neg" >> qi::lit('(') >> identifier >> ',' >> identifier >> ')' >> '.';

			using qi::_1;
			using qi::_val;
			using boost::phoenix::at_c;
			using boost::phoenix::push_back;
			start = *(
					pos [boost::phoenix::bind(insertPos, _val, at_c<0>(_1), at_c<1>(_1))]
					|
					neg [boost::phoenix::bind(insertNeg, _val, at_c<0>(_1), at_c<1>(_1))]
					);
		}

		qi::rule<Iterator, std::string()> identifier;
		qi::rule<Iterator, std::pair<std::string,std::string>()> pos;
		qi::rule<Iterator, std::pair<std::string,std::string>()> neg;
		qi::rule<Iterator, Problem::Instance()> start;
	};
}




Problem::Problem(const std::string& input)
	: input(input)
{
}

void Problem::parse()
{
	InstanceGrammar<std::string::const_iterator> instanceParser;

	std::string::const_iterator it = input.begin();
	std::string::const_iterator end = input.end();

	bool result = qi::phrase_parse(
			it,
			end,
			instanceParser,
			boost::spirit::qi::ascii::space,
			instance
			);

	if(!result || it != end)
		throw std::runtime_error("Parse error");
}

void Problem::preprocess()
{
}

sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	sharp::VertexSet clauseVertices;
	sharp::VertexSet atomVertices;
	sharp::EdgeSet edges;

	// XXX: Probably this is the wrong location for storing vertex names. Ideally we would do this during parsing
	foreach(const Instance::value_type& clause, instance) {
		sharp::Vertex clauseVertex = storeVertexName(clause.first);
		clauseVertices.insert(clauseVertex);

		// Positive atoms
		foreach(const std::string& atom, clause.second.first) {
			sharp::Vertex v = storeVertexName(atom);
			atomVertices.insert(v);
			edges.insert(std::make_pair(clauseVertex, v));
		}
		// Negative atoms
		foreach(const std::string& atom, clause.second.second) {
			sharp::Vertex v = storeVertexName(atom);
			atomVertices.insert(v);
			edges.insert(std::make_pair(clauseVertex, v));
		}
	}

	return createGraphFromDisjointSets(clauseVertices, atomVertices, edges);
}

} // namespace sat
