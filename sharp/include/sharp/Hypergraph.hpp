/*
***Description***
Class Hypergraph  models a hypergraph consisting of its edges and nodes.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_HYPERGRAPH)
#define CLS_HYPERGRAPH

#include <vector>
using namespace std;

namespace sharp
{
	class Hyperedge;
	class Node;
	class Hypertree;

	class Problem;

	class Hypergraph  
	{
		friend class Problem;

	protected:
		// Maximum number of edges contained in the hypergraph
		int iMyMaxNbrOfEdges;

		// Maximum number of nodes contained in the hypergraph
		int iMyMaxNbrOfNodes;

		// Array of pointers to the edges contained in the hypergraph
		vector<Hyperedge *> MyEdges;

		// Array of pointers to the nodes contained in the hypergraph
		vector<Node *> MyNodes;

		// Labels all edges reachable from Edge
		void labelReachEdges(Hyperedge *Edge);

		void setFillStatistics();

		int minFill;
		int maxFill;
		double avgFill;
		bool isgraph;

	public:
		// Constructor
		Hypergraph(bool isGraph = false);

		// Destructor
		virtual~Hypergraph();

		// Returns the number of edges in the hypergraph
		int getNbrOfEdges();

		// Returns the number of nodes in the hypergraph
		int getNbrOfNodes();

		// Returns the hyperedge stored at position iPos
		Hyperedge *getEdge(int iPos);

		// Returns the node stored at position iPos
		Node *getNode(int iPos);

		// Returns the hyperedge with ID iID
		Hyperedge *getEdgeByID(int iID);

		// Returns the node with ID iID
		Node *getNodeByID(int iID);

		// Sets labels of all edges to zero
		void resetEdgeLabels(int iVal = 0);

		// Sets labels of all nodes to zero
		void resetNodeLabels(int iVal = 0);

		// Checks whether the hypergraph is connected
		bool isConnected();

		// Returns if this graph is a true hypergraph or a normal graph
		bool isGraph() const;

		// Transforms the hypergraph into its dual hypergraph
		void makeDual();

		// Removes hyperedges that are covered by another hyperedge
		void reduce(bool bFinalOnly = false);

		// Updates the neighbourhood relation of hyperedges and nodes
		void updateNeighbourhood();

		// Updates the sizes of hyperedges and nodes
		void updateCompSizes();

		// Divides a hypergraph into subgraphs according to a node-partitioning
		void divideNodes(int *iPartitioning, Hypergraph ***Subgraphs, Hyperedge ***Separator);

		// Returns nodes in the same order as they are stored in the hypergraph
		Node **getInputOrder(int preElim = 0);

		// Returns nodes in the order determined by the minimum induced width (MIW) heuristic
		Node **getMIWOrder(int preElim = 0);

		// Returns nodes in the order determined by the minimum fill-in (MF) heuristic
		Node **getMFOrder(int preElim = 0);

		// Returns nodes in the order determined by the maximum cardinality search (MCS) heuristic
		Node **getMCSOrder(int preElim = 0);

		//Returns nodes in the order determined by iterative local search algorithm
		Node **getLocalSearchOrder(int preElim = 0);

		// Returns nodes in a random order
		Node **getRandomOrder();

	    // sets in the hypergraph the order of the Nodes based on MCS order
		void setMCSOrder();

	     // sets in the hypergraph the order of the Nodes based on Min-fill heuristic
		void setMFOrder();

		
		// label all edges consecutively
		void labelEdgesConsecutively();
		
		// label all nodes consecutively
		void labelNodesConsecutively();	

		double getAverageNodeDegree();
		int getMaximumNodeDegree();
		int getMinimumNodeDegree();

		double getAverageNodeFill();
		int getMaximumNodeFill();
		int getMinimumNodeFill();	
	    
		//checks if the set of nodes (given by their ID) are contained in any edge of hypergraph
	    int areNodesContainedInSomeEdge(int *iNodes, int iNrOfNodes);
	 
		// removes nodes contained only in one hyperedge
	    void removeNodesContainedInOneEdge();

	    //removes from the hypergraph edge with ID equal to iID 
	    void removeEdge(int iID);

	    // Checks if the niegborhood of node baseNode (including this node) is covered by 
		// k edges. These k edges should contain the baseNode
	    bool isNieghborhoodCoveredByKEdges(Node* baseNode, int iK);
	    
		bool areNodesCoveredByKEdges(int *iNodes, int iNrOfNodes, int iK);

		// Applies Graham-reduction to hypergraph
	    void GrahamReduction();
		
		//overloading operator =  (copies the content of input hypergraph into current hypergraph)
		void operator = (Hypergraph& param);

	    // inserts new hyperedge which contains the input nodes in the hypergraph 
	    void insertHyperedge(Node **edgeNodes, int iEdgeNodesSize, int edgeID, char* edgeName="Cx");

	    // inserts new hyperedge which contains nodes in the hypergraph which have ID as in iNodesID  
	    void insertHyperedge(int *iNodesID, int iNodesIDSize, int edgeID, char* edgeName="Cx");

	};
}

#endif // !defined(CLS_HYPERGRAPH)

