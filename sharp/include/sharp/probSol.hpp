//Implements solution for the ordering of nodes from which the tree/hypertree decomposition can be used 
// In this class also information for Input Graph are stored
#ifndef CLS_PROBSOL
#define CLS_PROBSOL

namespace sharp
{
	class Hypergraph;
	class Node;

	class probSol  
	{
	 private:
	    
		//0, 1 matrix which represents the Graph 
	    bool** iZeroOneGraph; 


	    // indicates for each node if it is already eliminated or not during the elimination process 
		bool *bNodeEliminated;

		Hypergraph *Hyp;


	 //just used for fitness function ...
		unsigned short int *iTempNodes; 
		int* iEdgeSelected;

	 
	 
	 public:

		//Number of neighboors for each node
		unsigned short int* iNrOfNeighboors;
	   
		//number of Edges each node has ...
	    unsigned short int* iNrOfNodeEdges;

		 //Input Graph
		//n*m matrix, n represent number of nodes. m number of edges
		unsigned short int** iNodesEdges;

		
		//Nodes Input Graph
		//n*n matrix, n represent number of nodes. For each node in that row are listed neighborhood nodes
		unsigned short int** iGraph;


		bool bHypertreeAlg;     
	   
	    //Number of neighboors for each node
		unsigned short int* iTempNrOfNeighboors;


	    //position of each node in ordering ...
	    unsigned short int *iNodePosInOrdering;

	    //Nodes that have the max clique ...
	    unsigned short int *iMaxCliqueNodes;

	    
		// Number of nodes
		int iNrOfNodes;
		//number of edges 
		int iNrOfEdges;

		//size of clique for each node ...
		unsigned short int *iCliqueSizes;

	    // Elimination ordering 
		unsigned short int *iElimOrdering;

		// number of iterations for which the solution was found...
	    long lNumberOfEvaluations;
	    
		float fFitness;
		int iLargestClique;
		int iLargestCliqueNode;
		
		int iNrNodesLargestClique;

	    probSol();
		  
		~probSol();

		void randomIntialSolution();	
	    void greedyIntialSolution();	
	  
		void MinFillOrdering();
		void MCSOrdering();
	    void MinDegreeOrdering();
	    void MinFillAndDegreeOrdering();
	    void MinDegreeAndFillOrdering();


		void NewMCSOrder();
		void NewMinDegreeOrder();  
		void NewMinFillOrder();				 
		
		//returns true if iNode1 and iNode2 are connected, otherwise false
		bool bAreNodesConnected(int iNode1, int iNode2);
		
		
		
		// iLowestPosition=-1, int iHighPosition=-1 are taken to know to which positions are moved nodes 
		// The cliques change only between these two positions and we can calculate more efficiently the fitness
		// By default they are -1, that means in the fitness fuinction do not take them into consideration
		void calculateFitness(int iLowestPosition=-1, int iHighPosition=-1);	

	    void calculateFitnessTest(int iLowestPosition=-1, int iHighPosition=-1);	


	    //Calculates fitness of current solution based on remove or add of a new set    
		double calculateFitnessFromMove(int iNode, int iPosition);

	      //Calculates fitness of current solution based on remove or add of a new set    
		double calculateFitnessFromSwap(int iNode, int iPosition);

		
	    //changes position of move ...
	    void DoMove(int iNode, int iNewPosition);
	    
	    //swaps the iNode with the node located in iPosition in elimination ordering
	    void Swap(int iNode, int iPosition);

	    //swaps the iNode with the node located in iPosition in elimination ordering
	    void doubleSwap(int iNode, int iPosition, int iNode2, int iPosition2);


	    //finds the position of the nearest nodes of node iNode (nodes from neighboorhod) in the ordering _
		// predcessor - node before iNode in ordering and succesor - the node after iNode in the ordering...
	    void nearestNodes(int iNode, int* iPrePos, int* iSucPos);

	    //sets new ordering in the solution...
		void setNewOrdering(unsigned short int* iNewOrdering);

		void readFromFile(char *strnameoffile);
		
		void createGraphFromHyp(Hypergraph *myHypergraph);

		void createGraphFromHypNew(Hypergraph *myHypergraph);


	    void performNRandMoves(int randomMoveNr); 
	    
	    void performNRandSwaps(int randomMoveNr); 

	    
		void destroyPart(int randomMoveNr);

		//performs random moves of all nodes with the largest clique
		void moveLargestCliques(int NumberOfMoves);

		void generatePosInOrdering()
		{int i; for(i=0;i<iNrOfNodes;i++) iNodePosInOrdering[iElimOrdering[i]]=i;}
	 

	// overloading operators = and ==
	//	void operator = (clsSolution&);

	//	int operator == (clsSolution&);

	};
}

#endif
