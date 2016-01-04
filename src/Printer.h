/*{{{
Copyright 2012-2016, Bernhard Bliem
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
#include "UncompressedItemTree.h"

class Decomposition;
class DecompositionNode;

class Printer : public Module
{
public:
	Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);
	virtual ~Printer() = 0;

	virtual void decomposerResult(const Decomposition& result);

	// A printer can manage a decomposition node stack that contains the
	// visited nodes. When a decomposition node is visited, a NodeStackElement
	// should be instantiated via visitNode(). This pushes the current node on
	// the printer's stack. When the NodeStackElement goes out of scope, the
	// node is popped.
	// For instance, put the following at the beginning of YourSolver::compute():
	// const auto nodeStackElement = app.getPrinter().visitNode(decomposition);
	class NodeStackElement
	{
	public:
		~NodeStackElement();
	private:
		friend class Printer;
		NodeStackElement(Printer& printer, const Decomposition& decompositionNode);
		Printer& printer;
	};
	NodeStackElement visitNode(const Decomposition& decompositionNode);

	virtual void solverInvocationInput(const Decomposition& decompositionNode, const std::string& input);

	// This may be called only once per decomposition node (typically when it
	// has been processed completely)
	virtual void solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result);

	virtual void uncompressedSolverInvocationResult(const Decomposition& decompositionNode, const UncompressedItemTree* result);

	// Whether calls to solverEvent() have any effect.
	virtual bool listensForSolverEvents() const; // this implementation returns false
	virtual void solverEvent(const std::string& msg);

	virtual void result(const ItemTreePtr& rootItemTree);

	virtual void select() override;

	// Report possibly suboptimal solutions (if run as an anytime algorithm)
	void provisionalSolution(const ItemTreeNode& solution);

protected:
	// Called by NodeStackElement
	virtual void enterNode(const Decomposition& decompositionNode);
	virtual void leaveNode();
};
