#ifndef MINIMUMFILLORDERING_H_
#define MINIMUMFILLORDERING_H_

#include <sharp/AbstractEliminationOrdering.hpp>

namespace sharp
{
	class MinimumFillOrdering : public AbstractEliminationOrdering
	{
	public:
		MinimumFillOrdering();
		virtual ~MinimumFillOrdering();

	public:
		Ordering getOrdering(Hypergraph *g);	
	};
}

#endif /* MINIMUMFILLORDERING_H_ */
