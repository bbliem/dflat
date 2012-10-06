#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
//#include <boost/spirit/include/phoenix.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"

namespace threeCol {

namespace qi = boost::spirit::qi;

using std::vector;
using std::string;
using std::pair;
using sharp::Vertex;

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




Problem::Problem(std::istream& input)
	: input(input)
{
}

void Problem::declareVertex(std::ostream& out, Vertex v, const sharp::VertexSet& currentVertices) const
{
	if(currentVertices.find(v) == currentVertices.end())
		return;

	// Need to declare adjacent edges to vertices also in the current bag
	assert(adjacent.find(v) != adjacent.end()); // Since we only allow vertices with neighbors
	const std::set<sharp::Vertex>& neighbors = adjacent.find(v)->second;
	foreach(sharp::Vertex neighbor, currentVertices) {
		if(v < neighbor && neighbors.find(neighbor) != neighbors.end()) // "<" because edges are undirected)
			out << "edge(v" << v << ",v" << neighbor << ")." << std::endl;
	}
}

void Problem::parse()
{
	SkipperGrammar<boost::spirit::istream_iterator> skipper;
	ThreeColGrammar<boost::spirit::istream_iterator> threeColParser(skipper);

	input.unsetf(std::ios::skipws);
	boost::spirit::istream_iterator it(input);
	boost::spirit::istream_iterator end;

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
