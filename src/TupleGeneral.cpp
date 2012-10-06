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

#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "TupleGeneral.h"
#include "Problem.h"

namespace {
	void declareTree(const TupleGeneral::Tree& node, std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, unsigned int level)
	{
		foreach(const TupleGeneral::Tree::Children::value_type& child, node.children) {
			foreach(const TupleGeneral::Assignment::value_type& assignment, child.first)
				out << "mapped(t" << &tupleAndSolution << ',' << level << ',' << assignment.first << ',' << assignment.second << ")." << std::endl;
			declareTree(child.second, out, tupleAndSolution, level+1);
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

void TupleGeneral::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix) const
{
	out << "childTuple" << predicateSuffix << "(t" << &tupleAndSolution << ")." << std::endl;
	declareTree(tree, out, tupleAndSolution, 0);
	out << "childCost(t" << &tupleAndSolution << ',' << cost << ")." << std::endl;
}

const Tuple::Assignment& TupleGeneral::getAssignment() const
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

#ifdef VERBOSE
namespace {
	void printTree(std::ostream& out, const TupleGeneral::Tree& tree, const std::string& indent = "") {
		foreach(const TupleGeneral::Tree::Children::value_type& child, tree.children) {
			out << indent;
			foreach(const Tuple::Assignment::value_type& pair, child.first)
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
