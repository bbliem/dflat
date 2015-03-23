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
#include "AtomRemovalSolver.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace solver { namespace asp {

AtomRemovalSolver::AtomRemovalSolver(const Decomposition& decomposition, const Application& app, const Hypergraph::Vertex& removedAtom)
	: SolverBase(decomposition, app)
	, removedAtom(removedAtom)
{
}

Solver::Result AtomRemovalSolver::compute()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	assert(decomposition.getChildren().size() == 1);
	Decomposition& childNode = **decomposition.getChildren().begin();
	Result childResult = childNode.getSolver().compute();
	Result result;

	if(childResult) {
		result = extendRoot(childResult);
		assert(childResult->getChildren().empty() == false);

		// Guess node to extend at depth 1
		for(const ItemTreeChildPtr& childCandidate : childResult->getChildren()) {
			ItemTreeNode::Items candidateItems = childCandidate->getNode()->getItems();
			candidateItems.erase(removedAtom);
			ItemTreeNode::Items candidateAuxItems = childCandidate->getNode()->getAuxItems();
			ItemTreeChildPtr candidate = extendCandidate(std::move(candidateItems), std::move(candidateAuxItems), childCandidate);

			for(const ItemTreeChildPtr& childCertificate : childCandidate->getChildren()) {
				ItemTreeNode::Items certificateItems = childCertificate->getNode()->getItems();
				certificateItems.erase(removedAtom);
				ItemTreeNode::Items certificateAuxItems = childCertificate->getNode()->getAuxItems();
				const ItemTreeNode::Type type = decomposition.isRoot() ? (certificateAuxItems.find(String("smaller")) == certificateAuxItems.end() ? ItemTreeNode::Type::ACCEPT : ItemTreeNode::Type::REJECT) : ItemTreeNode::Type::UNDEFINED;
				candidate->addChildAndMerge(extendCertificate(std::move(certificateItems), std::move(certificateAuxItems), childCertificate, type));
			}
			result->addChildAndMerge(std::move(candidate));
		}

		if(result->finalize(app, decomposition.isRoot(), app.isPruningDisabled() == false || decomposition.isRoot()) == false)
			result.reset();
	}

	app.getPrinter().solverInvocationResult(decomposition, result.get());

	return result;
}

}} // namespace solver::asp
