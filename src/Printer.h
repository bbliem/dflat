/*{{{
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
//}}}
#include "Module.h"
#include "ItemTree.h" // XXX Change Solver::compute() to return an ItemTree instead of an ItemTreePtr and we can dispense with this (forward declare)

class Decomposition;
class DecompositionNode;

class Printer : public Module
{
public:
	Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);
	virtual ~Printer() = 0;

	virtual void decomposerResult(const Decomposition& result);

	// Be default, the implementations of these methods don't do anything
	virtual void solverInvocationInput(const DecompositionNode& decompositionNode, const std::string& input);
	virtual void solverInvocationResult(const DecompositionNode& decompositionNode, const ItemTree* result);

	// Whether calls to solverEvent() have any effect.
	virtual bool listensForSolverEvents() const; // this implementation returns false
	virtual void solverEvent(const std::string& msg);

	virtual void result(const ItemTreePtr& rootItemTree);

	virtual void select() override;
};
