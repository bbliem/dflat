// H_BucketElim.cpp: implementation of the H_BucketElim class.
//
//////////////////////////////////////////////////////////////////////


#define BE_IMPR_ENVIR 4
#define BE_IMPR_STEPS 8


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctime>

#include <set>
#include <list>

using namespace std;

#include <H_BucketElim.hpp>
#include <Hypertree.hpp>
#include <Hypergraph.hpp>
#include <Hyperedge.hpp>
#include <Node.hpp>
#include <CompSet.hpp>
#include <SetCover.hpp>
#include <Globals.hpp>

namespace sharp
{

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	
	H_BucketElim::H_BucketElim()
	{
	}
	
	
	H_BucketElim::~H_BucketElim()
	{
	}
	
	
	
	//////////////////////////////////////////////////////////////////////
	// Class methods
	//////////////////////////////////////////////////////////////////////
	
	
	/*
	***Description***
	The method builds a tree decomposition from a given set of buckets according to the bucket
	elimination algorithm. It is assumed that each variable is labelled with its index according 
	to VarOrder.
	
	INPUT:	Buckets: Buckets to which the algorithm applies
			iNbrOfBuckets: Number of buckets stored in Buckets
			VarOrder: Order of the underlying variables resp. nodes
	OUTPUT: return: Hypertree decomposition
	
	***History***
	Written: (03.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	Hypertree *H_BucketElim::eliminateBuckets(CompSet **Buckets, int iNbrOfBuckets, Node **VarOrder, bool bDual)
	{
		int iParent, i, j;
		Hypertree *HTree = NULL;
		list<Hypertree *> Independent, **Children;
		list<Hypertree *>::iterator ListIter;
		set<Node *>::iterator ChiIter;
	
		// Create storage space for the children of each hypertree-node
		Children = new list<Hypertree *>*[iNbrOfBuckets];
		if(Children == NULL)
			writeErrorMsg("Error assigning memory.", "H_BucketElim::eliminateBuckets");
		for(i=0; i < iNbrOfBuckets; i++) {
			Children[i] = new list<Hypertree *>;
			if(Children[i] == NULL)
				writeErrorMsg("Error assigning memory.", "H_BucketElim::eliminateBuckets");
		}
	
		// Eliminate buckets in reverse order
		for(i=iNbrOfBuckets-1; i >= 0; i--) {
	
			HTree = new Hypertree();
			if(HTree == NULL)
				writeErrorMsg("Error assigning memory.", "H_BucketElim::eliminateBuckets");
	
			HTree->insChi(VarOrder[i]);
			if(!bDual) {
				// Store the content of the actual bucket in the chi-set
				for(j=0; j < Buckets[i]->size(); j++)
					HTree->insChi((Node *)((*(Buckets[i]))[j]));
			}
			else {
				// Remove the actual node from buckets with higher index
				for(j=i+1; j < iNbrOfBuckets; j++)
					Buckets[j]->remComp(VarOrder[i]);
			}
	
			// Store the content of each child in the actual hypertree-node
			for(ListIter=Children[i]->begin(); ListIter != Children[i]->end(); ListIter++) {
				HTree->insChild(*ListIter);
				for(ChiIter = (*ListIter)->getChi()->begin(); ChiIter != (*ListIter)->getChi()->end(); ChiIter++)
					if(bDual || ((*ChiIter)->getLabel() < i))
						HTree->insChi(*ChiIter);
			}
			if(bDual)
			 	minimize(HTree->getChi(), i, Buckets, iNbrOfBuckets);
	
			if(i > 0) {
				// Find the parent node, i.e., the node that corresponds to the variable with highest index in the chi-set
				iParent = -1;
				for(ChiIter = HTree->getChi()->begin(); ChiIter != HTree->getChi()->end(); ChiIter++) {
					if(bDual && ((*ChiIter)->getLabel() >= i)) {
						for(j=0; j < Buckets[(*ChiIter)->getLabel()]->size(); j++)
							if(((*(Buckets[(*ChiIter)->getLabel()]))[j]->getLabel() < i) && ((*(Buckets[(*ChiIter)->getLabel()]))[j]->getLabel() > iParent))
								iParent = (*(Buckets[(*ChiIter)->getLabel()]))[j]->getLabel();
					}
					else
						if(((*ChiIter)->getLabel() < i) && ((*ChiIter)->getLabel() > iParent))
							iParent = (*ChiIter)->getLabel();
				}
	
				if(iParent < 0)
					// No parent node found: the subtree is independent of all other hypertree-nodes
					Independent.push_back(HTree);
				else
					// Store the subtree as child of the parent node
					Children[iParent]->push_back(HTree);
			}
		}
	
		// Set all independent subtrees as children of the root
		for(ListIter=Independent.begin(); ListIter != Independent.end(); ListIter++)
			HTree->insChild(*ListIter);
	
		for(i=0; i < iNbrOfBuckets; i++)
			delete Children[i];
		delete [] Children;
	
		return HTree;
	}
	
	
	/*
	***Description***
	The method minimizes the chi-set of a hypertree-node in the sense that nodes with index
	higher than the actual index can be replaced by the content of the corresponding bucket if
	this leads to a smaller width.
	
	INPUT:	Chi: Set of nodes that has to be minimized
			iOrderPos: Actual order index
			Buckets: Array of buckets
			iNbrOfBuckets: Number of buckets stored in Buckets
	OUTPUT:
	
	***History***
	Written: (09.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void H_BucketElim::minimize(set<Node *> *Chi, int iOrderPos, CompSet **Buckets, int iNbrOfBuckets)
	{
		int i, j, k, iNbrOfSets, iCard, iPos1, iPos2, iStart, *iOrder;
		bool bSubset;
		Node **Sets;
		list<int> ResSets;
		list<int>::iterator ListIter;
		set<Node *>::iterator ChiIter;
	
		iNbrOfSets = Chi->size();
		Sets = new Node*[iNbrOfSets];
		iOrder = new int[iNbrOfSets];
		if((Sets == NULL) || (iOrder == NULL))
			writeErrorMsg("Error assigning memory.", "H_BucketElim::minimize");
	
		// Store the chi-labels in a local data structure
		for(i=0, ChiIter=Chi->begin(); ChiIter != Chi->end(); i++, ChiIter++)
			Sets[i] = *ChiIter;
		Chi->clear();
	
		// Restore all nodes with index less or equal to the actual index
		for(i=0; i < iNbrOfSets; i++)
			if((Sets[i] != NULL) && (Sets[i]->getLabel() <= iOrderPos)) {
				Chi->insert(Sets[i]);
				// Remove all nodes that occur in the actual chi-set from the resolvable sets
				for(j=0; j < iNbrOfSets; j++)
					if((Sets[j] != NULL) && (Sets[j]->getLabel() > iOrderPos)) {
						Buckets[Sets[j]->getLabel()]->remComp(Sets[i]);
						if(Buckets[Sets[j]->getLabel()]->size() == 0)
							Sets[j] = NULL;
					}
				Sets[i] = NULL;
			}
	
		// Sort resolvable sets according to their size
		for(j=0; j < iNbrOfSets; j++)
			Sets[j] == NULL ? iOrder[j] = 0 : iOrder[j] = -Buckets[Sets[j]->getLabel()]->size();
		sortPointers((void **)Sets, iOrder, 0, iNbrOfSets-1);
	
		// Sort resolvable sets of the same size randomly
		iCard = iStart = 0;
		for(j=0; Sets[j] != NULL; j++)
			if(Buckets[Sets[j]->getLabel()]->size() == iCard)
				iOrder[j] = rand();
			else {
				sortPointers((void **)Sets, iOrder, iStart, j-1);
				iCard = Buckets[Sets[j]->getLabel()]->size();
				iStart = j;
			}
		sortPointers((void **)Sets, iOrder, iStart, j-1);
	
		// Resolve sets with index greater than the actual index if this reduces the chi-set
		for(i=0; i < iNbrOfSets; i++)
			if(Sets[i] != NULL) {
				// Remember the position and size of the actual set
				iPos1 = Sets[i]->getLabel();
				iCard = Buckets[iPos1]->size();
				ResSets.push_back(i);
	
				// Find all sets that are a subset of the actual set
				for(j=i+1; j < iNbrOfSets; j++) 
					if(Sets[j] != NULL) {
						iPos2 = Sets[j]->getLabel();
						bSubset = true;
						for(k=0; k < Buckets[iPos2]->size(); k++)
							if(!Buckets[iPos1]->find((*(Buckets[iPos2]))[k])) {
								bSubset = false;
								break;
							}
		
						// Store the index of each subset
						if(bSubset)
							ResSets.push_back(j);
					}
	
				// Resolve the actual set if the number of subsets is larger than its cardinality
				if(iCard <= (int)ResSets.size()) {
					for(ListIter=ResSets.begin(); ListIter != ResSets.end(); ListIter++)
						Sets[*ListIter] = NULL;
					for(j=0; j < iCard; j++) {
						Chi->insert((Node *)((*(Buckets[iPos1]))[j]));
						// Remove all nodes added to the actual chi-set from the resolvable sets
						for(k=0; k < iNbrOfSets; k++)
							if(Sets[k] != NULL) {
								Buckets[Sets[k]->getLabel()]->remComp((*(Buckets[iPos1]))[j]);
								if(Buckets[Sets[k]->getLabel()]->size() == 0)
									Sets[k] = NULL;
							}
					}
					// Sort resolvable sets according to their size
					for(j=0; j < iNbrOfSets; j++)
						Sets[j] == NULL ? iOrder[j] = 0 : iOrder[j] = -Buckets[Sets[j]->getLabel()]->size();
					sortPointers((void **)Sets, iOrder, 0, iNbrOfSets-1);
	
					// Sort resolvable sets of the same size randomly
					iCard = iStart = 0;
					for(j=0; Sets[j] != NULL; j++)
						if(Buckets[Sets[j]->getLabel()]->size() == iCard)
							iOrder[j] = rand();
						else {
							sortPointers((void **)Sets, iOrder, iStart, j-1);
							iCard = Buckets[Sets[j]->getLabel()]->size();
							iStart = j;
						}
					sortPointers((void **)Sets, iOrder, iStart, j-1);
	
					i=0;  // Restart the resolve process
				}
				ResSets.clear();
			}
	
		// Restore all nodes that were not resolved
		for(i=0; i < iNbrOfSets; i++)
			if(Sets[i] != NULL) {
				if(Buckets[Sets[i]->getLabel()]->size() > 1)
					Chi->insert(Sets[i]);
				else
					Chi->insert((Node *)(*Buckets[Sets[i]->getLabel()])[0]);
			}
	
		delete [] Sets;
		delete [] iOrder;
	}
	
	
	/*
	***Description***
	The method fills each bucket i with all variables in hyperedges that contain a variable with 
	index i and no variable with index higher than i. It is assumed that each variable is labelled
	with its index according to VarOrder.
	
	INPUT:	Buckets: Buckets to be filled
			iNbrOfBuckets: Number of buckets
			VarOrder: Order of the underlying variables resp. nodes
	OUTPUT:
	
	***History***
	Written: (16.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void H_BucketElim::fillBuckets1(CompSet **Buckets, int iNbrOfBuckets, Node **VarOrder)
	{
		int iBucketSize, i, j, k;
		bool bInsert;
	
		// Build and initialize buckets
		for(i=0; i < iNbrOfBuckets; i++) {
			// Compute the size of the bucket, i.e., the sum over the cardinalities of hyperedges with highest variable index i
			iBucketSize = 0;
			for(j=0; j < VarOrder[i]->getNbrOfEdges(); j++)
				// If the edge has not been inserted
				if(VarOrder[i]->getEdge(j)->getLabel() == 0) {
					bInsert = true;
					for(k=0; k < VarOrder[i]->getEdge(j)->getNbrOfNodes(); k++)
						// If the edge contains a variable with index greater than i
						if(VarOrder[i]->getEdge(j)->getNode(k)->getLabel() > VarOrder[i]->getLabel()) {
							bInsert = false;
							break;
						}
	
					// If the highest index over alle variables in the edge is i
					if(bInsert) {
						iBucketSize += VarOrder[i]->getEdge(j)->getNbrOfNodes();
						VarOrder[i]->getEdge(j)->setLabel(1);
					}
				}
			
			// Create bucket
			Buckets[i] = new CompSet(iBucketSize);
			if(Buckets[i] == NULL)
				writeErrorMsg("Error assigning memory.", "H_BucketElim::fillBuckets1");
	
			// Copy all nodes of the edge into the bucket
			for(j=0; j < VarOrder[i]->getNbrOfEdges(); j++)
				if(VarOrder[i]->getEdge(j)->getLabel() > 0) {
					VarOrder[i]->getEdge(j)->setLabel(-1);
					for(k=0; k < VarOrder[i]->getEdge(j)->getNbrOfNodes(); k++)
						Buckets[i]->insComp(VarOrder[i]->getEdge(j)->getNode(k));
				}
			Buckets[i]->remComp(VarOrder[i]);
		}
	}
	
	
	/*
	***Description***
	The method fills each bucket i with the variables in hyperedges that contain a variable with 
	index i that have the highest index less than i. It is assumed that each variable is labelled
	with its index according to VarOrder.
	
	INPUT:	Buckets: Buckets to be filled
			iNbrOfBuckets: Number of buckets
			VarOrder: Order of the underlying variables resp. nodes
	OUTPUT:
	
	***History***
	Written: (16.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void H_BucketElim::fillBuckets2(CompSet **Buckets, int iNbrOfBuckets, Node **VarOrder)
	{
		int iMax, i, j, k;
	
		// Build and initialize buckets
		for(i=0; i < iNbrOfBuckets; i++) {
	
			// Create bucket
			Buckets[i] = new CompSet(VarOrder[i]->getNbrOfEdges());
			if(Buckets[i] == NULL)
				writeErrorMsg("Error assigning memory.", "H_BucketElim::fillBuckets2");
	
			// Copy the node with the highest index less than i into the bucket
			for(j=0; j < VarOrder[i]->getNbrOfEdges(); j++) {
				// Find a node with index less than i
				iMax = -1;
				for(k=0; k < VarOrder[i]->getEdge(j)->getNbrOfNodes(); k++)
					if(VarOrder[i]->getEdge(j)->getNode(k)->getLabel() < i) {
						iMax = k;
						break;
					}
	
				if(iMax >= 0) {
					// Finde the node with highest index less than i
					for(k=iMax+1; k < VarOrder[i]->getEdge(j)->getNbrOfNodes(); k++)
						if((VarOrder[i]->getEdge(j)->getNode(k)->getLabel() < i) && 
							(VarOrder[i]->getEdge(j)->getNode(k)->getLabel() > VarOrder[i]->getEdge(j)->getNode(iMax)->getLabel()))
							iMax = k;
	
					// Copy the node into the bucket
					Buckets[i]->insComp(VarOrder[i]->getEdge(j)->getNode(iMax));
				}
			}
		}
	}
	
	
	/*
	***Description***
	The method improves a variable ordering in such a way that the corresponding hypertree-width
	becomes smaller. This is done by randomly swapping neighboring variables.
	
	INPUT:	HGraph: Hypergraph that has to be decomposed
			VarOrder: Given order of variables
			iNbrOfImprSteps: Number of improvements steps
			bDual: true if dual bucket elimination should be used; otherwise false
	OUTPUT: VarOrder: Improved order of variables
	
	***History***
	Written: (11.04.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void H_BucketElim::improveOrderRand(Hypergraph *HGraph, Node **VarOrder, bool bDual)
	{
		int iNbrOfVars, iOldWidth, iNewWidth, iPos;
		Node *Tmp;
		Hypertree *HTree;
	
		iNbrOfVars = HGraph->getNbrOfNodes();
		if(iNbrOfVars > 1) {
			// Compute hypertree-width before improvement step
			HTree = buildHypertree(HGraph, VarOrder, bDual);
			iOldWidth = HTree->getHTreeWidth();
			delete HTree;
	
			for(int i=0; i < BE_IMPR_STEPS; i++) {
				// Randomly select a variable to be swapped with its neighbour
				iPos = random_range(0, iNbrOfVars-2);
				Tmp = VarOrder[iPos];
				VarOrder[iPos] = VarOrder[iPos+1];
				VarOrder[iPos+1] = Tmp;
	
				// Compute hypertree-width after improvement step
				HTree = buildHypertree(HGraph, VarOrder, bDual);
				iNewWidth = HTree->getHTreeWidth();
				delete HTree;
	
				if(iOldWidth < iNewWidth) {
					// Undo variable swapping if hypertree-width did not decrease
					Tmp = VarOrder[iPos];
					VarOrder[iPos] = VarOrder[iPos+1];
					VarOrder[iPos+1] = Tmp;
				}
				else
					// Update hypertree-width if hypertree-width decreased
					iOldWidth = iNewWidth;
			}
		}
	}
	
	
	/*
	***Description***
	The method improves a variable ordering in such a way that the corresponding hypertree-width
	becomes smaller. This is done by changing the order-position of variables occurring in 
	hypertree-nodes with maximal width.
	
	INPUT:	HGraph: Hypergraph that has to be decomposed
			VarOrder: Given order of variables
			iNbrOfImprSteps: Number of improvements steps
			bDual: true if dual bucket elimination should be used; otherwise false
	OUTPUT: VarOrder: Improved order of variables
	
	***History***
	Written: (21.04.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void H_BucketElim::improveOrderConf(Hypergraph *HGraph, Node **VarOrder, bool bDual)
	{
		int iNbrOfVars, iOldWidth, iNewWidth, iPos, i, j;
		Node *Tmp;
		Hypertree *HTree;
		set<Node *> ConflictVars;
		set<Node *>::iterator SetIter;
	
		iNbrOfVars = HGraph->getNbrOfNodes();
	
		// Compute hypertree-width and initial conflict-set before improvement step
		HTree = buildHypertree(HGraph, VarOrder, bDual);
		iOldWidth = HTree->getHTreeWidth();
		HTree->getConflictVars(&ConflictVars, iOldWidth, bDual);
		delete HTree;
	
		for(i=0; i < BE_IMPR_STEPS; i++) {
			// Randomly select a variable in the conflict-set
			iPos = random_range(0, ConflictVars.size()-1);
			for(SetIter=ConflictVars.begin(); iPos > 0; iPos--, SetIter++);
			for(iPos=0; VarOrder[iPos] != *SetIter; iPos++);
	
			// Swap selected variable with all variables in its environment
			iPos-BE_IMPR_ENVIR < 0 ? j=0 : j=iPos-BE_IMPR_ENVIR;
			for(; (j <= iPos+BE_IMPR_ENVIR) && (j < iNbrOfVars); j++)
				if(j != iPos) {
					// Swap variables
					Tmp = VarOrder[iPos];
					VarOrder[iPos] = VarOrder[j];
					VarOrder[j] = Tmp;
	
					// Compute hypertree-width after improvement step
					HTree = buildHypertree(HGraph, VarOrder, bDual);
					iNewWidth = HTree->getHTreeWidth();
	
					if(iOldWidth < iNewWidth) {
						// Undo variable swapping if hypertree-width did not decrease
						Tmp = VarOrder[iPos];
						VarOrder[iPos] = VarOrder[j];
						VarOrder[j] = Tmp;
					}
					else {
						// Update hypertree-width and conflict-set if hypertree-width decreased
						iOldWidth = iNewWidth;
						ConflictVars.clear();
						HTree->getConflictVars(&ConflictVars, iOldWidth, bDual);
						break;
					}
					delete HTree;
				}
		}
	}
	
	
	/*
	***Description***
	The method builds a tree decomposition of a given hypergraph according to the bucket 
	elimination algorithm.
	
	INPUT:	HGraph: Hypergraph that has to be decomposed
			VarOrder: Variable order
			bDual: true if dual bucket elimination should be used; otherwise false
	OUTPUT: return: Hypertree decomposition of HGraph
	
	***History***
	Written: (11.04.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	Hypertree *H_BucketElim::buildHypertree(Hypergraph *HGraph, Node **VarOrder, bool bDual)
	{
		int iNbrOfBuckets, i;
		Hypertree *HTree = NULL, *HTreeD = NULL;
		CompSet **Buckets;
	
		iNbrOfBuckets = HGraph->getNbrOfNodes();
		Buckets = new CompSet*[iNbrOfBuckets];
		if(Buckets == NULL)
			writeErrorMsg("Error assigning memory.", "H_BucketElim::buildHypertree");
	
		// Reset edge labels and store the variable order in the node labels
		HGraph->resetEdgeLabels();
		for(i=0; i < iNbrOfBuckets; i++)
			VarOrder[i]->setLabel(i);
	
		if(!bDual)
			fillBuckets1(Buckets, iNbrOfBuckets, VarOrder);
		else
			fillBuckets2(Buckets, iNbrOfBuckets, VarOrder);
	
		// Build a hypertree decomposition by applying bucket elimination
		HTree = eliminateBuckets(Buckets, iNbrOfBuckets, VarOrder, false);
		if(bDual)
			HTreeD = eliminateBuckets(Buckets, iNbrOfBuckets, VarOrder, true);
	
		for(i=0; i < iNbrOfBuckets; i++)
			delete Buckets[i];
		delete [] Buckets;
	
		// Finish the hypertree construction
		HTree->shrink();
		if(bDual) {
			HGraph->makeDual();
			HTree->swapChiLambda();
			HTree->reduceLambda();
			HTree->swapChiLambda();
			HTree->shrink();
			HTree->swapChiLambda();
			HTree->reduceLambda();
			HTree->setChi(HGraph);
			HTree->elimCovEdges();
			HTree->resetLambda(HGraph);
	
			HTreeD->shrink();
			HTreeD->swapChiLambda();
			HTreeD->reduceLambda();
			HTreeD->swapChiLambda();
			HTreeD->shrink();
			HTreeD->swapChiLambda();
			HTreeD->reduceLambda();
			HTreeD->setChi(HGraph);
			HTreeD->elimCovEdges();
			HTreeD->resetLambda(HGraph);
			HGraph->makeDual();
	
			if(HTreeD->getHTreeWidth() < HTree->getHTreeWidth()) {
				delete HTree;
				HTree = HTreeD;
			}
			HTree->shrink();
		}
		else
			HTree->setLambda(HGraph);
	
		return HTree;
	}
	
	
	/*
	***Description***
	The method builds a tree decomposition of a given hypergraph according to the bucket 
	elimination algorithm.
	
	INPUT:	HGraph: Hypergraph that has to be decomposed
			iHeuristic: Heuristic to compute the underlying variable order
			bDual: true if dual bucket elimination should be used; otherwise false
	OUTPUT: return: Hypertree decomposition of HGraph
	
	***History***
	Written: (22.02.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	Hypertree *H_BucketElim::buildHypertree(Hypergraph *HGraph, int iHeuristic, int preElim, bool bDual)
	{
		Hypertree *HTree;
		Node **VarOrder = NULL;
	
		if(bDual)
			HGraph->makeDual();
	
		// Determine the underlying variable order
		switch(iHeuristic) {
			case BE_INP_ORDER: VarOrder = HGraph->getInputOrder(preElim);
				break;
			case BE_MIW_ORDER: VarOrder = HGraph->getMIWOrder(preElim);
				break;
			case BE_MF_ORDER: VarOrder = HGraph->getMFOrder(preElim);
				break;
			case BE_MCS_ORDER: VarOrder = HGraph->getMCSOrder(preElim);
				break;
			default: writeErrorMsg("Unknown variable order heuristic.", "H_BucketElim::buildHypertree");
		}
	
		// Improve ordering by various heuristics
		// improveOrderRand(HGraph, VarOrder, bDual);
		// improveOrderConf(HGraph, VarOrder, bDual);
	
		HTree = buildHypertree(HGraph, VarOrder, bDual);
	
		delete [] VarOrder;
		if(bDual)
			HGraph->makeDual();
	
		return HTree;
	}

} // namespace sharp
