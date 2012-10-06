#ifndef MAXIMUMCARDINALITYSEARCHORDERING_H_
#define MAXIMUMCARDINALITYSEARCHORDERING_H_

#include <sharp/AbstractEliminationOrdering.hpp>

namespace sharp
{
	class MaximumCardinalitySearchOrdering : public AbstractEliminationOrdering
	{
	public:
		MaximumCardinalitySearchOrdering();
		virtual ~MaximumCardinalitySearchOrdering();

	public:
		Ordering getOrdering(Hypergraph *g);	
	};
}

#endif /* MAXIMUMCARDINALITYSEARCHORDERING_H_ */
