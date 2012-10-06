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

#include "CountingSolution.h"

namespace solution {

class CountingPlan : public sharp::Plan
{
public:
	static CountingPlan* leaf(const sharp::Tuple&);
	static CountingPlan* extend(const CountingPlan* base, const sharp::Tuple&);
	static CountingPlan* unify(const CountingPlan* left, const CountingPlan* right);
	static CountingPlan* join(const CountingPlan* left, const CountingPlan* right, const sharp::Tuple&);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	CountingPlan(const CountingSolution::CountType& count);
	CountingSolution::CountType count;
};

} // namespace solution
