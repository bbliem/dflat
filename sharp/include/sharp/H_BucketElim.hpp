/*
***Description***
Class H_BucketElim models the bucket elimination heuristic.
*/
//////////////////////////////////////////////////////////////////////

#if !defined(CLS_H_BUCKETELIM)
#define CLS_H_BUCKETELIM

#include <sharp/Global.hpp>

#define BE_INP_ORDER 1  // Input order
#define BE_MIW_ORDER 2  // Minimum induced width order
#define BE_MF_ORDER 3   // Minimum fill-in set order
#define BE_MCS_ORDER 4  // Maximum cardinality search order

namespace sharp
{

	class Hypergraph;
	class Hypertree;
	class Node;
	class CompSet;
	
	class H_BucketElim
	{
	private:
		// Eliminates a given set of buckets and constructs a tree- resp. hypertree-decomposition
		Hypertree *eliminateBuckets(CompSet **Buckets, int iNbrOfBuckets, Node **VarOrder, bool bDual = false);
	
		// Decreases the number of lables in the case of dual bucket elimination
		void minimize(std::set<Node *> *Chi, int iOrderPos, CompSet **Buckets, int iNbrOfBuckets);
	
		// Fills the buckets in the case of bucket elimination
		void fillBuckets1(CompSet **Buckets, int iNbrOfBuckets, Node **VarOrder);
	
		// Fills the buckets in the case of dual bucket elimination
		void fillBuckets2(CompSet **Buckets, int iNbrOfBuckets, Node **VarOrder);
	
		// Improves the variable order by randomly swapping neighbours
		void improveOrderRand(Hypergraph *HGraph, Node **VarOrder, bool bDual = false);
	
		// Improves the variable order by changing the order of conflict-variables
		// i.e., variables occurring in hypertree-nodes with maximum width
		void improveOrderConf(Hypergraph *HGraph, Node **VarOrder, bool bDual = false);
	
	public:
		// Constructor
		H_BucketElim();
	
		// Destructor
		virtual ~H_BucketElim();
	
		// Builds a hypertree according to the bucket elimination heuristic
		Hypertree *buildHypertree(Hypergraph *HGraph, Node **VarOrder, bool bDual = false);
	
		// Builds a hypertree according to the bucket elimination heuristic
		Hypertree *buildHypertree(Hypergraph *HGraph, int iHeuristic, int preElim = 0, bool bDual = false);
	};

} // namespace sharp

#endif // !defined(CLS_H_BUCKETELIM)

