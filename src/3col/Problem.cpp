#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
//#include <boost/spirit/include/phoenix.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"

namespace threeCol {

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

	template <typename Iterator>
	struct ThreeColGrammar : qi::grammar<Iterator, SkipperGrammar<Iterator>, Problem::Graph()>
	{
		typedef SkipperGrammar<Iterator> Skipper;

		ThreeColGrammar(Skipper& skipper) : ThreeColGrammar::base_type(start)
		{
			nodeName = qi::char_("a-z") >> *qi::char_("A-Za-z0-9_");

//			node = "node" >> qi::lit('(') >> nodeName >> ')' >> '.';
			edge = "edge" >> qi::lit('(') >> nodeName >> ',' >> nodeName >> ')' >> '.';

//			using qi::_1;
//			using qi::_val;
//			using boost::phoenix::at_c;
//			using boost::phoenix::push_back;
//			start = *(
//					node [push_back(at_c<0>(_val), _1)]
//					|
//					edge [push_back(at_c<1>(_val), _1)]
//					);
			start = *edge;
		}

		qi::rule<Iterator, Problem::Node()> nodeName;
//		qi::rule<Iterator, Skipper, Problem::Node()> node;
		qi::rule<Iterator, Skipper, Problem::Edge()> edge;
		qi::rule<Iterator, Skipper, Problem::Graph()> start;
	};
}




Problem::Problem(const std::string& input)
	: input(input)
{
}

void Problem::parse()
{
	SkipperGrammar<std::string::const_iterator> skipper;
	ThreeColGrammar<std::string::const_iterator> threeColParser(skipper);

	std::string::const_iterator it = input.begin();
	std::string::const_iterator end = input.end();

	bool result = qi::phrase_parse(
			it,
			end,
			threeColParser,
			skipper,
			graph
			);

	if(!result || it != end)
		throw std::runtime_error("Parse error");
}

void Problem::preprocess()
{
}

sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	sharp::VertexSet nodes;
	sharp::EdgeSet edges;

	foreach(const Edge& e, graph) {
		sharp::Vertex from = storeVertexName(e.first);
		sharp::Vertex to = storeVertexName(e.second);
		adjacent[from].insert(to);
		adjacent[to].insert(from);
		nodes.insert(from);
		nodes.insert(to);
		edges.insert(std::make_pair(from, to));
	}

	return createGraphFromSets(nodes, edges);
}

} // namespace threeCol
