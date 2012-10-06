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

namespace solution {

class OptEnumPlan : public sharp::Plan
{
public:
	static OptEnumPlan* leaf(const Tuple& tuple);
	static OptEnumPlan* extend(const OptEnumPlan* base, const Tuple& extension);
	static OptEnumPlan* unify(const OptEnumPlan* left, const OptEnumPlan* right);
	static OptEnumPlan* join(const OptEnumPlan* left, const OptEnumPlan* right, const Tuple& joined);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	OptEnumPlan(Operation operation, const Tuple::Assignment& assignment, unsigned cost, unsigned currentCost, const OptEnumPlan* left = 0, const OptEnumPlan* right = 0);
	OptEnumPlan(unsigned cost, unsigned currentCost, const OptEnumPlan* left, const OptEnumPlan* right); // Union

	Tuple::Assignment assignment;
	unsigned cost;
	unsigned currentCost;
	const OptEnumPlan* left;
	const OptEnumPlan* right;
};


} // namespace solution
