#include <config.h>

#include <AbstractAlgorithm.hpp>

#include <iostream>
using namespace std;

#include <ExtendedHypertree.hpp>

namespace sharp
{

	/*********************************\
	|* CLASS: Row
	\*********************************/
	Row::~Row() { }

	void Row::unify(Row&) { }

	/*********************************\
	|* CLASS: Solution
	\*********************************/
	Solution::~Solution()
	{
	}

	/***********************************\
	|* CLASS: AbstractHTDAlgorithm
	\***********************************/
	AbstractHTDAlgorithm::AbstractHTDAlgorithm(Problem *problem)
		: prob(problem)
	{
	}
	
	AbstractHTDAlgorithm::~AbstractHTDAlgorithm() 
	{ 
	}
	
	Table *AbstractHTDAlgorithm::evaluate(ExtendedHypertree *origroot)
	{
		ExtendedHypertree *root = this->prepareHypertreeDecomposition(origroot);
		return evaluateNode(root);
	}

	void AbstractHTDAlgorithm::addRowToTable(Table& table, Row* r) const
	{
		std::pair<Table::iterator, bool> result = table.insert(r);
		if(!result.second) {
			Row* origRow = *result.first;
			table.erase(result.first);
			r->unify(*origRow);
			delete origRow;
			table.insert(r);
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
	AbstractSemiNormalizedHTDAlgorithm::AbstractSemiNormalizedHTDAlgorithm(Problem *problem)
		: AbstractHTDAlgorithm(problem)
	{ }

	AbstractSemiNormalizedHTDAlgorithm::~AbstractSemiNormalizedHTDAlgorithm() { }

	ExtendedHypertree *AbstractSemiNormalizedHTDAlgorithm::prepareHypertreeDecomposition(ExtendedHypertree *root)
	{
		return root->normalize(SemiNormalization);
	}
	
	Table *AbstractSemiNormalizedHTDAlgorithm::evaluateNode(const ExtendedHypertree *node)
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
	AbstractNormalizedHTDAlgorithm::AbstractNormalizedHTDAlgorithm(Problem *problem)
		: AbstractSemiNormalizedHTDAlgorithm(problem)
	{ }

	AbstractNormalizedHTDAlgorithm::~AbstractNormalizedHTDAlgorithm() { }

	ExtendedHypertree *AbstractNormalizedHTDAlgorithm::prepareHypertreeDecomposition(ExtendedHypertree *root)
	{
		return root->normalize(DefaultNormalization);
	}

	Table *AbstractNormalizedHTDAlgorithm::evaluatePermutationNode(const ExtendedHypertree *node)
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
