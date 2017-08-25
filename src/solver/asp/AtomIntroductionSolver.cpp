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
//}}}
#include "AtomIntroductionSolver.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace solver { namespace asp {

AtomIntroductionSolver::AtomIntroductionSolver(const Decomposition& decomposition, const Application& app, String introducedAtom)
	: SolverBase(decomposition, app)
	, introducedAtom(introducedAtom)
{
}

ItemTreePtr AtomIntroductionSolver::compute()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	assert(decomposition.getChildren().size() == 1);
	Decomposition& childNode = **decomposition.getChildren().begin();
	ItemTreePtr childResult = childNode.getSolver().compute();
	ItemTreePtr result;

	if(childResult) {
		result = extendRoot(childResult);
		assert(childResult->getChildren().empty() == false);

		// Find out which rules are satisfied by setting introducedAtom to true or false, respectively, and which disappear from the reduct by setting introducedAtom to true.
		// TODO unordered_set?
		Rules rulesSatisfiedByTrue;
		Rules rulesSatisfiedByFalse;
		Rules rulesDisappearingByTrue;
		for(String bagElement : decomposition.getNode().getBag()) {
			AtomsInRule::const_iterator it = heads.find(bagElement);
			if(it != heads.end() && it->second.find(introducedAtom) != it->second.end())
				rulesSatisfiedByTrue.insert(bagElement);
			it = positiveBody.find(bagElement);
			if(it != positiveBody.end() && it->second.find(introducedAtom) != it->second.end())
				rulesSatisfiedByFalse.insert(bagElement);
			it = negativeBody.find(bagElement);
			if(it != negativeBody.end() && it->second.find(introducedAtom) != it->second.end()) {
				rulesSatisfiedByTrue.insert(bagElement);
				rulesDisappearingByTrue.insert(bagElement);
			}
		}

		// Guess node to extend at depth 1
		for(const ItemTreePtr& childCandidate : childResult->getChildren()) {
			// Make introducedAtom false
			ItemTreeNode::Items candidateItems = childCandidate->getNode()->getItems();
			ItemTreeNode::Items candidateAuxItems = childCandidate->getNode()->getAuxItems();
			// Add satisfied rules
			candidateAuxItems.insert(rulesSatisfiedByFalse.begin(), rulesSatisfiedByFalse.end());
			ItemTreePtr candidate = extendCandidate(std::move(candidateItems), std::move(candidateAuxItems), childCandidate);

			for(const ItemTreePtr& childCertificate : childCandidate->getChildren()) {
				ItemTreeNode::Items certificateItems = childCertificate->getNode()->getItems();
				ItemTreeNode::Items certificateAuxItems = childCertificate->getNode()->getAuxItems();
				// Add satisfied rules
				certificateAuxItems.insert(rulesSatisfiedByFalse.begin(), rulesSatisfiedByFalse.end());
				candidate->addChildAndMerge(extendCertificate(std::move(certificateItems), std::move(certificateAuxItems), childCertificate));
			}
			result->addChildAndMerge(std::move(candidate));

			// Make introducedAtom true
			candidateItems = childCandidate->getNode()->getItems();
			candidateItems.insert(introducedAtom);
			candidateAuxItems = childCandidate->getNode()->getAuxItems();
			// Add satisfied rules
			candidateAuxItems.insert(rulesSatisfiedByTrue.begin(), rulesSatisfiedByTrue.end());
			candidate = extendCandidate(std::move(candidateItems), std::move(candidateAuxItems), childCandidate);

			for(const ItemTreePtr& childCertificate : childCandidate->getChildren()) {
				// Make introducedAtom false in certificate (and add "smaller" flag)
				ItemTreeNode::Items certificateItems = childCertificate->getNode()->getItems();
				ItemTreeNode::Items certificateAuxItems = childCertificate->getNode()->getAuxItems();
				certificateAuxItems.emplace("smaller");
				certificateAuxItems.insert(rulesDisappearingByTrue.begin(), rulesDisappearingByTrue.end());
				certificateAuxItems.insert(rulesSatisfiedByFalse.begin(), rulesSatisfiedByFalse.end());
				candidate->addChildAndMerge(extendCertificate(std::move(certificateItems), std::move(certificateAuxItems), childCertificate));

				// Make introducedAtom true in certificate
				certificateItems = childCertificate->getNode()->getItems();
				certificateItems.insert(introducedAtom);
				certificateAuxItems = childCertificate->getNode()->getAuxItems();
				certificateAuxItems.insert(rulesSatisfiedByTrue.begin(), rulesSatisfiedByTrue.end());
				candidate->addChildAndMerge(extendCertificate(std::move(certificateItems), std::move(certificateAuxItems), childCertificate));
			}

			result->addChildAndMerge(std::move(candidate));
		}

		assert(!decomposition.isRoot());
		if(result->finalize(app, false, app.isPruningDisabled() == false || decomposition.isRoot()) == false)
			result.reset();
	}

	app.getPrinter().solverInvocationResult(decomposition, result.get());

	return result;
}

}} // namespace solver::asp
