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

#pragma once

#include <sharp/main>

#include "../Tuple.h"
#include "OptCountingSolution.h"

namespace solution {

class OptCountingPlan : public sharp::Plan
{
public:
	static OptCountingPlan* leaf(const Tuple& tuple);
	static OptCountingPlan* extend(const OptCountingPlan* base, const Tuple& extension);
	static OptCountingPlan* unify(const OptCountingPlan* left, const OptCountingPlan* right);
	static OptCountingPlan* join(const OptCountingPlan* left, const OptCountingPlan* right, const Tuple& joined);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	OptCountingPlan(const OptCountingSolution::CountType& count, unsigned cost, unsigned currentCost);
	OptCountingSolution::CountType count;
	unsigned cost;
	unsigned currentCost;
};

} // namespace solution
