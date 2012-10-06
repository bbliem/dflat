#ifndef ABSTRACTHYPERTREEDECOMPOSITIONALGORITHM_H_
#define ABSTRACTHYPERTREEDECOMPOSITIONALGORITHM_H_

namespace sharp
{
	class Hypergraph;
	class Hypertree;

	class AbstractHypertreeDecompositionAlgorithm
	{
	public:
		AbstractHypertreeDecompositionAlgorithm();
		virtual ~AbstractHypertreeDecompositionAlgorithm();

	public:
		virtual Hypertree *decompose(Hypergraph *g) = 0;
	};
}

#endif /* ABSTRACTHYPERTREEDECOMPOSITIONALGORITHM_H_ */
