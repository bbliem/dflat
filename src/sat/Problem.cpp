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




Problem::Problem(std::istream& input)
	: input(input), lastClause(0), atomsInClauses(0)
{
}

Problem::~Problem()
{
	delete[] atomsInClauses;
}

void Problem::declareVertex(std::ostream& out, Vertex v) const
{
	if(vertexIsClause(v)) {
		out << "clause(v" << v << ")." << std::endl;

		const Problem::VerticesInClause& atoms = getAtomsInClause(v);
		foreach(Vertex v2, atoms.pos)
			out << "pos(v" << v << ",v" << v2 << ")." << std::endl;
		foreach(Vertex v2, atoms.neg)
			out << "neg(v" << v << ",v" << v2 << ")." << std::endl;
	}
	else // an atom
		out << "atom(v" << v << ")." << std::endl;
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

	foreach(const Instance::value_type& kv, instance) {
		std::cout << kv.first << ": ";
		foreach(const std::string& a, kv.second.first)
			std::cout << a << " ";
		foreach(const std::string& a, kv.second.second)
			std::cout << '-' << a << " ";
		std::cout << std::endl;
	}
}

void Problem::preprocess()
{
}

sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	assert(atomsInClauses == 0 && lastClause == 0);
	atomsInClauses = new VerticesInClause[instance.size()];
	lastClause = instance.size();

	sharp::VertexSet clauseVertices;
	sharp::VertexSet atomVertices;
	sharp::EdgeSet edges;

	// XXX: Probably this is the wrong location for storing vertex names. Ideally we would do this during parsing
	for(Instance::const_iterator it = instance.begin(); it != instance.end(); ++it)
		clauseVertices.insert(createAuxiliaryVertex());

	sharp::Vertex vClause = 0;
	foreach(const Instance::value_type& clause, instance) {
		VerticesInClause& verticesInThisClause = atomsInClauses[vClause++];

		// Positive atoms
		foreach(const std::string& atom, clause.second.first) {
			sharp::Vertex v = storeVertexName(atom);
			atomVertices.insert(v);
			edges.insert(std::make_pair(vClause, v));
			verticesInThisClause.pos.insert(v);
		}
		// Negative atoms
		foreach(const std::string& atom, clause.second.second) {
			sharp::Vertex v = storeVertexName(atom);
			atomVertices.insert(v);
			edges.insert(std::make_pair(vClause, v));
			verticesInThisClause.neg.insert(v);
		}
	}

	return createGraphFromDisjointSets(clauseVertices, atomVertices, edges);
}

} // namespace sat
