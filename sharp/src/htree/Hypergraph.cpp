// Hypergraph.cpp: implementation of the Hypergraph class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <Hypergraph.hpp>
#include <Parser.hpp>
#include <Hyperedge.hpp>
#include <Node.hpp>
#include <Globals.hpp>

#include <CompSet.hpp>
#include <SetCover.hpp>
#include <probSol.hpp>
#include <TabuSearch.hpp>

static int G_EdgeID = 0;

using namespace sharp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Hypergraph::Hypergraph(bool isGraph)
{
	iMyMaxNbrOfEdges = 0;
	iMyMaxNbrOfNodes = 0;
	avgFill = minFill = maxFill = -1;
	this->isgraph = isGraph;
}


Hypergraph::~Hypergraph()
{
	int i;

	for(i=0; i < iMyMaxNbrOfEdges; i++)
		delete MyEdges[i];
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		delete MyNodes[i];
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


/*
***Description***
The method labels all hyperedges reachable from Edge.

INPUT:	Edge: Hyperedge
OUTPUT: 

***History:***
Written: (28.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::labelReachEdges(Hyperedge *Edge)
{
	Edge->setLabel(1);
	for(int i=0; i < Edge->getNbrOfNeighbours(); i++)
		if(Edge->getNeighbour(i)->getLabel() == 0)
			labelReachEdges(Edge->getNeighbour(i));
}



// Returns the number of edges in the hypergraph
int Hypergraph::getNbrOfEdges()
{
	return iMyMaxNbrOfEdges;
}


// Returns the number of nodes in the hypergraph
int Hypergraph::getNbrOfNodes()
{
	return iMyMaxNbrOfNodes;
}


// Returns the hyperedge stored at position iPos
Hyperedge *Hypergraph::getEdge(int iPos)
{
	if(iPos < iMyMaxNbrOfEdges)
		return MyEdges[iPos];
	else {
		writeErrorMsg("Position not available.", "Hypergraph::getEdge");
		return NULL;
	}
}


// Returns the node stored at position iPos
Node *Hypergraph::getNode(int iPos)
{
	if(iPos < iMyMaxNbrOfNodes)
		return MyNodes[iPos];
	else {
		writeErrorMsg("Position not available.", "Hypergraph::getNode");
		return NULL;
	}
}


// Returns the hyperedge with ID iID
Hyperedge *Hypergraph::getEdgeByID(int iID)
{
	Hyperedge *HEdge = NULL;

	for(int i=0; i < iMyMaxNbrOfEdges; i++)
		if(MyEdges[i]->getID() == iID) {
			HEdge = MyEdges[i];
			break;
		}

	return HEdge;	
}


// Returns the node with ID iID
Node *Hypergraph::getNodeByID(int iID)
{
	Node *Node = NULL;

	for(int i=0; i < iMyMaxNbrOfNodes; i++)
		if(MyNodes[i]->getID() == iID) {
			Node = MyNodes[i];
			break;
		}

	return Node;	
}


// Sets labels of all edges to zero
void Hypergraph::resetEdgeLabels(int iVal)
{
	for(int i=0; i < iMyMaxNbrOfEdges; i++)
		MyEdges[i]->setLabel(iVal);
}


// Sets labels of all nodes to zero
void Hypergraph::resetNodeLabels(int iVal)
{
	for(int i=0; i < iMyMaxNbrOfNodes; i++)
		MyNodes[i]->setLabel(iVal);
}


/*
***Description***
The method checks whether the hypergraph is connected, i.e., whether it consists of a single
component.

INPUT:
OUTPUT:	return: true if the hypergraph is connected; otherwise false

***History:***
Written: (28.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

bool Hypergraph::isConnected()
{
	// Label all edges reachable from the first edge
	resetEdgeLabels();
	if(iMyMaxNbrOfEdges > 0)
		labelReachEdges(MyEdges[0]);

	// Check whether all edges are labeled
	for(int i=0; i < iMyMaxNbrOfEdges; i++)
		if(MyEdges[i]->getLabel() == 0)
			return false;

	return true;
}

bool Hypergraph::isGraph() const
{
	return this->isgraph;
}

/*
***Description***
The method transforms the hypergraph into its dual hypergraph.

INPUT:
OUTPUT:

***History:***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::makeDual()
{
	int iTmp;
	vector<Node *> Nodes; Nodes.reserve(this->MyEdges.size());
	vector<Hyperedge *> Edges; Edges.reserve(this->MyNodes.size());

	// Swap hyperedges and nodes
	for(int i = 0; i < (int)this->MyEdges.size(); ++i)
		Nodes[i] = (Node *)this->MyEdges[i];

	for(int i = 0; i < (int)this->MyNodes.size(); ++i)
		Edges[i] = (Hyperedge *)this->MyNodes[i];

	MyEdges = Edges;
	MyNodes = Nodes;

	// Swap limiters
	iTmp = iMyMaxNbrOfEdges;
	iMyMaxNbrOfEdges = iMyMaxNbrOfNodes;
	iMyMaxNbrOfNodes = iTmp;
}


/*
***Description***
The method removes hyperedges that are covered by another hyperedge.

INPUT:	bFinalOnly: Indicates whether to check only the last hyperedge for covering.
OUTPUT:

***History:***
Written: (08.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::reduce(bool bFinalOnly)
{
	int i, j, k;
	bool bCovered;
	list<int>::iterator ListIter;

	resetNodeLabels();
	for(i=0; i < iMyMaxNbrOfEdges; i++) {
		// Label all nodes in the hyperedge
		for(k=0; k < MyEdges[i]->getNbrOfNodes(); k++)
			MyEdges[i]->getNode(k)->setLabel(1);
		
		// Search for a hyperedge that is covered, i.e. whose nodes are labelled
		bFinalOnly && (i < iMyMaxNbrOfEdges-1) ? j=iMyMaxNbrOfEdges-1 : j=0;
		for(; j < iMyMaxNbrOfEdges; j++)
			if(j != i) {
				bCovered = true;
				for(k=0; k < MyEdges[j]->getNbrOfNodes(); k++)
					if(MyEdges[j]->getNode(k)->getLabel() == 0) {
						bCovered = false;
						break;
					}

				if(bCovered) {
					MyEdges[i]->insCoveredID(MyEdges[j]->getID());
					for(ListIter = MyEdges[j]->getCoveredIDs()->begin(); ListIter != MyEdges[j]->getCoveredIDs()->end(); ListIter++)
						MyEdges[i]->insCoveredID(*ListIter);

					// Remove the hyperedge
					--iMyMaxNbrOfEdges;
					for(k=0; k < MyEdges[j]->getNbrOfNodes(); k++)
						MyEdges[j]->getNode(k)->remEdge(MyEdges[j]);
					for(k=0; k < MyEdges[j]->getNbrOfNeighbours(); k++)
						MyEdges[j]->getNeighbour(k)->remNeighbour(MyEdges[j]);
					for(k=j; k < iMyMaxNbrOfEdges; k++)
						MyEdges[k] = MyEdges[k+1];
					MyEdges[iMyMaxNbrOfEdges] = NULL;
					if(j < i) --i; --j;
				}
			}
		
		// Reset all node labels in the hyperedge
		for(k=0; k < MyEdges[i]->getNbrOfNodes(); k++)
			MyEdges[i]->getNode(k)->setLabel(0);
	}
}


/*
***Description***
The method updates the neighbourhood relation of all hyperedges and nodes, i.e., it resets their
neighbours.

INPUT:
OUTPUT:

***History:***
Written: (06.05.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::updateNeighbourhood()
{
	int i;

	for(i=0; i < iMyMaxNbrOfNodes; i++)
		MyNodes[i]->updateNeighbourhood();
	for(i=0; i < iMyMaxNbrOfEdges; i++)
		MyEdges[i]->updateNeighbourhood();
}


/*
***Description***
The method updates the sizes of all hyperedges and nodes, i.e., it reduces their size if NULL
pointers are occurring.

INPUT:
OUTPUT:

***History:***
Written: (09.05.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::updateCompSizes()
{
	int i;

	for(i=0; i < iMyMaxNbrOfNodes; i++)
		MyNodes[i]->updateSize();
	for(i=0; i < iMyMaxNbrOfEdges; i++)
		MyEdges[i]->updateSize();
}


/*
***Description***
The method divides a hypergraph into a subgraph according to a given assignment of each
variable in the hypergraph to a partition. It is assumed that the partitions are numbered
from 0 to n.

INPUT:	iPartition: An array of integers assigning variable i the i-th partition in the array
OUTPUT:	Subgraphs:	Array of pointers to subgraphs finalized by NULL
		Separator:	Array of pointers to hyperedges finalized by NULL

***History:***
Written: (06.05.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::divideNodes(int *iPartitioning, Hypergraph ***Subgraphs, Hyperedge ***Separator)
{
	int *iPartSizes, iNbrOfParts, iPart, i, j, k;
	bool bRemove;
	Hyperedge *HEdge;
	Node *HNode;
	vector<Hyperedge *> Separators;
	vector<Hyperedge *>::iterator SepIter;
	set<int> TmpNodes, TmpEdges;
	set<int>::iterator TmpIter;

	// Compute number of partitions
	iNbrOfParts = 0;
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		if(iPartitioning[i]+1 > iNbrOfParts)
			iNbrOfParts = iPartitioning[i]+1;

	// Create a subgraph for each partition
	*Subgraphs = new Hypergraph*[iNbrOfParts+1];
	if(*Subgraphs == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::divideNodes");
	for(i=0; i < iNbrOfParts; i++) {
		(*Subgraphs)[i] = new Hypergraph();
		if((*Subgraphs)[i] == NULL)
			writeErrorMsg("Error assigning memory.", "Hypergraph::divideNodes");
	}
	(*Subgraphs)[iNbrOfParts] = NULL;

	// Create an auxiliary counter for each partition
	iPartSizes = new int[iNbrOfParts];
	if(iPartSizes == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::divideNodes");

	// Label each node and each edge with its partition and with -1 if it is a separator
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		MyNodes[i]->setLabel(iPartitioning[i]);
	for(i=0; i < iMyMaxNbrOfEdges; i++) {
		iPart = MyEdges[i]->getNode(0)->getLabel();
		MyEdges[i]->setLabel(iPart);
		for(j=1; j < MyEdges[i]->getNbrOfNodes(); j++)
			if(MyEdges[i]->getNode(j)->getLabel() != iPart) {
				MyEdges[i]->setLabel(-1);
				break;
			}
	}
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		bRemove = true;
		for(j=0; j < MyNodes[i]->getNbrOfEdges(); j++)
			if(MyNodes[i]->getEdge(j)->getLabel() != -1) {
				bRemove = false;
				break;
			}
		if(bRemove) {
			MyNodes[i]->setLabel(-1);
			iPartitioning[i] = -1;
		}
	}

	// Insert nodes into each subgraph
	for(i=0; i < iNbrOfParts; i++)
		iPartSizes[i] = 0;
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		if(MyNodes[i]->getLabel() >= 0)
			++iPartSizes[iPartitioning[i]];
	for(i=0; i < iNbrOfParts; i++) {
		(*Subgraphs)[i]->iMyMaxNbrOfNodes = iPartSizes[i];
		(*Subgraphs)[i]->MyNodes.reserve(iPartSizes[i]);
	}
	for(i=0; i < iNbrOfParts; i++)
		iPartSizes[i] = 0;
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		if(MyNodes[i]->getLabel() >= 0)
			(*Subgraphs)[iPartitioning[i]]->MyNodes[iPartSizes[iPartitioning[i]]++] = MyNodes[i]->clone();

	// Insert edges into each subgraph
	for(i=0; i < iNbrOfParts; i++)
		iPartSizes[i] = 0;
	for(i=0; i < iMyMaxNbrOfEdges; i++)
		if(MyEdges[i]->getLabel() >= 0)
			++iPartSizes[MyEdges[i]->getLabel()];
	for(i=0; i < iNbrOfParts; i++) {
		(*Subgraphs)[i]->iMyMaxNbrOfEdges = iPartSizes[i];
		(*Subgraphs)[i]->MyEdges.reserve(iPartSizes[i]+1);
	}
	for(i=0; i < iNbrOfParts; i++)
		iPartSizes[i] = 0;
	for(i=0; i < iMyMaxNbrOfEdges; i++)
		if(MyEdges[i]->getLabel() >= 0)
			(*Subgraphs)[MyEdges[i]->getLabel()]->MyEdges[iPartSizes[MyEdges[i]->getLabel()]++] = MyEdges[i]->clone();
		else
			Separators.push_back(MyEdges[i]);

	delete [] iPartSizes;

	// Construct a special hyperedge for each partition
	for(i=0; i < iNbrOfParts; i++) {
		for(SepIter = Separators.begin(); SepIter != Separators.end(); SepIter++)
			for(j=0; j < (*SepIter)->getNbrOfNodes(); j++)
				if((*SepIter)->getNode(j)->getLabel() == i) {
					TmpNodes.insert((*SepIter)->getNode(j)->getID());
					for(k=0; k < (*SepIter)->getNode(j)->getNbrOfEdges(); k++)
						if((*SepIter)->getNode(j)->getEdge(k)->getLabel() == i)
							TmpEdges.insert((*SepIter)->getNode(j)->getEdge(k)->getID());
				}
		
		if((TmpNodes.size() > 0) && (TmpEdges.size() > 0)) {
			HEdge = new Hyperedge(G_EdgeID++, 0, TmpNodes.size(), TmpEdges.size());
			for(j=k=0; j < (*Subgraphs)[i]->iMyMaxNbrOfNodes; j++)
				if(TmpNodes.find((*Subgraphs)[i]->MyNodes[j]->getID()) != TmpNodes.end()) {
					HEdge->insNode((*Subgraphs)[i]->MyNodes[j], k++);
					(*Subgraphs)[i]->MyNodes[j]->insEdge(HEdge, 0);
				}
			(*Subgraphs)[i]->MyEdges[(*Subgraphs)[i]->iMyMaxNbrOfEdges] = HEdge;
			(*Subgraphs)[i]->iMyMaxNbrOfEdges++;
		}
		TmpNodes.clear();
		TmpEdges.clear();
	}

	// Update relations between nodes and hyperedges
	for(i=0; i < iMyMaxNbrOfEdges; i++) {
		iPart = MyEdges[i]->getLabel();
		if(iPart >= 0) {
			for(j=0; j < MyEdges[i]->getNbrOfNodes(); j++)
				TmpNodes.insert(MyEdges[i]->getNode(j)->getID());
			
			HEdge = (*Subgraphs)[iPart]->getEdgeByID(MyEdges[i]->getID());
			for(j=0, TmpIter = TmpNodes.begin(); TmpIter != TmpNodes.end(); j++, TmpIter++) {
				HNode = (*Subgraphs)[iPart]->getNodeByID(*TmpIter);
				if(HNode == NULL)
					writeErrorMsg("Node not found.", "Hypergraph::divideNodes");
				HEdge->insNode(HNode, j);
				for(k=0; HNode->getEdge(k) != NULL; k++);
				HNode->insEdge(HEdge, k);
			}
			TmpNodes.clear();
		}
	}
	for(i=0; i < iNbrOfParts; i++) {
		(*Subgraphs)[i]->updateCompSizes();
		(*Subgraphs)[i]->updateNeighbourhood();
	}

	// Store separators in an array
	*Separator = new Hyperedge*[Separators.size()+1];
	if(*Separator == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::divideNodes");
	for(i=0, SepIter = Separators.begin(); SepIter != Separators.end(); i++, SepIter++)
		(*Separator)[i] = *SepIter;
	(*Separator)[i] = NULL;
}


/*
***Description***
The method returns the nodes of the underlying hypergraph in the same order as they are stored 
in the hypergraph.

INPUT:
OUTPUT: return: Order of nodes

***History***
Written: (22.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node **Hypergraph::getInputOrder(int preElim)
{
	int i;
	Node **VarOrder;

	VarOrder = new Node*[iMyMaxNbrOfNodes+1];
	if(VarOrder == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getInputOrder");

	// Copy pointers to the nodes from the hypergraph into the array
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		VarOrder[i] = MyNodes[i];
	VarOrder[i] = NULL;

	return VarOrder;
}


/*
***Description***
Sets the order of the nodes in hypergraph with MCS

INPUT:
OUTPUT: 

***History***
Written: (06.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::setMCSOrder()
{
	int i;
	Node **VarOrder;

	VarOrder = this->getMCSOrder();
	if(VarOrder == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getInputOrder");

	// Copy pointers to the nodes from the hypergraph into the array
	for(i=0; i < iMyMaxNbrOfNodes; i++)
	{	MyNodes[i]=VarOrder[i];
	    VarOrder[i] = NULL;
	}


	delete [] VarOrder;
}




/*
***Description***
Sets the order of the nodes in hypergraph with Min-fill heuristic

INPUT:
OUTPUT: 

***History***
Written: (06.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypergraph::setMFOrder()
{
	int i;
	Node **VarOrder;

	VarOrder = this->getMFOrder();
	if(VarOrder == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getInputOrder");

	// Copy pointers to the nodes from the hypergraph into the array
	for(i=0; i < iMyMaxNbrOfNodes; i++)
	{	MyNodes[i]=VarOrder[i];
	    VarOrder[i] = NULL;
	}


	delete [] VarOrder;
}







/*
***Description***
The method returns the nodes of the underlying hypergraph in the order determined by the
minimum induced width (MIW) heuristic.

INPUT:
OUTPUT: return: Order of nodes

***History***
Written: (22.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node **Hypergraph::getMIWOrder(int preElim)
{
	int *iOrder, iMinDegree, iMinDegreePos, i, j;
	Node **VarOrder;
	set<Node *> **NodeNeighbours;
	set<Node *>::iterator SetIter, SetIter2;
	vector<int> Candidates;

	iOrder = new int[iMyMaxNbrOfNodes];
	VarOrder = new Node*[iMyMaxNbrOfNodes+1];
	if((iOrder == NULL) || (VarOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");

	// Create storage space for the neighbours of each node
	NodeNeighbours = new set<Node *>*[iMyMaxNbrOfNodes];
	if(NodeNeighbours == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		NodeNeighbours[i] = new set<Node *>;
		if(NodeNeighbours[i] == NULL)
			writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");
	}

	// Initialize variable order and set the set of neighbours of each node
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		VarOrder[i] = MyNodes[i];
		VarOrder[i]->setLabel(i);
		for(j=0; j < VarOrder[i]->getNbrOfNeighbours(); j++)
			NodeNeighbours[i]->insert(VarOrder[i]->getNeighbour(j));
	}
	VarOrder[i] = NULL;

	// Remove nodes with smallest degree iteratively
	for(i=0; i < iMyMaxNbrOfNodes; i++) {

		if(i < preElim)
		{
			iMinDegreePos = i;
		}
		else
		{
			// Search for the first node that has not been removed yet
			for(j=0; NodeNeighbours[j] == NULL; j++);
			iMinDegree = NodeNeighbours[j]->size();
			Candidates.push_back(j);

			// Search for the node with smallest degree
			for(++j; j < iMyMaxNbrOfNodes; j++)
				if(NodeNeighbours[j] != NULL)
					if((int)NodeNeighbours[j]->size() <= iMinDegree) 
					{
						if((int)NodeNeighbours[j]->size() < iMinDegree) 
						{
							iMinDegree = NodeNeighbours[j]->size();
							Candidates.clear();
						}
						Candidates.push_back(j);
					}
		
			// Randomly select the next node with smallest degree
			iMinDegreePos = Candidates[random_range(0, Candidates.size()-1)];
			Candidates.clear();
		}

		// Disconnect the selected node and connect all its neighbours
		for(SetIter = NodeNeighbours[iMinDegreePos]->begin(); SetIter != NodeNeighbours[iMinDegreePos]->end(); SetIter++) {
			
			// Search for the selected node in the neighbour set of each neighbour
			SetIter2 = NodeNeighbours[(*SetIter)->getLabel()]->find(VarOrder[iMinDegreePos]);
			if(SetIter2 == NodeNeighbours[(*SetIter)->getLabel()]->end())
				writeErrorMsg("Illegal neighbourhood relation.", "Hypergraph::getMIWOrder");
			
			// Remove the selected node from the neighbour set of each neighbour
			NodeNeighbours[(*SetIter)->getLabel()]->erase(*SetIter2);
			
			// Connect all neighbours of the selected node
			for(SetIter2=NodeNeighbours[iMinDegreePos]->begin(); SetIter2 != NodeNeighbours[iMinDegreePos]->end(); SetIter2++)
				if(*SetIter2 != *SetIter)
					NodeNeighbours[(*SetIter)->getLabel()]->insert(*SetIter2);
		}

		// Remove the selected node
		delete NodeNeighbours[iMinDegreePos];
		NodeNeighbours[iMinDegreePos] = NULL;

		// Remember the order of the node removal
		iOrder[iMinDegreePos] = iMyMaxNbrOfNodes-i;
	}

	// Sort variables according to their removal order
	sortPointers((void **)VarOrder, iOrder, 0, iMyMaxNbrOfNodes-1);

	delete [] NodeNeighbours;
	delete [] iOrder;

	return VarOrder;
}

int Hypergraph::getMinimumNodeDegree()
{
	if(this->iMyMaxNbrOfNodes == 0) return -1;

	int min = this->getNode(0)->getNbrOfNeighbours();
	for(int i = 1; i < this->iMyMaxNbrOfNodes; ++i)
	{
		if(min > this->getNode(i)->getNbrOfNeighbours())
			min = this->getNode(i)->getNbrOfNeighbours();
	}
	return min;
}

int Hypergraph::getMaximumNodeDegree()
{
	if(this->iMyMaxNbrOfNodes == 0) return -1;

	int max = this->getNode(0)->getNbrOfNeighbours();
	for(int i = 1; i < this->iMyMaxNbrOfNodes; ++i)
	{
		if(max < this->getNode(i)->getNbrOfNeighbours())
			max = this->getNode(i)->getNbrOfNeighbours();
	}
	return max;
}

double Hypergraph::getAverageNodeDegree()
{
	if(this->iMyMaxNbrOfNodes == 0) return -1;

	int sum = 0;
	for(int i = 0; i < this->iMyMaxNbrOfNodes; ++i)
		sum += this->getNode(i)->getNbrOfNeighbours();
		
	return (float)sum/this->iMyMaxNbrOfNodes;
}

int Hypergraph::getMinimumNodeFill()
{
	if(minFill == -1) setFillStatistics();
	return minFill;
}

int Hypergraph::getMaximumNodeFill()
{
	if(maxFill == -1) setFillStatistics();
	return maxFill;
}

double Hypergraph::getAverageNodeFill()
{
	if(avgFill == -1) setFillStatistics();
	return avgFill;
}

/*
***Description***
The method returns the nodes of the underlying hypergraph in the order determined by the
minimum fill-in (MF) heuristic.

INPUT:
OUTPUT: return: Order of nodes

***History***
Written: (10.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node **Hypergraph::getMFOrder(int preElim)
{
	int *iOrder, iMinFill, iMinFillPos, iTmp, i, j;
	Node **VarOrder;
	set<Node *> **NodeNeighbours, *Neighbours;
	set<Node *>::iterator SetIter, SetIter2;
	vector<int> Candidates;

	iOrder = new int[iMyMaxNbrOfNodes];
	VarOrder = new Node*[iMyMaxNbrOfNodes+1];
	if((iOrder == NULL) || (VarOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");

	// Create storage space for the neighbours of each node
	NodeNeighbours = new set<Node *>*[iMyMaxNbrOfNodes];
	if(NodeNeighbours == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		NodeNeighbours[i] = new set<Node *>;
		if(NodeNeighbours[i] == NULL)
			writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");
	}

	// Initialize variable order and set the set of neighbours of each node
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		VarOrder[i] = MyNodes[i];
		VarOrder[i]->setLabel(i);
		for(j=0; j < VarOrder[i]->getNbrOfNeighbours(); j++)
			NodeNeighbours[i]->insert(VarOrder[i]->getNeighbour(j));
	}
	VarOrder[i] = NULL;

	// Remove nodes with smallest fill-in set iteratively
	for(i=0; i < iMyMaxNbrOfNodes; i++) {

		// Search for the first node that has not been removed yet
		for(j=0; NodeNeighbours[j] == NULL; j++);
		// Compute the cardinality of the  minimum fill-in set
		for(iMinFill = 0, SetIter = NodeNeighbours[j]->begin(); SetIter != NodeNeighbours[j]->end();) {
			Neighbours = NodeNeighbours[(*SetIter)->getLabel()];
			for(SetIter2 = ++SetIter; SetIter2 != NodeNeighbours[j]->end(); SetIter2++)
				if(Neighbours->find(*SetIter2) == Neighbours->end())
					++iMinFill;
		}
		Candidates.push_back(j);

		// Search for the node with smallest minimum fill-in set
		for(++j; j < iMyMaxNbrOfNodes; j++) {
			if(NodeNeighbours[j] != NULL) {
				// Compute the cardinality of the  minimum fill-in set
				for(iTmp = 0, SetIter = NodeNeighbours[j]->begin(); SetIter != NodeNeighbours[j]->end();) {
					Neighbours = NodeNeighbours[(*SetIter)->getLabel()];
					for(SetIter2 = ++SetIter; SetIter2 != NodeNeighbours[j]->end(); SetIter2++)
						if(Neighbours->find(*SetIter2) == Neighbours->end())
							++iTmp;
				}
				if(iTmp <= iMinFill) {
					if(iTmp < iMinFill) {
						iMinFill = iTmp;
						Candidates.clear();
					}
					Candidates.push_back(j);
				}
			}
		}

		// Randomly select the next node with minimum fill-in set
		iMinFillPos = Candidates[random_range(0, Candidates.size()-1)];
		Candidates.clear();

		// Disconnect the selected node and connect all its neighbours
		for(SetIter = NodeNeighbours[iMinFillPos]->begin(); SetIter != NodeNeighbours[iMinFillPos]->end(); SetIter++) {
			
			// Search for the selected node in the neighbour set of each neighbour
			SetIter2 = NodeNeighbours[(*SetIter)->getLabel()]->find(VarOrder[iMinFillPos]);
			if(SetIter2 == NodeNeighbours[(*SetIter)->getLabel()]->end())
				writeErrorMsg("Illegal neighbourhood relation.", "Hypergraph::getMIWOrder");
			
			// Remove the selected node from the neighbour set of each neighbour
			NodeNeighbours[(*SetIter)->getLabel()]->erase(*SetIter2);
			
			// Connect all neighbours of the selected node
			for(SetIter2=NodeNeighbours[iMinFillPos]->begin(); SetIter2 != NodeNeighbours[iMinFillPos]->end(); SetIter2++)
				if(*SetIter2 != *SetIter)
					NodeNeighbours[(*SetIter)->getLabel()]->insert(*SetIter2);
		}

		// Remove the selected node
		delete NodeNeighbours[iMinFillPos];
		NodeNeighbours[iMinFillPos] = NULL;

		// Remember the order of the node removal
		iOrder[iMinFillPos] = iMyMaxNbrOfNodes-i;
	}

	// Sort variables according to their removal order
	sortPointers((void **)VarOrder, iOrder, 0, iMyMaxNbrOfNodes-1);

	delete [] NodeNeighbours;
	delete [] iOrder;

	return VarOrder;
}

void Hypergraph::setFillStatistics()
{
	int iTmp, i, j;
	set<Node *> **NodeNeighbours, *Neighbours;
	set<Node *>::iterator SetIter, SetIter2;

	// Create storage space for the neighbours of each node
	NodeNeighbours = new set<Node *>*[iMyMaxNbrOfNodes];
	if(NodeNeighbours == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		NodeNeighbours[i] = new set<Node *>;
		if(NodeNeighbours[i] == NULL)
			writeErrorMsg("Error assigning memory.", "Hypergraph::getMIWOrder");
	}

	// Initialize variable order and set the set of neighbours of each node
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		MyNodes[i]->setLabel(i);
		for(j=0; j < MyNodes[i]->getNbrOfNeighbours(); j++)
			NodeNeighbours[i]->insert(MyNodes[i]->getNeighbour(j));
	}

	int min = 0, max = 0, sum = 0;

	// Search for the node with smallest minimum fill-in set
	for(j=0; j < iMyMaxNbrOfNodes; j++) {
		// Compute the cardinality of the  minimum fill-in set
		for(iTmp = 0, SetIter = NodeNeighbours[j]->begin(); SetIter != NodeNeighbours[j]->end();) {
			Neighbours = NodeNeighbours[(*SetIter)->getLabel()];
			for(SetIter2 = ++SetIter; SetIter2 != NodeNeighbours[j]->end(); SetIter2++)
				if(Neighbours->find(*SetIter2) == Neighbours->end())
					++iTmp;
		}
		if(j == 0) min = max = iTmp;
		if(min > iTmp) min = iTmp;
		if(max < iTmp) max = iTmp;
		sum += iTmp;
	}

	delete [] NodeNeighbours;

	this->minFill = min;
	this->maxFill = max;
	this->avgFill = (double)sum/this->iMyMaxNbrOfNodes;
}

/*
***Description***
The method returns the nodes of the underlying hypergraph in the order determined by the
maximum cardinality search (MCS) heuristic.

INPUT:
OUTPUT: return: Order of nodes

***History***
Written: (22.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node **Hypergraph::getMCSOrder(int preElim)
{
	int *iOrder, iMaxCard, iTmpCard, iMaxCardPos, iInitialVertex, i, j, k;
	Node **VarOrder, *Var;
	vector<int> Candidates;

	iOrder = new int[iMyMaxNbrOfNodes];
	VarOrder = new Node*[iMyMaxNbrOfNodes+1];
	if((iOrder == NULL) || (VarOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypergraph::getMCSOrder");

	// Initialize variable order
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		VarOrder[i] = MyNodes[i];
		VarOrder[i]->setLabel(0);
	}
	VarOrder[i] = NULL;

	iInitialVertex = rand()%iMyMaxNbrOfNodes;
	iOrder[iInitialVertex] = 0;
	VarOrder[iInitialVertex]->setLabel(1);

	// Remove nodes with highest connectivity iteratively
	for(i=1; i < iMyMaxNbrOfNodes; i++) {

		// Search for the first node that has not been removed yet and count its connectivity
		for(iMaxCard=j=0; MyNodes[j]->getLabel() != 0; j++);
		Var = MyNodes[j];
		for(k=0; k < Var->getNbrOfNeighbours(); k++)
			iMaxCard += Var->getNeighbour(k)->getLabel();
		Candidates.push_back(j);

		// Search for the node with highest connectivity, 
		// i.e., with highest number of neighbours in the set of nodes already removed
		for(++j; j < iMyMaxNbrOfNodes; j++) {
			Var = MyNodes[j];
			if(Var->getLabel() == 0) {
				iTmpCard = 0;
				for(k=0; k < Var->getNbrOfNeighbours(); k++)
					iTmpCard += Var->getNeighbour(k)->getLabel();
				if(iTmpCard >= iMaxCard) {
					if(iTmpCard > iMaxCard) {
						iMaxCard = iTmpCard;
						Candidates.clear();
					}
					Candidates.push_back(j);
				}
			}
		}

		// Randomly select the next node with highest connectivity
		iMaxCardPos = Candidates[random_range(0, Candidates.size()-1)];
		Candidates.clear();

		// Remove the selected node
		VarOrder[iMaxCardPos]->setLabel(1);

		// Remember the order of the node removal
		iOrder[iMaxCardPos] = i;
	}

	// Sort variables according to their removal order
	sortPointers((void **)VarOrder, iOrder, 0, iMyMaxNbrOfNodes-1);

	delete [] iOrder;

	return VarOrder;
}


/*
***Description***
Returns order of nodes based on iterated heuristic algorithm 

INPUT:
OUTPUT: return: Order of nodes

***History***
Written: (06.11.06, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node **Hypergraph::getLocalSearchOrder(int preElim)
{
	// TO CHANGE --- add the appropriate code for iterated heuristic algorithm
	
	int i, k;
	Node **VarOrder;
	vector<int> Candidates;

   int intRandomProb1=100;
   int timeToStop1=2000;
   int timeWithImproveToStop1=1000;
   int iMaxNrNotImproves1=10;
   
   int iAlgoType1=4;
   int intTabuLength1=1;
   int lNumberOfEvaluations1=100000000;
   int iNumberOfIterLocal1=10000000;
   bool bTSDeterministic1=false;
   int iPerturbationType1=2;
   int iInitiPertSize1=3;
   int maxPertSize1=10;
   bool mixPerturbation1=true;
   int iSolAcceptanceILS1=3;
   bool useSwap1=true;
   int initFitnessBound1=3;
  
   probSol objProbSol;
   TabuSearch objTS;
       
	objProbSol.createGraphFromHyp(this); 

    //call the algorithm which will produce the particular order with iterated local search

	objTS.setTSParametres(iAlgoType1,intRandomProb1,intTabuLength1, lNumberOfEvaluations1,
						 timeToStop1,timeWithImproveToStop1,
						 iNumberOfIterLocal1, iMaxNrNotImproves1,bTSDeterministic1,iPerturbationType1,
						 iInitiPertSize1,maxPertSize1,mixPerturbation1,iSolAcceptanceILS1,useSwap1, initFitnessBound1);
					 
	objTS.iteratedLocalSearch(objProbSol); 

    //objProbSol.iElimOrdering 
    VarOrder = new Node*[iMyMaxNbrOfNodes+1];
	if(VarOrder == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getLocalSearchOrder");

	// Initialize variable order
	for(i=0; i < iMyMaxNbrOfNodes; i++) {
		
		k=objProbSol.iNodePosInOrdering[i];
			
		VarOrder[iMyMaxNbrOfNodes-k-1] = MyNodes[i];
		VarOrder[iMyMaxNbrOfNodes-k-1]->setLabel(0);
	}
	VarOrder[i] = NULL;

	return VarOrder;
}




/*
***Description***
The method returns the nodes of the underlying hypergraph in a random order.

INPUT:
OUTPUT: return: Order of nodes

***History***
Written: (20.12.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node **Hypergraph::getRandomOrder()
{
	int i;
	Node **VarOrder;
	vector<int> iOrder;

	VarOrder = new Node*[iMyMaxNbrOfNodes+1];
	if(VarOrder == NULL)
		writeErrorMsg("Error assigning memory.", "Hypergraph::getRandomOrder");

	// Initialize variable order
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		iOrder.push_back(i);

	random_shuffle(iOrder.begin(), iOrder.end());

	// Store randomized variable order
	for(i=0; i < iMyMaxNbrOfNodes; i++)
		VarOrder[i] = MyNodes[iOrder[i]];
	VarOrder[i] = NULL;

	return VarOrder;
}


/*
***Description***
The method labels the edges in the hypergraph consecutively such that the labels can be used
as indices in arrays.

INPUT:
OUTPUT: 

***History***
Written: (10.05.05, TG)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void Hypergraph::labelEdgesConsecutively()
{
	int i;
	for(i=0; i < iMyMaxNbrOfEdges; i++)
	{
		MyEdges[i]->setLabel(i);
	}
}


/*
***Description***
The method labels the nodes in the hypergraph consecutively such that the labels can be used
as indices in arrays.

INPUT:
OUTPUT: 

***History***
Written: (10.05.05, TG)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void Hypergraph::labelNodesConsecutively()
{
	int i;
	for(i=0; i < iMyMaxNbrOfNodes; i++)
	{
		MyNodes[i]->setLabel(i);
	}
}


