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

#include "Module.h"
#include "Solver.h"

class ItemTree;
class Decomposition;

class SolverFactory : public Module
{
public:
	typedef Solver::ChildSolvers ChildSolvers;

	SolverFactory(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);

	// Construct a solver responsible for the root of the given decomposition
	virtual Solver* newSolver(const Decomposition& decomposition, const ChildSolvers& childSolvers) const = 0;

	virtual void select() override;
};
