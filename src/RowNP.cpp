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

#include "RowNP.h"
#include "Problem.h"

RowNP::RowNP()
	: currentCost(0), cost(0)
{
}

bool RowNP::operator<(const sharp::Row& rhs) const
{
	return items < dynamic_cast<const RowNP&>(rhs).items;
}

bool RowNP::operator==(const sharp::Row& rhs) const
{
	return items == dynamic_cast<const RowNP&>(rhs).items;
}

void RowNP::unify(const sharp::Row& old)
{
	assert(currentCost == dynamic_cast<const RowNP&>(old).currentCost);
	cost = std::min(cost, dynamic_cast<const RowNP&>(old).cost);
}

bool RowNP::matches(const Row& other) const
{
	return items == dynamic_cast<const RowNP&>(other).items;
}

RowNP* RowNP::join(const Row& other) const
{
	// Since according to matches() the rows must coincide, we suppose equal currentCost
	assert(currentCost == dynamic_cast<const RowNP&>(other).currentCost);
	assert(cost >= currentCost);
	assert(dynamic_cast<const RowNP&>(other).cost >= dynamic_cast<const RowNP&>(other).currentCost);

	RowNP* t = new RowNP(*this);
	// currentCost is contained in both left->cost and right->cost, so subtract it once
	t->cost = (this->cost - currentCost) + dynamic_cast<const RowNP&>(other).cost;
	return t;
}

void RowNP::declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, unsigned childNumber) const
{
	std::ostringstream rowName;
	rowName << 'r' << &rowAndSolution;
	out << "childRow(" << rowName.str() << ',' <<  childNumber << ")." << std::endl;
	out << "childCost(" << rowName.str() << ',' << cost << ")." << std::endl;
	foreach(const std::string& value, items)
		out << "childItem(" << rowName.str() << ',' << value << ")." << std::endl;
}

const RowNP::Items& RowNP::getItems() const
{
	return items;
}

unsigned int RowNP::getCurrentCost() const
{
	return currentCost;
}

unsigned int RowNP::getCost() const
{
	return cost;
}

#ifdef PRINT_COMPUTED_ROWS
void RowNP::print(std::ostream& str) const
{
	str << "Row: ";
	foreach(const std::string& value, items)
		str << value << ' ';
	str << "(cost " << cost << ")" << std::endl;
}
#endif
