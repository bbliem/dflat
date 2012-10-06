#include <BucketEliminationAlgorithm.hpp>

#include <H_BucketElim.hpp>
#include <AbstractEliminationOrdering.hpp>

using namespace sharp;

BucketEliminationAlgorithm::BucketEliminationAlgorithm(AbstractEliminationOrdering *ordercalc)
{
	//TODO: check not null
	this->ordercalc = ordercalc;
}

BucketEliminationAlgorithm::~BucketEliminationAlgorithm()
{
	if(this->ordercalc) delete this->ordercalc;
}

Hypertree *BucketEliminationAlgorithm::decompose(Hypergraph *g)
{
	H_BucketElim be;
	return be.buildHypertree(g, this->ordercalc->getOrdering(g)); //TODO: implement this here, nice and fast
}
