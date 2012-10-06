#if (!defined CLS_TABUSEARCH)
#define CLS_TABUSEARCH

namespace sharp
{
	class probSol;

	class TabuSearch
	 {
	private:
		

	    // tabu list which is organised as a first input, first output list..
		int *iTabuList;
		int *iTabuListPos;
	     
	    // Frequency based memory...
		int *lFrequencyBasedMemory;
	    
	    // function which returns 1 if the solution is tabu, otherwise 0
		int isSolutionTabu(int iNode, int pos =-1);

		int isSwapTabu(int iNode1, int iNode2);
	    
	    // updates tabu list 
		void updateTabuList(int iNode, int pos=-1);

	    void resetTabuList();


		//tests is a node is tabu for swapping
		int isNodeTabu(int iNode1, int iIteration);

		//NM: updates tabu list: marks the iteration in whcih the node was swapped
		void updateNodeTabuList(int iNode, int iIteration);

		// Number of iterations with no improve to stop the local search procedure...
		int iMaxNrNotImproves;
		
	    // Number of evaluations to stop the procedure...
		long lNumberOfEvaluations;

		//time in seconds to stop the algorithm
		int timeToStop;
	    //time after no improvment to stop the algorithm...
		int timeWithImproveToStop;


	    // Number of iterations in the iterated local search...
		long iNumberOfIterLocal;

		
		// variables which takes values from 0-100 and determines the probability that a random _
		// move will be executed during a particular iteration. 
	    int intRandomProb; 
		
	    // length of tabu list...
		int intTabuLength; 

	    // indicates if tabu search procedure is deterministic or not
	    bool bTSDeterministic;


		//indicates the type of algorithm to be used in iterated local search
		int iAlgoType;

		//indicates type of perturbation used
		int iPerturbationType;

		//indicates size of perturbation
		int iPerturbationSize;


		//Indicates type of acceptance criteria in iterated local search
		// 1-- each iteration begins with the perturbated best solution from previous iteration
		// 2 -- each iteration begins with the perturbated best yet existing solution
		int iSolAcceptanceILS;


		bool useSwap;

	     /// just used in all procedures ...
		unsigned short int* bestSetSolution;


	 public:
		 TabuSearch();
		 virtual ~TabuSearch();

	    // Calls other procedures for calculating of minimal tree width
	    int tabuSearchAlg(probSol& objProbSol, bool initSolGiven=false);
	    
	    // Calls other procedures for calculating of minimal tree width
	    int fiduciaSearchAlg(probSol& objProbSol, bool initSolGiven=false);

	    // Calls other procedures for calculating of minimal tree width
	    int iteratedSearchAlg(probSol& objProbSol, bool initSolGiven=false);

	    // Calls other procedures for calculating of minimal tree width
	    int iteratedLocalSearch(probSol& objProbSol);

	    
		// Calls other procedures for calculating of minimal tree width
	    int minConflicts(probSol& objProbSol, bool initSolGiven=false);

		// Calls other procedures for calculating of minimal tree width
	    int minConflictsTS(probSol& objProbSol, bool initSolGiven=false);

		// Simulated Annelaing procedure
	    int SA(probSol& objProbSol, bool initSolGiven=false);

		//great de luge algotithms ...
		int GreatDeLuge(probSol& objProbSol, bool initSolGiven=false);

		
		// Calls other procedures for calculating of minimal tree width
	    int randomWalk(probSol& objProbSol, bool initSolGiven=false);


		// sets relevant parametres for Tabu Search
		void setTSParametres(int iAlgoType1=3,int intRandomProb1=50, int intTabuLength1=7, 
			int lNumberOfEvaluations1=100000000,int timeToStop1=100000,int timeWithImproveToStop1=10, 
			int iNumberOfIterLocal1=10000, int iMaxNrNotImproves1=100,bool bTSDeterministic1=false,int iPerturbationType1=1, 
			int iInitiPertSize1=2,int maxPertSize1=8, 
			bool mixPerturbation1=true,int iSolAcceptanceILS1=3,bool useSwap1=true, int initFitnessBound1=3);

	};
}
#endif // !defined CLS_TABUSEARCH)



	
