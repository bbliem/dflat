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

#include "CountingPlan.h"

namespace solution {

CountingPlan::CountingPlan(const CountingSolution::CountType& count)
	: Plan(LEAF), count(count)
{
}

CountingPlan* CountingPlan::leaf(const sharp::Tuple&)
{
	return new CountingPlan(1);
}

CountingPlan* CountingPlan::extend(const CountingPlan* base, const sharp::Tuple&)
{
	return new CountingPlan(base->count);
}

CountingPlan* CountingPlan::unify(const CountingPlan* left, const CountingPlan* right)
{
	return new CountingPlan(left->count + right->count);
}

CountingPlan* CountingPlan::join(const CountingPlan* left, const CountingPlan* right, const sharp::Tuple&)
{
	return new CountingPlan(left->count * right->count);
}

sharp::Solution* CountingPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return new CountingSolution(count);
}

sharp::Solution* CountingPlan::materializeExtension() const
{
	assert(false);
	return 0;
}

sharp::Solution* CountingPlan::materializeUnion() const
{
	assert(false);
	return 0;
}

sharp::Solution* CountingPlan::materializeJoin() const
{
	assert(false);
	return 0;
}

} // namespace solution
