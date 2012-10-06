#ifndef EXTENDEDHYPERTREE_H_
#define EXTENDEDHYPERTREE_H_

#include <sharp/Global.hpp>
#include <sharp/Hypertree.hpp>

namespace sharp
{
	enum TreeNodeType
	{
		General = 0,
		Leaf = 1, 
		Branch = 2, 
		Removal = 3, 
		Introduction = 4,
		Permutation = 5
	};

	enum NormalizationType
	{
		DefaultNormalization,
		SemiNormalization,
		StrongNormalization,
		NoNormalization
	};
		
	class ExtendedHypertree : public Hypertree
	{
	public:
		ExtendedHypertree(Hypertree *node);
		ExtendedHypertree(const VertexSet &vertices);
		virtual ~ExtendedHypertree();

		ExtendedHypertree *normalize(NormalizationType type = DefaultNormalization);

		const VertexSet &getVertices() const;
		TreeNodeType getType() const;
		bool isRoot() const;
		virtual int getTreeWidth();
		//virtual int getHTreeWidth();
		const VertexSet &getIntroducedVertices() const;
		const VertexSet &getRemovedVertices() const;
		Vertex getDifference() const;

		ExtendedHypertree *parent() const;
		ExtendedHypertree *firstChild() const;
		ExtendedHypertree *secondChild() const;

		double getAverageWidth();
		unsigned int getWidthSum();
		unsigned int getJoinNodeWidthSum();
		double getAverageJoinNodeWidth();
		unsigned int getNodeCountWithWidth(unsigned int width);
		double getAverageJoinLeafDistance();
		double getAverageJoinJoinDistance();
		double getJoinNodePercentage();
		unsigned int getDistanceToNearestLeaf();
		unsigned int getDistanceToNearestJoinAncestor();
		unsigned int getJoinLeafDistanceSum();
		unsigned int getJoinJoinDistanceSum();
		virtual unsigned int getNumberOfJoinNodes();

	private:
		TreeNodeType type;

		VertexSet vertices;
		VertexSet introduced;
		VertexSet removed;
		
		void adapt();

		ExtendedHypertree *createNormalizedJoinNode(
				ExtendedHypertree *left, 
				ExtendedHypertree *right, 
				const VertexSet &top, 
				NormalizationType normalization);

		static ExtendedHypertree *createChild(ExtendedHypertree *child, 
				const VertexSet &vertices, 
				Vertex difference, 
				TreeNodeType type);

#ifdef DEBUG
	public: //PRINTING FUNCTIONS
		void print();
#endif
	};
}

#endif
