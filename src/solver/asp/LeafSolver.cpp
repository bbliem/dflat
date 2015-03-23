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
//}}}
#include "LeafSolver.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace solver { namespace asp {

LeafSolver::LeafSolver(const Decomposition& decomposition, const Application& app)
	: ::Solver(decomposition, app)
{
	if(decomposition.getNode().getBag().empty() == false)
		throw std::runtime_error("ASP solver requires empty leaves");
}

Solver::Result LeafSolver::compute()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);
	assert(decomposition.getChildren().empty());
	assert(decomposition.getNode().getBag().empty());

	Result result(new ItemTree(ItemTree::Node(new ItemTreeNode({}, {}, {{}}, ItemTreeNode::Type::OR))));
	ItemTreeChildPtr candidate(new ItemTree(ItemTree::Node(new ItemTreeNode({}, {}, {{}}, ItemTreeNode::Type::AND))));
	ItemTreeChildPtr certificate(new ItemTree(ItemTree::Node(new ItemTreeNode)));
	candidate->addChildAndMerge(std::move(certificate));
	result->addChildAndMerge(std::move(candidate));

	if(result->finalize(app, decomposition.isRoot(), app.isPruningDisabled() == false || decomposition.isRoot()) == false)
		result.reset();

	app.getPrinter().solverInvocationResult(decomposition, result.get());

	return result;
}

}} // namespace solver::asp
