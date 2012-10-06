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

#include "Row.h"
#include "Problem.h"

Row::Row(const Row::Items& topLevelItems, unsigned int initialCount)
	: count(initialCount), currentCost(0), cost(0)
{
	tree.children[topLevelItems];
}

bool Row::Tree::operator==(const Tree& rhs) const
{
	return children == rhs.children;
}

bool Row::Tree::operator<(const Tree& rhs) const
{
	return children < rhs.children;
}

bool Row::operator<(const sharp::Row& rhs) const
{
	return tree < dynamic_cast<const Row&>(rhs).tree;
}

bool Row::operator==(const sharp::Row& rhs) const
{
	return tree == dynamic_cast<const Row&>(rhs).tree;
}

void Row::unify(sharp::Row& other)
{
	Row& o = dynamic_cast<Row&>(other);
	assert(currentCost == o.currentCost);
	assert(getItems() == o.getItems());

	if(o.cost < cost) {
		count = o.count;
		cost = o.cost;
		tree.children.swap(o.tree.children);
		extensionPointers.swap(o.extensionPointers);
	}
	else if(o.cost == cost) {
		count += o.count;
		extensionPointers.insert(extensionPointers.end(), o.extensionPointers.begin(), o.extensionPointers.end());
	}
}

bool Row::matches(const Row& other) const
{
	return tree == dynamic_cast<const Row&>(other).tree;
}

Row* Row::join(const Row& other) const
{
	// Since according to matches() the trees must coincide, we suppose equal currentCost
	assert(currentCost == dynamic_cast<const Row&>(other).currentCost);
	assert(cost >= currentCost);
	assert(dynamic_cast<const Row&>(other).cost >= dynamic_cast<const Row&>(other).currentCost);

	Row* t = new Row(*this);
	// currentCost is contained in both this->cost and other.cost, so subtract it once
	t->cost = (this->cost - currentCost) + dynamic_cast<const Row&>(other).cost;
	// TODO: Test this; I'm sure there's something missing
	return t;
}

void Row::declare(std::ostream& out, unsigned childNumber) const
{
	std::ostringstream rowName;
	rowName << 'r' << this;
	out << "childRow(" << rowName.str() << ',' << childNumber << ")." << std::endl;
	out << "childCost(" << rowName.str() << ',' << cost << ")." << std::endl;

	assert(tree.children.size() == 1);
	Tree::Children::const_iterator root = tree.children.begin();
	// Print the top-level row
	foreach(const std::string& value, root->first)
		out << "childItem(" << rowName.str() << ',' << value << ")." << std::endl;
	// Print subsidiary rows
	root->second.declare(out, rowName.str());
}

#ifdef PRINT_COMPUTED_ROWS
void Row::Tree::print(std::ostream& out, const std::string& indent) const {
	foreach(const Row::Tree::Children::value_type& child, children) {
		out << indent;
		foreach(const std::string& value, child.first)
			out << value << ' ';
		out << std::endl;
		child.second.print(out, indent + "  ");
	}
}

void Row::print(std::ostream& str) const
{
	str << "Row:" << std::endl;
	tree.print(str);
	str << "(cost " << cost << ")" << std::endl;
}
#endif

void Row::Tree::declare(std::ostream& out, const std::string& parent) const
{
	foreach(const Children::value_type& child, children) {
		// Declare this subsidiary row
		std::ostringstream thisName;
		thisName << 'a' << &child;
		out << "sub(" << parent << ',' << thisName.str() << ")." << std::endl;
		// Print the row
		foreach(const std::string& value, child.first)
			out << "childItem(" << thisName.str() << ',' << value << ")." << std::endl;
		// Print its child rows
		child.second.declare(out, thisName.str());
	}
}

void Row::addExtensionPointerTuple(const ExtensionPointerTuple& ep)
{
#ifndef DISABLE_ANSWER_SET_CHECKS
	if(!extensionPointers.empty() && extensionPointers.front().size() != ep.size())
		throw std::runtime_error("Tried to add extension pointer tuple with different arity than before");
#endif
	extensionPointers.push_back(ep);
	// Default counting
	mpz_class product = 1;
	foreach(const Row* predecessor, ep)
		product *= predecessor->getCount();
	std::cout << "Add " << product << " to " << count << " because of " << ep.size() << " EPs " << '\n';
	count += product;
}
