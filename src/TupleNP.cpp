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

#include "TupleNP.h"
#include "Problem.h"

TupleNP::TupleNP()
	: currentCost(0), cost(0)
{
}

bool TupleNP::operator<(const sharp::Tuple& rhs) const
{
	return row < dynamic_cast<const TupleNP&>(rhs).row;
}

bool TupleNP::operator==(const sharp::Tuple& rhs) const
{
	return row == dynamic_cast<const TupleNP&>(rhs).row;
}

void TupleNP::unify(const sharp::Tuple& old)
{
	assert(currentCost == dynamic_cast<const TupleNP&>(old).currentCost);
	cost = std::min(cost, dynamic_cast<const TupleNP&>(old).cost);
}

bool TupleNP::matches(const Tuple& other) const
{
	return row == dynamic_cast<const TupleNP&>(other).row;
}

TupleNP* TupleNP::join(const Tuple& other) const
{
	// Since according to matches() the rows must coincide, we suppose equal currentCost
	assert(currentCost == dynamic_cast<const TupleNP&>(other).currentCost);
	assert(cost >= currentCost);
	assert(dynamic_cast<const TupleNP&>(other).cost >= dynamic_cast<const TupleNP&>(other).currentCost);

	TupleNP* t = new TupleNP(*this);
	// currentCost is contained in both left->cost and right->cost, so subtract it once
	t->cost = (this->cost - currentCost) + dynamic_cast<const TupleNP&>(other).cost;
	return t;
}

void TupleNP::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, unsigned childNumber) const
{
	std::ostringstream tupleName;
	tupleName << 't' << &tupleAndSolution;
	out << "childTuple(" << tupleName.str() << ',' <<  childNumber << ")." << std::endl;
	declareTupleExceptName(out, tupleName.str());
}

void TupleNP::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateName) const
{
	std::ostringstream tupleName;
	tupleName << 't' << &tupleAndSolution;
	out << predicateName << '(' << tupleName.str() << ")." << std::endl;
	declareTupleExceptName(out, tupleName.str());
}

const TupleNP::Row& TupleNP::getRow() const
{
	return row;
}

unsigned int TupleNP::getCurrentCost() const
{
	return currentCost;
}

unsigned int TupleNP::getCost() const
{
	return cost;
}

#ifdef PRINT_COMPUTED_TUPLES
void TupleNP::print(std::ostream& str) const
{
	str << "Tuple: ";
	foreach(const Row::value_type& a, row)
		str << a.first << '=' << a.second << ' ';
	str << "(cost " << cost << ")" << std::endl;
}
#endif

inline void TupleNP::declareTupleExceptName(std::ostream& out, const std::string& tupleName) const
{
	out << "childCost(" << tupleName << ',' << cost << ")." << std::endl;
	foreach(const Row::value_type& a, row)
		out << "mapped(" << tupleName << ',' << a.first << ',' << a.second << ")." << std::endl;
}
