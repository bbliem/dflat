#ifndef MINIMUMDEGREEORDERING_H_
#define MINIMUMDEGREEORDERING_H_

#include <sharp/AbstractEliminationOrdering.hpp>

namespace sharp
{
	class MinimumDegreeOrdering : public AbstractEliminationOrdering
	{
	public:
		MinimumDegreeOrdering();
		virtual ~MinimumDegreeOrdering();

	public:
		Ordering getOrdering(Hypergraph *g);	
	};
}

#endif /* MINIMUMDEGREEORDERING_H_ */
