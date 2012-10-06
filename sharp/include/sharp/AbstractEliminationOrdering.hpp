#ifndef ABSTRACTELIMINATIONORDERING_H_
#define ABSTRACTELIMINATIONORDERING_H_

namespace sharp
{
	class Hypergraph;
	class Node;

	typedef Node **Ordering; //TODO: better way to pass orderings

	class AbstractEliminationOrdering
	{
	public:
		AbstractEliminationOrdering();
		virtual ~AbstractEliminationOrdering();

	public:
		virtual Ordering getOrdering(Hypergraph *g) = 0;
	};
}

#endif /* ABSTRACTELIMINATIONORDERING_H_ */
