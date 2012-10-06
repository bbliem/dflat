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

#include "RowGeneral.h"
#include "Problem.h"

namespace {
	void declareTree(const RowGeneral::Tree& node, std::ostream& out, const std::string& parent)
	{
		foreach(const RowGeneral::Tree::Children::value_type& child, node.children) {
			// Declare this subsidiary row
			std::ostringstream thisName;
			thisName << 'a' << &child;
			out << "sub(" << parent << ',' << thisName.str() << ")." << std::endl;
			// Print the row
			foreach(const std::string& value, child.first)
				out << "childItem(" << thisName.str() << ',' << value << ")." << std::endl;
			// Print its child rows
			declareTree(child.second, out, thisName.str());
		}
	}
}

RowGeneral::RowGeneral()
	: currentCost(0), cost(0)
{
}

bool RowGeneral::Tree::operator==(const Tree& rhs) const
{
	return children == rhs.children;
}

bool RowGeneral::Tree::operator<(const Tree& rhs) const
{
	return children < rhs.children;
}

bool RowGeneral::operator<(const sharp::Row& rhs) const
{
	return tree < dynamic_cast<const RowGeneral&>(rhs).tree;
}

bool RowGeneral::operator==(const sharp::Row& rhs) const
{
	return tree == dynamic_cast<const RowGeneral&>(rhs).tree;
}

void RowGeneral::unify(const sharp::Row& old)
{
	assert(currentCost == dynamic_cast<const RowGeneral&>(old).currentCost);
	cost = std::min(cost, dynamic_cast<const RowGeneral&>(old).cost);
}

bool RowGeneral::matches(const Row& other) const
{
	return tree == dynamic_cast<const RowGeneral&>(other).tree;
}

RowGeneral* RowGeneral::join(const Row& other) const
{
	// Since according to matches() the trees must coincide, we suppose equal currentCost
	assert(currentCost == dynamic_cast<const RowGeneral&>(other).currentCost);
	assert(cost >= currentCost);
	assert(dynamic_cast<const RowGeneral&>(other).cost >= dynamic_cast<const RowGeneral&>(other).currentCost);

	RowGeneral* t = new RowGeneral(*this);
	// currentCost is contained in both this->cost and other.cost, so subtract it once
	t->cost = (this->cost - currentCost) + dynamic_cast<const RowGeneral&>(other).cost;
	return t;
}

void RowGeneral::declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, unsigned childNumber) const
{
	std::ostringstream rowName;
	rowName << 'r' << &rowAndSolution;
	out << "childRow(" << rowName.str() << ',' << childNumber << ")." << std::endl;
	declareRowExceptName(out, rowName.str());
}

void RowGeneral::declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, const char* predicateName) const
{
	std::ostringstream rowName;
	rowName << 'r' << &rowAndSolution;
	out << predicateName << '(' << rowName.str() << ")." << std::endl;
	declareRowExceptName(out, rowName.str());
}

const Row::Items& RowGeneral::getItems() const
{
	assert(tree.children.size() == 1);
	return tree.children.begin()->first;
}

unsigned int RowGeneral::getCurrentCost() const
{
	return currentCost;
}

unsigned int RowGeneral::getCost() const
{
	return cost;
}

#ifdef PRINT_COMPUTED_ROWS
namespace {
	void printTree(std::ostream& out, const RowGeneral::Tree& tree, const std::string& indent = "") {
		foreach(const RowGeneral::Tree::Children::value_type& child, tree.children) {
			out << indent;
			foreach(const std::string& value, child.first)
				out << value << ' ';
			out << std::endl;
			printTree(out, child.second, indent + "  ");
		}
	}
}

void RowGeneral::print(std::ostream& str) const
{
	str << "Row:" << std::endl;
	printTree(str, tree);
	str << "(cost " << cost << ")" << std::endl;
}
#endif

inline void RowGeneral::declareRowExceptName(std::ostream& out, const std::string& rowName) const
{
	out << "childCost(" << rowName << ',' << cost << ")." << std::endl;

	assert(tree.children.size() == 1);
	Tree::Children::const_iterator root = tree.children.begin();
	// Print the top-level row
	foreach(const std::string& value, root->first)
		out << "childItem(" << rowName << ',' << value << ")." << std::endl;
	// Print subsidiary rows
	declareTree(root->second, out, rowName);
}
