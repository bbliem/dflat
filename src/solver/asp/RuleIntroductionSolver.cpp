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
#include "RuleIntroductionSolver.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace {
	// http://stackoverflow.com/questions/1964150/c-test-if-2-sets-are-disjoint
	template<class Set1, class Set2>
	inline bool disjoint(const Set1 &set1, const Set2 &set2)
	{
		if(set1.empty() || set2.empty())
			return true;

		typename Set1::const_iterator it1 = set1.begin(), it1End = set1.end();
		typename Set2::const_iterator it2 = set2.begin(), it2End = set2.end();

		if(*it1 > *set2.rbegin() || *it2 > *set1.rbegin())
			return true;

		while(it1 != it1End && it2 != it2End) {
			if(*it1 == *it2)
				return false;
			if(*it1 < *it2)
				it1++;
			else
				it2++;
		}

		return true;
	}
}

namespace solver { namespace asp {

RuleIntroductionSolver::RuleIntroductionSolver(const Decomposition& decomposition, const Application& app, String introducedRule)
	: SolverBase(decomposition, app)
	, introducedRule(introducedRule)
{
}

ItemTreePtr RuleIntroductionSolver::compute()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	assert(decomposition.getChildren().size() == 1);
	Decomposition& childNode = **decomposition.getChildren().begin();
	ItemTreePtr childResult = childNode.getSolver().compute();
	ItemTreePtr result;

	if(childResult) {
		result = extendRoot(childResult);
		assert(childResult->getChildren().empty() == false);

		for(const ItemTreePtr& childCandidate : childResult->getChildren()) {
			// Make introducedAtom false
			ItemTreeNode::Items candidateItems = childCandidate->getNode()->getItems();
			ItemTreeNode::Items candidateAuxItems = childCandidate->getNode()->getAuxItems();

			// Calculate false atoms of this candidate
			Atoms candidateFalseAtoms;
			std::set_symmetric_difference(decomposition.getNode().getBag().begin(), decomposition.getNode().getBag().end(), candidateItems.begin(), candidateItems.end(), std::inserter(candidateFalseAtoms, candidateFalseAtoms.begin()));

			const bool introducedRuleDisappears = doesIntroducedRuleDisappear(candidateItems);
			if(introducedRuleDisappears || isIntroducedRuleSatisfied(candidateItems, candidateFalseAtoms))
				candidateAuxItems.insert(introducedRule);
			ItemTreePtr candidate = extendCandidate(std::move(candidateItems), std::move(candidateAuxItems), childCandidate);

			for(const ItemTreePtr& childCertificate : childCandidate->getChildren()) {
				ItemTreeNode::Items certificateItems = childCertificate->getNode()->getItems();
				ItemTreeNode::Items certificateAuxItems = childCertificate->getNode()->getAuxItems();
				if(introducedRuleDisappears)
					certificateAuxItems.insert(introducedRule);
				else {
					// Calculate false atoms of this certificate
					Atoms certificateFalseAtoms;
					std::set_symmetric_difference(decomposition.getNode().getBag().begin(), decomposition.getNode().getBag().end(), certificateItems.begin(), certificateItems.end(), std::inserter(certificateFalseAtoms, certificateFalseAtoms.begin()));
					if(isIntroducedRuleSatisfied(certificateItems, certificateFalseAtoms))
						certificateAuxItems.insert(introducedRule);
				}
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

inline bool RuleIntroductionSolver::isIntroducedRuleSatisfied(const Atoms& trueAtoms, const Atoms& falseAtoms) const
{
	assert(heads.find(introducedRule) != heads.end());
	assert(positiveBody.find(introducedRule) != positiveBody.end());
	assert(negativeBody.find(introducedRule) != negativeBody.end());
	return !disjoint(heads.at(introducedRule), trueAtoms) || !disjoint(positiveBody.at(introducedRule), falseAtoms) || !disjoint(negativeBody.at(introducedRule), trueAtoms);
}

inline bool RuleIntroductionSolver::doesIntroducedRuleDisappear(const Atoms& trueAtoms) const
{
	assert(negativeBody.find(introducedRule) != negativeBody.end());
	return !disjoint(negativeBody.at(introducedRule), trueAtoms);
}

}} // namespace solver::asp
