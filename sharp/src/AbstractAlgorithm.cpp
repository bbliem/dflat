#include <config.h>

#include <AbstractAlgorithm.hpp>

#include <iostream>
using namespace std;

#include <ExtendedHypertree.hpp>

namespace sharp
{

	/*********************************\
	|* CLASS: Tuple
	\*********************************/
	Tuple::~Tuple() { }

	void Tuple::unify(const Tuple&) { }

	/*********************************\
	|* CLASS: Solution
	\*********************************/
	Solution::~Solution()
	{
	}

	/*********************************\
	|* CLASS: Plan
	\*********************************/
	Plan::Plan(Operation operation)
		: operation(operation)
	{
	}

	Plan::~Plan()
	{
	}

	Solution* Plan::materialize() const
	{
		switch(operation) {
			case LEAF:
				return materializeLeaf();
			case UNION:
				return materializeUnion();
			case JOIN:
				return materializeJoin();
		}
		PrintError("Invalid operation", "");
		return 0;
	}

	/*********************************\
	|* CLASS: PlanFactory
	\*********************************/
	PlanFactory::~PlanFactory()
	{
	}

	/***********************************\
	|* CLASS: AbstractHTDAlgorithm
	\***********************************/
	AbstractHTDAlgorithm::AbstractHTDAlgorithm(Problem *problem, const PlanFactory& planFactory)
		: prob(problem), planFactory(planFactory)
	{
	}
	
	AbstractHTDAlgorithm::~AbstractHTDAlgorithm() 
	{ 
	}
	
	Plan *AbstractHTDAlgorithm::evaluate(ExtendedHypertree *origroot)
	{
		ExtendedHypertree *root = this->prepareHypertreeDecomposition(origroot);
		return selectPlan(evaluateNode(root), root);
	}

	void AbstractHTDAlgorithm::addRowToTupleTable(TupleTable& table, Tuple* t, Plan* p) const
	{
		std::pair<TupleTable::iterator, bool> result = table.insert(TupleTable::value_type(t, p));
		if(!result.second) {
			Tuple* origTuple = result.first->first;
			Plan* origPlan = result.first->second;
			table.erase(result.first);
			t->unify(*origTuple);
			table.insert(TupleTable::value_type(t, planFactory.unify(origPlan, p)));
		}
	}
	

	Problem *AbstractHTDAlgorithm::problem()
	{
		return this->prob;
	}

	ExtendedHypertree *AbstractHTDAlgorithm::prepareHypertreeDecomposition(ExtendedHypertree *root)
	{
		return root;
	}


	/*********************************************\
	|* CLASS: AbstractSemiNormalizedHTDAlgorithm
	\*********************************************/
	AbstractSemiNormalizedHTDAlgorithm::AbstractSemiNormalizedHTDAlgorithm(Problem *problem, const PlanFactory& planFactory)
		: AbstractHTDAlgorithm(problem, planFactory)
	{ }

	AbstractSemiNormalizedHTDAlgorithm::~AbstractSemiNormalizedHTDAlgorithm() { }

	ExtendedHypertree *AbstractSemiNormalizedHTDAlgorithm::prepareHypertreeDecomposition(ExtendedHypertree *root)
	{
		return root->normalize(SemiNormalization);
	}
	
	TupleTable *AbstractSemiNormalizedHTDAlgorithm::evaluateNode(const ExtendedHypertree *node)
	{
		switch(node->getType())
	        {
	        case Branch: 
			return this->evaluateBranchNode(node);
		case Permutation:
		case Introduction:
	        case Removal: 
		case Leaf:
			return this->evaluatePermutationNode(node);
		default:
			PrintError("invalid node type, check normalization", "");
			return NULL;
	        }
	}

	/*********************************************\
	|* CLASS: AbstractNormalizedHTDAlgorithm
	\*********************************************/
	AbstractNormalizedHTDAlgorithm::AbstractNormalizedHTDAlgorithm(Problem *problem, const PlanFactory& planFactory)
		: AbstractSemiNormalizedHTDAlgorithm(problem, planFactory)
	{ }

	AbstractNormalizedHTDAlgorithm::~AbstractNormalizedHTDAlgorithm() { }

	ExtendedHypertree *AbstractNormalizedHTDAlgorithm::prepareHypertreeDecomposition(ExtendedHypertree *root)
	{
		return root->normalize(DefaultNormalization);
	}

	TupleTable *AbstractNormalizedHTDAlgorithm::evaluatePermutationNode(const ExtendedHypertree *node)
	{
		switch(node->getType())
		{
		case Introduction:
			return this->evaluateIntroductionNode(node);
		case Removal:
			return this->evaluateRemovalNode(node);
		case Leaf:
			return this->evaluateLeafNode(node);
		default:
			PrintError("invalid node type, check normalization", "");
			return NULL;
		}
	}
} // namespace sharp
