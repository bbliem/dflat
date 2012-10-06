#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Problem.h"

namespace qi = boost::spirit::qi;

using std::vector;
using std::string;
using std::pair;

namespace {
	typedef Problem::Identifier Identifier;
	typedef Problem::Atom Atom;
	typedef Problem::Literal Literal;
	typedef Problem::Head Head;
	typedef Problem::Body Body;
	typedef Problem::Rule Rule;
	typedef Problem::Program Program;

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
	struct LogicProgramGrammar : qi::grammar<Iterator, SkipperGrammar<Iterator>, Program()>
	{
		typedef SkipperGrammar<Iterator> Skipper;

		LogicProgramGrammar(Skipper& skipper) : LogicProgramGrammar::base_type(program)
		{
			identifier = qi::char_("a-z") >> *qi::char_("A-Za-z0-9_");

			atom =
				identifier
				>> -(
						qi::char_('(')
						>> identifier >> *(qi::char_(',') >> identifier)
						>> qi::char_(')')
					)
				;

			// Use qi::lexeme to switch off skipping one space after "not".
			// If there are any subsequent spaces, these are skipped when parsing the atom.
			literal = qi::matches[qi::lexeme["not" >> skipper]] >> atom;

			head = atom % '|';

			body = literal % ',';

			program = *(
					(head || (":-" >> body)) >> '.'
					);
		}

		qi::rule<Iterator, Identifier()> identifier; // ground term or predicate name
		qi::rule<Iterator, Skipper, Atom()> atom;
		qi::rule<Iterator, Skipper, Literal()> literal;
		qi::rule<Iterator, Skipper, Head()> head;
		qi::rule<Iterator, Skipper, Body()> body;
		qi::rule<Iterator, Skipper, Program()> program;
	};
}




Problem::Problem(std::istream& input, bool printBenchmarkInformation)
	: sharp::Problem(printBenchmarkInformation), input(input), lastRule(0), atomsInRules(0)
//	: sharp::Problem(new sharp::BucketEliminationAlgorithm(new sharp::MaximumCardinalitySearchOrdering())), input(input), lastRule(0), atomsInRules(0)
{
}

Problem::~Problem()
{
	delete[] atomsInRules;
}

void Problem::parse()
{
	SkipperGrammar<boost::spirit::istream_iterator> skipper;
	LogicProgramGrammar<boost::spirit::istream_iterator> logicProgramParser(skipper);

	input.unsetf(std::ios::skipws);
	boost::spirit::istream_iterator it(input);
	boost::spirit::istream_iterator end;

	bool result = qi::phrase_parse(
			it,
			end,
			logicProgramParser,
			skipper,
			program
			);

	if(!result || it != end)
		throw std::runtime_error("Parse error");

	/*
	// Print parsed problem for debugging purposes
	std::cout << "done:" << std::endl;

	for(Program::iterator r = program.begin(); r != program.end(); ++r) {
		for(Head::iterator it = r->first.begin(); it != r->first.end(); ++it) {
			if(it != r->first.begin())
				std::cout << " | ";
			std::cout << *it;
		}
		std::cout << " :- ";
		for(Body::iterator it = r->second.begin(); it != r->second.end(); ++it) {
			if(it != r->second.begin())
				std::cout << ", ";
			bool negative = it->first;
			string atom = it->second;
			if(negative)
				std::cout << "not ";
			std::cout << atom;
		}
		std::cout << '.' << std::endl;
	}
	*/
}

void Problem::preprocess()
{
}

sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	assert(atomsInRules == 0 && lastRule == 0);
	atomsInRules = new VerticesInRule[program.size()];
	lastRule = program.size();

	sharp::VertexSet ruleVertices;
	sharp::VertexSet atomVertices;
	sharp::EdgeSet edges;

	// XXX: Probably this is the wrong location for storing vertex names. Ideally we would do this during parsing
	for(Program::iterator r = program.begin(); r != program.end(); ++r) {
		sharp::Vertex vRule = createAuxiliaryVertex();
		assert(vRule == r - program.begin() + 1); // Suppose that the first vertex is 1
		ruleVertices.insert(vRule);
	}

	sharp::Vertex vRule = 0;
	foreach(const Rule& rule, program) {
		VerticesInRule& verticesInThisRule = atomsInRules[vRule++];

		foreach(const Atom& a, rule.first) {
			sharp::Vertex v = storeVertexName(a);
			atomVertices.insert(v);
			edges.insert(std::make_pair(vRule, v));

			verticesInThisRule.head.insert(v);
		}
		foreach(const Literal& l, rule.second) {
			sharp::Vertex v = storeVertexName(l.second);
			atomVertices.insert(v);
			edges.insert(std::make_pair(vRule, v));

			if(l.first == true)
				verticesInThisRule.neg.insert(v);
			else
				verticesInThisRule.pos.insert(v);
		}
	}

	return createGraphFromDisjointSets(ruleVertices, atomVertices, edges);
}
