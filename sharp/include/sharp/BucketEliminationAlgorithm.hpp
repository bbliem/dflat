#ifndef BUCKETELIMINATIONALGORITHM_H_
#define BUCKETELIMINATIONALGORITHM_H_

#include <sharp/AbstractHypertreeDecompositionAlgorithm.hpp>

namespace sharp
{
	class AbstractEliminationOrdering;
	
	class BucketEliminationAlgorithm : public AbstractHypertreeDecompositionAlgorithm
	{
	public:
		BucketEliminationAlgorithm(AbstractEliminationOrdering *ordercalc);
		virtual ~BucketEliminationAlgorithm();

	public:
		virtual Hypertree *decompose(Hypergraph *g);

	private:
		AbstractEliminationOrdering *ordercalc;
	};
}

#endif /* BUCKETELIMINATIONALGORITHM_H_ */
