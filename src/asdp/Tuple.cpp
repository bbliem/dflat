#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Tuple.h"
#include "Problem.h"

namespace {
	void declareTree(const asdp::Tuple::Tree& node, std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, unsigned int level)
	{
		foreach(const asdp::Tuple::Tree::Children::value_type& child, node.children) {
			foreach(const asdp::Tuple::Assignment::value_type& assignment, child.first)
				out << "mapped(t" << &tupleAndSolution << ',' << level << ',' << assignment.first << ',' << assignment.second << ")." << std::endl;
			declareTree(child.second, out, tupleAndSolution, level+1);
		}
	}
}

namespace asdp {

Tuple::Tuple()
	: currentCost(0), introducedCost(0)
{
}

bool Tuple::Tree::operator==(const Tree& rhs) const
{
	return children == rhs.children;
}

bool Tuple::Tree::operator<(const Tree& rhs) const
{
	return children < rhs.children;
}

bool Tuple::operator<(const sharp::Tuple& rhs) const
{
	return tree < dynamic_cast<const Tuple&>(rhs).tree;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return tree == dynamic_cast<const Tuple&>(rhs).tree;
}

bool Tuple::matches(const ::Tuple& other) const
{
	return tree == dynamic_cast<const Tuple&>(other).tree;
}

Tuple* Tuple::join(const ::Tuple& other) const
{
	return new Tuple(*this);
}

void Tuple::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix) const
{
	out << "childTuple" << predicateSuffix << "(t" << &tupleAndSolution << ")." << std::endl;
	declareTree(tree, out, tupleAndSolution, 0);
}

const Tuple::Assignment& Tuple::getAssignment() const
{
	assert(tree.children.size() == 1);
	return tree.children.begin()->first;
}

unsigned int Tuple::getCurrentCost() const
{
	return currentCost;
}

unsigned int Tuple::getIntroducedCost() const
{
	return introducedCost;
}

#ifdef VERBOSE
namespace {
	void printTree(std::ostream& out, const Tuple::Tree& tree, const std::string& indent = "") {
		foreach(const Tuple::Tree::Children::value_type& child, tree.children) {
			out << indent;
			foreach(const Tuple::Assignment::value_type& pair, child.first)
				out << pair.first << '=' << pair.second << ' ';
			out << std::endl;
			printTree(out, child.second, indent + "  ");
		}
	}
}

void Tuple::print(std::ostream& str) const
{
	str << "Tuple:" << std::endl;
	printTree(str, tree);
}
#endif

} // namespace asdp
