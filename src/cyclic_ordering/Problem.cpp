#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"

namespace cyclic_ordering {

namespace qi = boost::spirit::qi;

namespace {
	template <typename Iterator>
	struct InstanceGrammar : qi::grammar<Iterator, Problem::Instance()>
	{
		InstanceGrammar() : InstanceGrammar::base_type(start)
		{
			identifier = qi::char_("a-z") >> *qi::char_("A-Za-z0-9_");

			order = "order" >> qi::lit('(') >> identifier >> ',' >> identifier >> ',' >> identifier >> ')' >> '.';
			start = *order;
		}

		qi::rule<Iterator, std::string()> identifier;
		qi::rule<Iterator, Problem::StringTriple()> order;
		qi::rule<Iterator, Problem::Instance()> start;
	};

	inline std::string tripleToString(const Problem::StringTriple& t) {
		std::ostringstream ss;
		ss << '(' << t[0] << ',' << t[1] << ',' << t[2] << ')';
		return ss.str();
	}
}




Problem::Problem(std::istream& input)
	: input(input)
{
}

Problem::~Problem()
{
}

void Problem::declareVertex(std::ostream& out, sharp::Vertex v, const sharp::VertexSet& currentVertices) const
{
	// Declare all orderings that contain v s.t. all elements are in the current bag
	assert(elementToTriples.find(v) != elementToTriples.end());
	const VertexTripleSet& triples = elementToTriples.find(v)->second;
	foreach(const VertexTriple& t, triples) {
		if(currentVertices.find(t[0]) != currentVertices.end()
				&& currentVertices.find(t[1]) != currentVertices.end()
				&& currentVertices.find(t[2]) != currentVertices.end())
			out << "order(v" << t[0] << ",v" << t[1] << ",v" << t[2] << ")." << std::endl;
	}
}

void Problem::parse()
{
	InstanceGrammar<boost::spirit::istream_iterator> instanceParser;

	boost::spirit::istream_iterator it(input);
	boost::spirit::istream_iterator end;

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
	sharp::VertexSet vertices;
	sharp::HyperedgeSet hyperedges;

	foreach(const StringTriple& t, instance) {
		sharp::Vertex v1 = storeVertexName(t[0]);
		sharp::Vertex v2 = storeVertexName(t[1]);
		sharp::Vertex v3 = storeVertexName(t[2]);

		vertices.insert(v1);
		vertices.insert(v2);
		vertices.insert(v3);

		sharp::VertexSet hyperedge;
		hyperedge.insert(v1);
		hyperedge.insert(v2);
		hyperedge.insert(v3);
		hyperedges.insert(hyperedge);

		VertexTriple vt;
		vt.reserve(3);
		vt.push_back(v1);
		vt.push_back(v2);
		vt.push_back(v3);
		elementToTriples[v1].insert(vt);
		elementToTriples[v2].insert(vt);
		elementToTriples[v3].insert(vt);
	}

	return createHypergraphFromSets(vertices, hyperedges);
}

} // namespace cyclic_ordering
