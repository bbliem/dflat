/*
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

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

#include <vector>

class ItemTree;
class Decomposition;
class Application;

class Solver
{
public:
	// Construct a solver responsible for the root of the given decomposition
	Solver(const Decomposition& decomposition, const Application& app);

	// Return the complete item tree
	virtual ItemTree compute() = 0;

	// Return the next subtree that is a candidate for a winning strategy
	//virtual ItemTree nextCandidate() = 0;

	// Return the next candidate that has an optimization value below the given one
	//virtual ItemTree nextFeasibleCandidate(int bestSoFar) = 0;

protected:
	const Decomposition& decomposition;
	const Application& app;
};
