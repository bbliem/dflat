/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include <chrono>
#include <stack>

#include "../Printer.h"

namespace printer {

class Performance : public Printer
{
public:
	Performance(Application& app, bool newDefault = false);

	virtual void solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result) override;
	virtual bool listensForSolverEvents() const override;
	virtual void result(const ItemTreePtr& rootItemTree) override;

protected:
	virtual void enterNode(const Decomposition& decompositionNode) override;
	virtual void leaveNode() override;

	void printGraphMlElements(std::ostream& out, const Decomposition* node) const;

	struct NodeData
	{
		// Time at last call to enterNode()
		std::chrono::high_resolution_clock::time_point startTime;

		// Total elapsed time in the respective node (sum of durations between enterNode() and leaveNode())
		std::chrono::milliseconds elapsedTime;

		// Estimated size of the respective node's item tree after calling printer::Performance::solverInvocationResult()
		// XXX If size_t has, e.g., 32 bits, we can only measure up to 4 GiB
		std::size_t itemTreeSize = 0;
	};

	std::unordered_map<const Decomposition*, NodeData> nodeData;
	std::stack<const Decomposition*> computationStack;
};

} // namespace printer
