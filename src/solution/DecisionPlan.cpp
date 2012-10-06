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

#include "DecisionPlan.h"
#include "DecisionSolution.h"


namespace solution {

DecisionPlan::DecisionPlan()
	: sharp::Plan(LEAF)
{
}

DecisionPlan* DecisionPlan::leaf(const sharp::Tuple&)
{
	return new DecisionPlan;
}

DecisionPlan* DecisionPlan::extend(const DecisionPlan*, const sharp::Tuple&)
{
	return new DecisionPlan;
}

DecisionPlan* DecisionPlan::unify(const DecisionPlan*, const DecisionPlan*)
{
	return new DecisionPlan;
}

DecisionPlan* DecisionPlan::join(const DecisionPlan*, const DecisionPlan*, const sharp::Tuple&)
{
	return new DecisionPlan;
}

sharp::Solution* DecisionPlan::materializeLeaf() const
{
	return new DecisionSolution;
}

sharp::Solution* DecisionPlan::materializeExtension() const
{
	assert(false);
	return 0;
}

sharp::Solution* DecisionPlan::materializeUnion() const
{
	assert(false);
	return 0;
}

sharp::Solution* DecisionPlan::materializeJoin() const
{
	assert(false);
	return 0;
}

} // namespace solution
