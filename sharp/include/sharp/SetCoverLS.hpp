#if (!defined CLS_SETCOVERLS)
#define CLS_SETCOVERLS

namespace sharp
{
	class SetCoverLS
	 {
	private:
		// m*n matrix for set cover problem. 
		//Rows represents variables and columns reperesent sets. If the set i covers the variable j
		// the corrosponding element (i,j) of matrix is 1 otherwise 0. 
		bool *lProblem;

	    // array of variables for each set; with iSetsVarRange is determined the range of varibales for paricular set
		//Ex. for set 3  : iSetsVarRange[6]=10; iSetsVarRange[7]=12 --- means that 
		//variables from iSetsVar[10] to iSetsVar[12] belongs to set 3...
		int *iSetsVar;
		int *iSetsVarRange;

	    // array of sets  for each var; with iVarSetsRange is determined the range of sets 
		//for paricular set 
		int *iVarSets;
		int *iVarSetsRange;

		// Number of variables to cover in the set cover problem
		int iNrOfVar;

		//Number of sets
		int iNrOfSets;  

		// array which indicates for each variable from how many sets is covered.
		// If variable j is covered by n sets the value iCoveredVar[j]=n
		int *iCoveredVar; 

	    //Number of variables covered by a single set
		int iNrVarCovBySingleSet;
	    
	    //Number of covered variables
		int iNrOfCoveredVar;
	    
		// Array which indicates if the particular set is selected to cover variables or not
		// For a particular set j, if set j is selected then iSelectedSets[j]=1, otherwise 0 
		int *iSelectedSets;
	    
	    // number of selected sets
		int iNumberOfSelectedSets;

		// Array which indicates the number of uncovered variables contained in each set
		int *iUVarInSets;
	    
		 // number of variables in each set
		 int *iVarInSets;

	    // tabu list which is organised as a first input, first output list..
		int *lTabuList;
	     
	    // Frequency based memory...
		int *lFrequencyBasedMemory;
	    
	    // function which returns 1 if the solution is tabu, otherwise 0
		int isSolutionTabu(int iSet);
	    
	    // updates tabu list 
		void updateTabuList(int iSet);
	       
		//Fitness of solution
		double dFitness;
	    
	    //generates the arrays which contains information for sets covering each variable, 
		//and the variables wach set covers 
		void setVarSets();

		//Calculates fitness of current solution   
		void calculateFitness();

		//Calculates fitness of current solution based on remove or add of a new set    
		double calculateFitnessFromMove(int iSet, int iOption);

	    //Calculates fitness of current solution based on remove or add of a new set    
		double calculateFitnessFromSwapMove(int iSet1, int iSet2);


		//removes od adds a set iSet from the selected sets for set cover
	    void DoMove(int iSet, int iOption);

		//removes iSet1 and adds iSet2 from the selected sets for set cover
		void DoSwapMove(int iSet1, int iSet2);

		// Number of iterations with no improve to stop the local search procedure...
		int iMaxNrNotImproves;
		
	    // Number of evaluations to stop the procedure...
		long lNumberOfEvaluations;
		
		// variables which takes values from 0-100 and determines the probability that a random _
		// move will be executed during a particular iteration. 
	    int intRandomProb; 
		
	    // length of tabu list...
		int intTabuLength; 

	    // indicates if tabu search procedure is deterministic or not
	    bool bTSDeterministic;
		
		// determines if in tabu search intial solution should be greedy or not
		bool bInitialSolGreedy;

		// greedy algorithm for set cover
		void greedySetCover(int iGreedyType=1);

		// calls local search procedure to improve the current solution for set cover. 
		int localSearchSC2();

		//produces solution with empty sets
		void emptySetCover();

	    //changes the penalties for the number of sets and for the number of uncovered variables
	    void changePenalties();

		//tests if the solution is legal (all variables covered) and gives the number of selected aets
	    void iCheckSolution();

	    
		//Followign 3 functions are used just for EXPERIMENTS AND ARE NOT COMPELETELY TESTED...  
		
		// greedy algorithm for set cover
		void greedySetCover1(int iGreedyType=1);

		// calls local search procedure to improve the current solution for set cover. 
		void localSearchSC();

		// calls local search procedure to improve the current solution for set cover. 
		int localSearchSC1();


	 public:
		SetCoverLS();
		virtual ~SetCoverLS();

		
		int getNrOfSelectedSets(){return iNumberOfSelectedSets;}

		int getNrOfUncovVar(){return iNrOfVar-iNrOfCoveredVar;}
		
		//returns 1 if the set iSet is selected for cover otherwise 0
	    int isSetSelected(int iSet) {return iSelectedSets[iSet];}
	 
	    // Calls other procedures for calculating of miniml set cover
	    int minimalSC(bool* setCovMatrix, int iNumberOfSets, int iNumberOfVariables, int iSearchOption=1);

		// sets relevant parametres for Tabu Search
	    void setTSParametres(long lMaxNrOfEvaluations, int iNrOfMaxIterationsNoImprovment, int iProbForRandomMove, int iLengthOfTabuList,bool bIsTSDeterministic, bool bGenerateGreedyInitialSol);


	};
}
#endif // !defined CLS_SETCOVERLS)



	
