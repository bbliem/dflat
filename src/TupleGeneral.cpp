/*
Copyright 2012, Bernhard Bliem
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

#include <sstream>
#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "TupleGeneral.h"
#include "Problem.h"

namespace {
	void declareTree(const TupleGeneral::Tree& node, std::ostream& out, const std::string& parent)
	{
		foreach(const TupleGeneral::Tree::Children::value_type& child, node.children) {
			// Declare this subsidiary row
			std::ostringstream thisName;
			thisName << 'a' << &child;
			out << "sub(" << parent << ',' << thisName.str() << ")." << std::endl;
			// Print the row
			foreach(const TupleGeneral::Row::value_type& mapping, child.first)
				out << "mapped(" << thisName.str() << ',' << mapping.first << ',' << mapping.second << ")." << std::endl;
			// Print its child rows
			declareTree(child.second, out, thisName.str());
		}
	}
}

TupleGeneral::TupleGeneral()
	: currentCost(0), cost(0)
{
}

bool TupleGeneral::Tree::operator==(const Tree& rhs) const
{
	return children == rhs.children;
}

bool TupleGeneral::Tree::operator<(const Tree& rhs) const
{
	return children < rhs.children;
}

bool TupleGeneral::operator<(const sharp::Tuple& rhs) const
{
	return tree < dynamic_cast<const TupleGeneral&>(rhs).tree;
}

bool TupleGeneral::operator==(const sharp::Tuple& rhs) const
{
	return tree == dynamic_cast<const TupleGeneral&>(rhs).tree;
}

void TupleGeneral::unify(const sharp::Tuple& old)
{
	assert(currentCost == dynamic_cast<const TupleGeneral&>(old).currentCost);
	cost = std::min(cost, dynamic_cast<const TupleGeneral&>(old).cost);
}

bool TupleGeneral::matches(const Tuple& other) const
{
	return tree == dynamic_cast<const TupleGeneral&>(other).tree;
}

TupleGeneral* TupleGeneral::join(const Tuple& other) const
{
	// Since according to matches() the trees must coincide, we suppose equal currentCost
	assert(currentCost == dynamic_cast<const TupleGeneral&>(other).currentCost);
	assert(cost >= currentCost);
	assert(dynamic_cast<const TupleGeneral&>(other).cost >= dynamic_cast<const TupleGeneral&>(other).currentCost);

	TupleGeneral* t = new TupleGeneral(*this);
	// currentCost is contained in both this->cost and other.cost, so subtract it once
	t->cost = (this->cost - currentCost) + dynamic_cast<const TupleGeneral&>(other).cost;
	return t;
}

void TupleGeneral::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, unsigned childNumber) const
{
	std::ostringstream tupleName;
	tupleName << 't' << &tupleAndSolution;
	out << "childTuple(" << tupleName.str() << ',' << childNumber << ")." << std::endl;
	declareTupleExceptName(out, tupleName.str());
}

void TupleGeneral::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateName) const
{
	std::ostringstream tupleName;
	tupleName << 't' << &tupleAndSolution;
	out << predicateName << '(' << tupleName.str() << ")." << std::endl;
	declareTupleExceptName(out, tupleName.str());
}

const Tuple::Row& TupleGeneral::getRow() const
{
	assert(tree.children.size() == 1);
	return tree.children.begin()->first;
}

unsigned int TupleGeneral::getCurrentCost() const
{
	return currentCost;
}

unsigned int TupleGeneral::getCost() const
{
	return cost;
}

#ifdef PRINT_COMPUTED_TUPLES
namespace {
	void printTree(std::ostream& out, const TupleGeneral::Tree& tree, const std::string& indent = "") {
		foreach(const TupleGeneral::Tree::Children::value_type& child, tree.children) {
			out << indent;
			foreach(const Tuple::Row::value_type& pair, child.first)
				out << pair.first << '=' << pair.second << ' ';
			out << std::endl;
			printTree(out, child.second, indent + "  ");
		}
	}
}

void TupleGeneral::print(std::ostream& str) const
{
	str << "Tuple:" << std::endl;
	printTree(str, tree);
	str << "(cost " << cost << ")" << std::endl;
}
#endif

inline void TupleGeneral::declareTupleExceptName(std::ostream& out, const std::string& tupleName) const
{
	out << "childCost(" << tupleName << ',' << cost << ")." << std::endl;

	assert(tree.children.size() == 1);
	Tree::Children::const_iterator root = tree.children.begin();
	// Print the top-level row
	foreach(const Row::value_type& mapping, root->first)
		out << "mapped(" << tupleName << ',' << mapping.first << ',' << mapping.second << ")." << std::endl;
	// Print subsidiary rows
	declareTree(root->second, out, tupleName);
}
