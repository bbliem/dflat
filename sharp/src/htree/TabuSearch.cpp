#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <math.h>

using namespace std;

#include <probSol.hpp>
#include <TabuSearch.hpp>

using namespace sharp;

    //Constants...

	const long 	lLargestFitness = 1000000; 

	//constants for the type of search
	const int TS=1;
	const int MC=2;
	const int MCTS=3;
	const int SimAnn=4;
	const int GDeLuge=5;
	const int RW=6;
	

	//Simulated Annealing Parameters:
	
	int iMaxIterPerTemsperature=10;
    
	float fInitTemperature=5;
	float fMinTemperature=0.01;
	float fTempDecrease=0.99;
	
	int iMaxRestarts=5;
	int iMaxInsideNoImprove=10;
	int iRandomNoiseProb=10;
   	
	//Great Deluge/Record-to-Record travel parameters ...
    int Level;
	int UP=1;
	int DEV=2;
	int iGDRandomNoiseProb=20;
   	int iGDMaxRestarts=1;
	int iGDMaxInsideNoImprove=5;


	const int acceptBestSol=1;
	const int acceptIterSol=2;
    const int acceptSolWithBound=3;

	//constants for type of perturbation
	const int randPerturbation=1;
	const int largestCperturbation=2;
	const int partPerturbation=3;
	bool mixPerturbation=true;
	
	int iInitiPertSize=5;
	int maxPertSize=8;
    
	int initFitnessBound=3;
    int acceptedFitnessBound=initFitnessBound;


//////////////////////////////////////////////////////////////////////////////////////
void TabuSearch::setTSParametres(int iAlgoType1,int intRandomProb1, int intTabuLength1, 
		int lNumberOfEvaluations1, int timeToStop1, int timeWithImproveToStop1,
		int iNumberOfIterLocal1, int iMaxNrNotImproves1,bool bTSDeterministic1,int iPerturbationType1, 
		int iInitiPertSize1,int maxPertSize1,
		bool mixPerturbation1,int iSolAcceptanceILS1,bool useSwap1, int initFitnessBound1)
	{
		iAlgoType=iAlgoType1;
		intRandomProb=intRandomProb1;
		lNumberOfEvaluations=lNumberOfEvaluations1;
		timeToStop=timeToStop1;
		timeWithImproveToStop=timeWithImproveToStop1;
		iNumberOfIterLocal=iNumberOfIterLocal1;
		iMaxNrNotImproves=iMaxNrNotImproves1;
		intTabuLength=intTabuLength1;
		bTSDeterministic=bTSDeterministic1;

		
		this->iPerturbationType =iPerturbationType1;
		this->iPerturbationSize=iInitiPertSize1;
		iInitiPertSize=iInitiPertSize1;
		maxPertSize=maxPertSize1;
		
 		mixPerturbation=mixPerturbation1;
	
	    iSolAcceptanceILS=iSolAcceptanceILS1;

		useSwap=useSwap1;
		initFitnessBound=initFitnessBound1;

	}


TabuSearch::~TabuSearch()
{
	delete [] iTabuList;
	delete [] iTabuListPos;
    delete [] lFrequencyBasedMemory;	
	delete [] bestSetSolution;
}


TabuSearch::TabuSearch()
{
	iTabuList=NULL;
	iTabuListPos=NULL;
	lFrequencyBasedMemory=NULL;
	bestSetSolution=NULL;

	intRandomProb=50;
	lNumberOfEvaluations=1000000000;
	timeToStop=10000;
	timeWithImproveToStop=10000;
	iNumberOfIterLocal=1000000;
    iMaxNrNotImproves=10;
	intTabuLength=8;
    bTSDeterministic=false;

	iAlgoType=MCTS;
	this->iPerturbationSize=iInitiPertSize;
	this->iPerturbationType =randPerturbation;
    //	this->iPerturbationType =partPerturbation;
    //this->iPerturbationType =largestCperturbation;
	mixPerturbation=true;
	
    //iSolAcceptanceILS=acceptBestSol;
    // iSolAcceptanceILS=acceptIterSol;
	iSolAcceptanceILS=acceptSolWithBound;

	useSwap=true;
}



int TabuSearch::iteratedLocalSearch(probSol& objProbSol)
{
	time_t start, end, starNotImprove;
    int bestSolTime;
	int i;

	float fFitnessOfBestSol;
 
	unsigned short int* bestSolution;
    int iNotImproves;
	int repetitionOfSol=0;
   
    iTabuList = new int[objProbSol.iNrOfNodes];
 
	bestSetSolution =new unsigned short int[objProbSol.iNrOfNodes];
	
    // objProbSol.bHypertreeAlg =true;

	bestSolution=new unsigned short int[objProbSol.iNrOfNodes];
 
    acceptedFitnessBound=initFitnessBound;
 


//	  objProbSol.MinFillOrdering();
//	  objProbSol.MinDegreeOrdering();
	  objProbSol.MCSOrdering();


	  //objProbSol.bHypertreeAlg =true;
//	  objProbSol.calculateFitness(); 
//      fFitnessOfBestSol=objProbSol.fFitness;
	  //objProbSol.bHypertreeAlg =false; 

//	  cout<<endl<<endl<<"Initial Fitness...:"<<endl<<objProbSol.fFitness<<endl ;	

	 return 0;
	
	// generate solution, by calling one of local search procedures 
    switch (iAlgoType)
	{
		case TS : 
				 tabuSearchAlg(objProbSol);
				 break;
		case MC : 
				this->minConflicts(objProbSol); 		  			
				  break;
		case MCTS :
				this->minConflictsTS(objProbSol);
			    break;
		case SimAnn:
				this->SA(objProbSol);
			    break;
		case GDeLuge:
				this->GreatDeLuge(objProbSol);
			    break;
		case RW :
				this->randomWalk(objProbSol);
			    break;
	}

 
      for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSolution[i]=objProbSol.iElimOrdering[i]; 

   	 
	iNotImproves=0;
	repetitionOfSol=0;
	//repeat until 10 iterations of not improves are arrived
	while (iNotImproves<iNumberOfIterLocal)
	{
	
						
    //    	cout<<endl<<endl<<"ILS...:"<<endl<<endl;
	

	    time(&end);
        if (timeToStop<(int)(difftime(end, start))) break;

		if (timeWithImproveToStop<(int)(difftime(end, starNotImprove))) break; 
        
	
         
   //		if ((int)(difftime(end, starNotImprove))>10) objProbSol.bHypertreeAlg =true; 


		/*
        cout<<endl<<repetitionOfSol<<"/"<<iNotImproves<<"  Fitness...:"
			<< objProbSol.fFitness<<"/"<<fFitnessOfBestSol<< "     pert:  "
			<<	iPerturbationSize<<"    Bound: "<<acceptedFitnessBound
			<<"     "<<nrIR<<"  "<<nrILQ<<"  "<<iPerturbationType;
		*/

        if (this->iPerturbationType==randPerturbation)  
		    objProbSol.performNRandSwaps(this->iPerturbationSize);
		else if (this->iPerturbationType==largestCperturbation)  
			 //objProbSol.moveLargestCliques(objProbSol.iNrOfNodes);
		     objProbSol.moveLargestCliques(iPerturbationSize);
		else if (this->iPerturbationType==partPerturbation)  
			 objProbSol.destroyPart(this->iPerturbationSize);


	if (mixPerturbation==true)
	{

	/*
		if (iPerturbationType ==largestCperturbation)
				iPerturbationType =randPerturbation;
		else
			iPerturbationType =largestCperturbation;
      */
	
		
		if (iNotImproves%50==0 && iNotImproves>0 && iPerturbationType ==largestCperturbation)
				iPerturbationType =randPerturbation;
        else if (iNotImproves%50==0 && iNotImproves>0 && iPerturbationType ==randPerturbation)
					this->iPerturbationType =largestCperturbation;
       
	}

    /*
	if (iNotImproves%100==0 && iNotImproves>0 ) 
		{
			//	if (iPerturbationSize<=maxPertSize) this->iPerturbationSize++;
			iPerturbationSize=maxPertSize;
        
		}
        
    */

        //dynamic change of perturbation...
		if ((float)((float)repetitionOfSol/(float)iNotImproves) > 0.2)
		{	if (iPerturbationSize<maxPertSize) 
				iPerturbationSize++;
		}
        else
		{ if (iPerturbationSize>iInitiPertSize) 
			   iPerturbationSize--;
		}

       // acceptedFitnessBound=iPerturbationSize;
		

        /*

        if (this->iPerturbationSize>=maxPertSize)
		{
			this->iPerturbationSize=iInitiPertSize;
		//	acceptedFitnessBound=initFitnessBound;
		}
        */

		// treeToHyp
		if (difftime(end, starNotImprove)>3600 && objProbSol.bHypertreeAlg ==false) 
		{
			//objProbSol.bHypertreeAlg =true; 
			//sets new ordering and it calulates the fitness ...
			 objProbSol.setNewOrdering(bestSolution);

			fFitnessOfBestSol=objProbSol.fFitness; 
			cout<<endl<<endl<<"CHANGE TO HYPERTREE **********************************"<<endl<<objProbSol.fFitness<<endl;
		}

		
	
		switch (iAlgoType)
			{
				case TS : 
					 tabuSearchAlg(objProbSol,true);
					 break;
				case MC : 
					this->minConflicts(objProbSol,true); 		  			
    				break;
				case MCTS :
				    this->minConflictsTS(objProbSol,true); 	
					break;
				case SimAnn:
					this->SA(objProbSol, true);
					break;
				case GDeLuge:
					this->GreatDeLuge(objProbSol, false);
				    break;
				case RW :
						this->randomWalk(objProbSol);
						break;
   			}


		//decide if to change the solution or not ...
       if (objProbSol.fFitness<= fFitnessOfBestSol)
	   {
		    
		   if (objProbSol.bHypertreeAlg ==false)
			  { //objProbSol.bHypertreeAlg =true;
			    objProbSol.calculateFitness();
				//objProbSol.bHypertreeAlg =false;
			  }
		
   
		   if (objProbSol.fFitness < fFitnessOfBestSol) 
		   {
 		 
			   time(&starNotImprove);
			   iNotImproves=0;
			   repetitionOfSol=0;

			   time(&end);
               bestSolTime=(int)(difftime(end, start));

	    	   cout<<endl<<endl<<"Fitness Improve...:"<<endl<< objProbSol.fFitness<<"   Time " <<bestSolTime<<endl;
				

			   fFitnessOfBestSol=objProbSol.fFitness;
			   for(i=0;i<objProbSol.iNrOfNodes;i++) 
				bestSolution[i]=objProbSol.iElimOrdering[i];
	
		   }
		   else if (objProbSol.fFitness == fFitnessOfBestSol)   
		   {
			   iNotImproves++;
			   repetitionOfSol++;
		   }
		   
	   }
	   else if (objProbSol.fFitness> fFitnessOfBestSol)
	   {
		   iNotImproves++;
	       // for this type of acceptance of solution the solution with best current 
		   //fitness is perturbated for next iteration
		   if (iSolAcceptanceILS==acceptBestSol)
	        objProbSol.setNewOrdering(bestSolution);
           else if (iSolAcceptanceILS == acceptSolWithBound)  
		   {
			   if (objProbSol.fFitness>= fFitnessOfBestSol+acceptedFitnessBound)
			   {
				   objProbSol.setNewOrdering(bestSolution);
				//    repetitionOfSol++;
			   }
            

		   }

       }





	}


   objProbSol.setNewOrdering(bestSolution);
  
   cout<<endl<<endl<<"TREE WIDTH:  "<<objProbSol.fFitness<<endl<<endl;
 
   //objProbSol.bHypertreeAlg =true;
   objProbSol.calculateFitness(); 
   cout<<endl<<endl<<"HYPERTREE WIDTH:  "<<objProbSol.fFitness<<endl<<endl;
   //objProbSol.bHypertreeAlg =false;
   
   

   //  time(&end);
  //  bestSolTime=(int)(difftime(end, start));

   delete [] iTabuList;
   iTabuList=NULL;
   
   delete [] bestSolution;
   return bestSolTime;

}



//tabu search procedure ...
int TabuSearch::tabuSearchAlg(probSol& objProbSol, bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;

	int i,j,k;
	int intRandom;
	int iTempNode=0;
	int iTempPos=0;

	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
	double dBestIterationFitness;
  
	double dTempFitness;
	bool bSolutionImproved;

    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
    int sucPosition, prePosition;
    int	iNrSolEqual=0;

    time(&start);

    dOldBestIterationFitness=lLargestFitness+1;
	
 
	if (initSolGiven==false)
	{
       objProbSol.randomIntialSolution(); 
    	objProbSol.calculateFitness(); 

	}

   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
   
   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   iTabuList = new int[intTabuLength];
   iTabuListPos = new int[intTabuLength];

   


    
 //  if (iTabuList == NULL)
//			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

  // lFrequencyBasedMemory=new int[objProbSol.iNrOfNodes];
 //  if (lFrequencyBasedMemory == NULL)
//			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
  // for(i=0;i<objProbSol.iNrOfNodes;i++) 
   //lFrequencyBasedMemory[i]=0;
   

    iIteratWithNoImprove=0;

while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
		    iIterationNumber++;

			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=(rand() % 100);

			//cout<<"rand nr..."<<intRandom<<endl;
		if (intRandom< intRandomProb)
			{  	
  						//Select randomly the set to be removed or added
    			        iTempNode=objProbSol.iLargestCliqueNode; 
					   	iTempPos=(rand() % (objProbSol.iNrOfNodes));
		
			}
		else 
		{
			for(i=0; i< objProbSol.iNrOfNodes; i++)   
			{
				objProbSol.nearestNodes(i,&prePosition,&sucPosition); 

				if (objProbSol.iCliqueSizes[i]>objProbSol.iLargestClique-objProbSol.iLargestClique)
				{
				//for(j=0;j<objProbSol.iNrOfNodes;j++)
				
				for(k=0;k<2;k++)
				{
					if (k==0) j=prePosition;
					else j=sucPosition;

						dTempFitness=lLargestFitness+1; 	

						dTempFitness=objProbSol.calculateFitnessFromMove(i,j);
				
		//				cout<<endl<<"Fitness...:   "<<dTempFitness;
		
						
						lNumberOfEval++;
						if (isSolutionTabu(i)==0)   
						{
							if (dBestIterationFitness>dTempFitness)
							{
								dBestIterationFitness=dTempFitness;
								iTempNode=i;
								iTempPos=j;
								iNrSolEqual=1;

							
							}

							else if (dBestIterationFitness==dTempFitness )
							{	
							   /*
								// use of long term memory considering movement of each set...
								// diversification...
								else if (iIteratWithNoImprove>10)
								{
									if (lFrequencyBasedMemory[i]<lFrequencyBasedMemory[iTempNode])
									{	iTempNode=i;
										 

									}
								}

								 */
								if (bTSDeterministic==false)
								{
							         	iNrSolEqual++;
										//To break ties randomly in case the solutions have the same fitness
										intRandom=(rand() % iNrSolEqual )+1;
										if (intRandom==iNrSolEqual)  
										{
											iTempNode=i;
											iTempPos=j;
										
										}
										
								}

							}
						}

						else if (isSolutionTabu(i)==1)   
						{
							//aspiration criteria...
							if (dTempFitness<fFitnessOfBestSol)
							{
								dBestIterationFitness=dTempFitness;
						
								iTempNode=i;
								iTempPos=j;
								iNrSolEqual=1;
							
						  }

						}
				}
				}		
			}
	
		
			// if no better solution in neighborhood exists...
			if(dBestIterationFitness>= objProbSol.fFitness)
			{		iTempNode=objProbSol.iLargestCliqueNode; 
			
					//cout<<".................";
					iTempPos=(rand() % (objProbSol.iNrOfNodes));
			}
			
		
		
		}
	
		
		    //set new fitness 
		    if (iTempNode>=0)
			{	
			   // register sets which 
			   objProbSol.DoMove(iTempNode,iTempPos);
			   	
			   lNumberOfEval++;
			}
     
			
			if (fFitnessOfBestSol>objProbSol.fFitness)
			{  

				for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
				fFitnessOfBestSol=objProbSol.fFitness;

				time(&end);
		      	bestSolTime=(int)(difftime(end, start));
				
				bSolutionImproved=true;

//			cout<<endl<<endl<<objProbSol.iLargestClique <<"   ********************   "<<endl<<endl; 
        
        
			}
        
			updateTabuList(iTempNode);
			
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}
        
    	    if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;
        
}       
        
        objProbSol.setNewOrdering(bestSetSolution);
		
        
	
		delete [] iTabuList;
		delete [] iTabuListPos;

		iTabuList=NULL;
		iTabuListPos=NULL;
        
        return bestSolTime;

}



//min conflicts procedure ...
int TabuSearch::randomWalk(probSol& objProbSol, bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;

	int i;
	int iTempNode;
	int iTempPos;

	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
	double dBestIterationFitness;
  
	double dTempFitness;
	bool bSolutionImproved;

    double dOldBestIterationFitness;

    time(&start);
	

    dOldBestIterationFitness=lLargestFitness+1;
	
 

    if (initSolGiven==false)
	{
       objProbSol.randomIntialSolution(); 
    	objProbSol.calculateFitness(); 

	}

	
   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
   
   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   iTabuList = new int[intTabuLength];
   iTabuListPos=new int [intTabuLength];
    
   //  if (iTabuList == NULL)
   //			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

  // lFrequencyBasedMemory=new int[objProbSol.iNrOfNodes];
   //  if (lFrequencyBasedMemory == NULL)
   //			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
  // for(i=0;i<objProbSol.iNrOfNodes;i++) 
   //lFrequencyBasedMemory[i]=0;
   

    iIteratWithNoImprove=0;

while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
		    iIterationNumber++;

			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
				
				for(i=0;i<objProbSol.iNrOfNodes;i++)
				{
				       if(objProbSol.iCliqueSizes[i]==objProbSol.iLargestClique)
					   {
					    
						 iTempNode=objProbSol.iElimOrdering[i];
						 iTempPos=(rand() % (objProbSol.iNrOfNodes));

  						 dTempFitness=objProbSol.calculateFitnessFromMove(iTempNode,iTempPos);

						 if (dTempFitness-fFitnessOfBestSol<0){
							objProbSol.DoMove(iTempNode,iTempPos);
							lNumberOfEval++;
							fFitnessOfBestSol=dTempFitness;
							bSolutionImproved=true;

						 }
					   }
				}			
		

				for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
				fFitnessOfBestSol=objProbSol.fFitness;

				time(&end);
		      	bestSolTime=(int)(difftime(end, start));


	
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}
}
		
        objProbSol.setNewOrdering(bestSetSolution);


		delete [] iTabuList;
		delete [] iTabuListPos;
		iTabuListPos=NULL;
		iTabuList=NULL;
 
        return bestSolTime;

}













//min conflicts procedure ...
int TabuSearch::minConflicts(probSol& objProbSol, bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;

	int i,j,k;
	int intRandom;
	int iTempNode=0;
	int iTempPos=0;

	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
	double dBestIterationFitness;
  
	double dTempFitness;
	bool bSolutionImproved;

    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
	int iNrSolEqual=0;

    time(&start);
	

    dOldBestIterationFitness=lLargestFitness+1;
	
 

    if (initSolGiven==false)
	{
       objProbSol.randomIntialSolution(); 
    	objProbSol.calculateFitness(); 

	}

	
   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
   
   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   iTabuList = new int[intTabuLength];
   iTabuListPos= new int[intTabuLength];
    
   //  if (iTabuList == NULL)
   //			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

  // lFrequencyBasedMemory=new int[objProbSol.iNrOfNodes];
   //  if (lFrequencyBasedMemory == NULL)
   //			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
  // for(i=0;i<objProbSol.iNrOfNodes;i++)
   //lFrequencyBasedMemory[i]=0;
   

    iIteratWithNoImprove=0;

while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
		    iIterationNumber++;

			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=(rand() % 100);

			//cout<<"rand nr..."<<intRandom<<endl;
			if (intRandom< intRandomProb)
			{  	
				/*
				for(i=0;i<objProbSol.iNrOfNodes;i++)
				{
				       if(objProbSol.iCliqueSizes[i]==objProbSol.iLargestClique)
					   {
					    
						 iTempNode=objProbSol.iElimOrdering[i];
						 // iTempNode=objProbSol.iLargestCliqueNode; 
						 iTempPos=(rand() % (objProbSol.iNrOfNodes));

  						 dTempFitness=objProbSol.calculateFitnessFromMove(iTempNode,iTempPos);

						 // if (isSolutionTabu(iTempNode)==0) 
						 // {
							 // cout<<endl<< objProbSol.iLargestClique <<endl;
							//	cout<<endl<<endl<<iTempNode<<"   "<<iTempPos<<endl<<endl; 
						
						 if (dTempFitness-fFitnessOfBestSol<3){
							objProbSol.DoMove(iTempNode,iTempPos);
							lNumberOfEval++;
						//	updateTabuList(iTempNode);
						 }
						 // else updateTabuList(-1);
					   }
				}
				
				  */
				
				 
	        			//Select randomly the set to be removed or added

				        //iTempNode=(rand() % (objProbSol.iNrOfNodes));
				        
				        iTempNode=objProbSol.iLargestCliqueNode; 
						iTempPos=(rand() % (objProbSol.iNrOfNodes));
					
			    		//	cout<<endl<<endl<<iTempNode<<"   "<<iTempPos<<endl<<endl; 
	
						objProbSol.DoMove(iTempNode,iTempPos);
						
						lNumberOfEval++;
				
				
			}
		else 
		{
	            
		//	cout<<"Number of maximal cliques...   "<<objProbSol.iNrNodesLargestClique<<endl;  
			// to be changed here --- One node among all nodes with maximum cliques should be choosen
			    i=objProbSol.iLargestCliqueNode;
				

				if (objProbSol.iCliqueSizes[i]==objProbSol.iLargestClique)
				{
     			//	objProbSol.nearestNodes(i,&prePosition,&sucPosition); 

 	//			   for(j=0;j<objProbSol.iNrOfNodes;j++)
				  //  for(k=0;k<2 ;k++)
					
					for(k=0;k<objProbSol.iTempNrOfNeighboors[i] ;k++)
					{
					   j=objProbSol.iNodePosInOrdering[objProbSol.iGraph[i][k]]; 
                 
					//	if (k==0) j=prePosition;
					//	else j=sucPosition;

					        dTempFitness=objProbSol.calculateFitnessFromMove(i,j);
							
						        lNumberOfEval++;
								if (dBestIterationFitness>dTempFitness)
								{
									dBestIterationFitness=dTempFitness;
									iTempNode=i;
									iTempPos=j;
									iNrSolEqual=1;
							
								}
								
								else if (dBestIterationFitness==dTempFitness )
								{	
                                        
        								iNrSolEqual++;
										//To break ties randomly in case the solutions have the same fitness
										intRandom=(rand() % iNrSolEqual )+1;
										if (intRandom==iNrSolEqual)  
										{
											iTempNode=i;
											iTempPos=j;
										
										}

								}
								
							
						}
				}
			}		
			
            //set new fitness 
		    if (iTempNode>=0)
			{	
			   // register sets which 
			   objProbSol.DoMove(iTempNode,iTempPos);
			   lNumberOfEval++;
			}

			
			if (fFitnessOfBestSol>objProbSol.fFitness)
			{    

				for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
				fFitnessOfBestSol=objProbSol.fFitness;

				time(&end);
		      	bestSolTime=(int)(difftime(end, start));

				bSolutionImproved=true;

    	//		cout<<endl<<endl<<objProbSol.iLargestClique <<"   ********************   "<<endl<<endl; 

			}

	
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}

    	    if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;

   }
		
        objProbSol.setNewOrdering(bestSetSolution);

	
		delete [] iTabuList;
		iTabuList=NULL;

		delete [] iTabuListPos;
		iTabuListPos=NULL;

        return bestSolTime;

}



//min conflicts procedure ...
int TabuSearch::minConflictsTS(probSol& objProbSol, bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;
	
	int i,j,k;
	int intRandom;
	int iTempNode;
	int iTempPos;
    int iTempNode1;
	int iTempPos1;

	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
	double dBestIterationFitness;
  
	double dTempFitness;
	bool bSolutionImproved;

	
    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
	int iNrSolEqual=0;
	bool isSolTabu;
	int iNodePosition;
	int iNode2Position;


    time(&start);
	

    dOldBestIterationFitness=lLargestFitness+1;
	
	


    if (initSolGiven==false)
	{
       //objProbSol.randomIntialSolution(); 
       objProbSol.calculateFitness(); 
	}

	
   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
   // cout<< endl<<fFitnessOfBestSol<<endl;
  
    for(i=0;i<objProbSol.iNrOfNodes;i++) 
		iTabuList[i]=0;

   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
  
      
   //  if (iTabuList == NULL)
   //			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

   //   lFrequencyBasedMemory=new int[objProbSol.iNrOfNodes];
   //  if (lFrequencyBasedMemory == NULL)
   //			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
   // for(i=0;i<objProbSol.iNrOfNodes;i++) 
   //lFrequencyBasedMemory[i]=0;
   

    iIteratWithNoImprove=0;

	while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
		    iIterationNumber++;

			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=(rand() % 100);

			//cout<<"rand nr..."<<intRandom<<endl;
			if (intRandom< intRandomProb)
			{  	
				 
			//	for(i=0;i<objProbSol.iNrNodesLargestClique;i ++)		
			//	{       
					
						iTempNode=objProbSol.iMaxCliqueNodes[rand() % objProbSol.iNrNodesLargestClique]; 
				    	
						iTempPos=(rand() % (objProbSol.iNrOfNodes));
						iNodePosition=objProbSol.iNodePosInOrdering[iTempNode]; 


					 /*   objProbSol.nearestNodes(iTempNode,&prePosition,&sucPosition);     
                    	if (iTempPos>prePosition && iTempPos<sucPosition)
								iTempPos=prePosition;
					   */ 
					

					    iTempNode1=objProbSol.iMaxCliqueNodes[rand() % objProbSol.iNrNodesLargestClique];
                        iTempPos1=(rand() % (objProbSol.iNrOfNodes));
                        iNode2Position=objProbSol.iNodePosInOrdering[iTempNode1]; 
					
					if (iIterationNumber%2 ==0)
					{		
						if (this->isNodeTabu(iTempNode,iIterationNumber) ==0 &&  isNodeTabu(iTempNode1,iIterationNumber) ==0)
						
						{
							objProbSol.doubleSwap(iTempNode, iTempPos, iTempNode1, iTempPos1);
							//objProbSol.doubleSwap(iTempNode, iNode2Position, iTempNode1, iNodePosition);
							
							//	reverese the move if the fitness is worse ...
							if (objProbSol.fFitness>fFitnessOfBestSol)
							   objProbSol.doubleSwap(iTempNode, iNodePosition, iTempNode1, iNode2Position);
							else
							{ updateNodeTabuList(iTempNode, iIterationNumber);  
							  updateNodeTabuList(iTempNode1, iIterationNumber);  
							}
						}
					}
					else 
					{
						if (isNodeTabu(iTempNode,iIterationNumber) ==0)
						{ 
								objProbSol.Swap(iTempNode,iTempPos);
							//	 updateNodeTabuList(iTempNode, iIterationNumber);  
						

							//	reverese the move if the fitness is worse ...
							if (objProbSol.fFitness>fFitnessOfBestSol)
									objProbSol.Swap(iTempNode,iNodePosition);
							else	
							{
								this->updateNodeTabuList(iTempNode, iIterationNumber);
							}	
							
						}	
						
					}	

						lNumberOfEval++;
				
						
                    //    cout<<endl<<objProbSol.fFitness<<endl; 

			}

		else 
		{
	            
				i=objProbSol.iLargestCliqueNode;
				
				for(k=0;k<objProbSol.iNrOfNeighboors[i]; k++)
				{       
						int iOldPosition=objProbSol.iNodePosInOrdering[i];
						int bSolAccepted=false;
						
						j=objProbSol.iNodePosInOrdering[objProbSol.iGraph[i][k]]; 
                 
						objProbSol.Swap(i,j);
						dTempFitness=objProbSol.fFitness; 
					
						lNumberOfEval++;

						if (this->isNodeTabu(i,iIterationNumber) ==0 || this->isNodeTabu(j,iIterationNumber) ==0)
								isSolTabu=false;
							else
								isSolTabu=true;
		

						if (isSolTabu==false) 
						{
								if (dBestIterationFitness>dTempFitness)
								{
									dBestIterationFitness=dTempFitness;
									iNrSolEqual=1;
									bSolAccepted=true;
								}

								else if (dBestIterationFitness==dTempFitness )
								{	
									 
									if (bTSDeterministic==false)
									{
										 
        								iNrSolEqual++;
										//To break ties randomly in case the solutions have the same fitness
										intRandom=(rand() % iNrSolEqual )+1;
										if (intRandom!=iNrSolEqual)  
											objProbSol.Swap(i,iOldPosition);
										else
											bSolAccepted=true;
										
									}
								}
						}

						else if (isSolTabu==true)   
							{
								//aspiration criteria...
								if (dTempFitness<fFitnessOfBestSol)
								{
									dBestIterationFitness=dTempFitness;
									iNrSolEqual=1;
									bSolAccepted=true;
									


								}
								else
								{
									objProbSol.Swap(i,iOldPosition);
								}

							}
					
				
						if (bSolAccepted==true)
						{
							
							updateNodeTabuList(i, iIterationNumber); 
							updateNodeTabuList(objProbSol.iGraph[i][k], iIterationNumber); 
						}
				
				
				}
			}		
			
		
			
		
	        		
		
			if (fFitnessOfBestSol>objProbSol.fFitness)
			{    

				for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
				
				fFitnessOfBestSol=objProbSol.fFitness;

				time(&end);
		      	bestSolTime=(int)(difftime(end, start));

				bSolutionImproved=true;

    			//cout<<endl<<endl<<objProbSol.iLargestClique <<"   ********************   "<<endl<<endl; 

			}

	
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}

    	    if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;

	}
		
        objProbSol.setNewOrdering(bestSetSolution);

        return bestSolTime;

}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Simulated annealing procedure  ...
int TabuSearch::SA(probSol& objProbSol, bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;

	int i;
	float fRandom;
	float fTemp;
	int iTempNode;
	int iTempPos;
	int iNodeOldPosition;
    	

	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
  
	float dTempFitness;

	int iInsideNoImprove;
	float fOldFitness=0;

	
	float fTemperature=fInitTemperature;
	int iNrTries=0;
	int iNrIter=0;
    time(&start);

	
   if (initSolGiven==false)
	{
       //objProbSol.randomIntialSolution();
       objProbSol.calculateFitness(); 
	}
   
   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
  
   iIteratWithNoImprove=0;
   
   iNrTries=0;
   while (iNrTries<iMaxRestarts)
	{

	   if (iNrTries>0)
	   {
		   //reheat OR RANDOM RESTART ...
	//		objProbSol.setNewOrdering(bestSetSolution); 
			fTemperature=fInitTemperature;
	   }

		iNrTries++;
//        cout<<endl<<"TRY...  "<<iNrTries<<endl;
		
	   while (fTemperature>fMinTemperature) 
			{
				iIterationNumber++;
		
				iInsideNoImprove=0;
				iNrIter=0;
				
				while (iInsideNoImprove<iMaxInsideNoImprove)
				//while(iNrIter<iMaxIterPerTemperature)
				{
					iNrIter++;
					
					fOldFitness=objProbSol.fFitness; 		
					
					if (rand() % 100 <iRandomNoiseProb)
					iTempNode=(rand() % (objProbSol.iNrOfNodes));	
					else
					iTempNode=objProbSol.iMaxCliqueNodes[rand() % objProbSol.iNrNodesLargestClique]; 
					
					iTempPos=(rand() % (objProbSol.iNrOfNodes));
					iNodeOldPosition=objProbSol.iNodePosInOrdering[iTempNode]; 
                	
					
					if(iTempPos != iNodeOldPosition)
						dTempFitness=objProbSol.calculateFitnessFromSwap(iTempNode,iTempPos);
					else 
						dTempFitness=fOldFitness; 

					/*
					iTempNode1=objProbSol.iMaxCliqueNodes[rand() % objProbSol.iNrNodesLargestClique];
					iTempPos1=(rand() % (objProbSol.iNrOfNodes));
					iNode1OldPosition=objProbSol.iNodePosInOrdering[iTempNode1];

					objProbSol.doubleSwap(iTempNode, iTempPos, iTempNode1, iTempPos1); 
					*/
					
					//if (dTempFitness < fOldFitness || dTempFitness<fFitnessOfBestSol )    ///dTempFitness<fFitnessOfBestSol ex. the case 46<46.23  (fFitnessOfBestSol can be larger, because it includes hyp. decomposition) 
					if (dTempFitness < fOldFitness)
						{
							/*if (fFitnessOfBestSol<=dTempFitness) 
								objProbSol.Swap(iTempNode,iTempPos);
							else
						   {  
						   */
								objProbSol.Swap(iTempNode,iTempPos);

								//objProbSol.bHypertreeAlg =true;
								objProbSol.calculateFitness(); 
								//objProbSol.bHypertreeAlg =false;
	    
								if (fFitnessOfBestSol>objProbSol.fFitness) 
								{
									for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
									fFitnessOfBestSol=objProbSol.fFitness;
							
									time(&end);
		      						bestSolTime=(int)(difftime(end, start));
									iInsideNoImprove=0;
								
								  //  cout<<endl<<"IMPROVED...."<<endl<<objProbSol.fFitness<<"        "<< "       Temperature:"<<"      "<<fTemperature<<endl<<endl; 
								}
								
								//calculate again the fitness to fill approprietly the data structures for tree decompositions ...
								objProbSol.calculateFitness(); 

							
						        
						  // }

						}
					else
						{
							
							iInsideNoImprove++;
							
							fRandom=(float)(rand() % 100)/100 ;
					
							if (fOldFitness-dTempFitness==0)
								fTemp=0.5;
							else
								fTemp =	exp((fOldFitness-dTempFitness)/fTemperature);
							
							//othe rpossibilities for the temperature ...
							//fTemp =	1/(1+ exp(-(fOldFitness-objProbSol.fFitness)/fTemperature));
							//fTemp=	exp((fFitnessOfBestSol-objProbSol.fFitness)/fTemperature);
					

							if (fRandom<fTemp )
							{  
								objProbSol.Swap(iTempNode,iTempPos);
								
							}
													
						}			

				}

				//change Temperature ...
				fTemperature=fTemperature*fTempDecrease;
			
	}


}
        objProbSol.setNewOrdering(bestSetSolution);
        return bestSolTime;

}





//----------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Simulated annealing procedure  ...
int TabuSearch::GreatDeLuge(probSol& objProbSol, bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;

	int i;
	int iTempNode;
	int iTempPos;
	int iNodeOldPosition;
    	

	
	int iIteratWithNoImprove=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
  
	double dTempFitness;

	int iInsideNoImprove=0;
	float fOldFitness=0;
	
	int iNrTries=0;
    
	//Initial 1 , minumum 0.1., decline 0.9, not improvments 10 ...
    time(&start);

	
   if (initSolGiven==false)
	{
       //objProbSol.randomIntialSolution();
       objProbSol.calculateFitness(); 
	}
   
   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
  
   iIteratWithNoImprove=0;
 
   
   iNrTries=0;
   while (iNrTries<iGDMaxRestarts)
	{

		   if (iNrTries>0)
		   {
			   //reheat OR RANDOM RESTART ...
			//	objProbSol.setNewOrdering(bestSetSolution); 

				//parameters for level, up, dev ...
		   }

			iNrTries++;

			iInsideNoImprove=0;
			
			while (iInsideNoImprove<iGDMaxInsideNoImprove)
			{

				
				fOldFitness=objProbSol.fFitness; 		
				
				if (rand() % 100 <iGDRandomNoiseProb)
				iTempNode=(rand() % (objProbSol.iNrOfNodes));	
				else
				iTempNode=objProbSol.iMaxCliqueNodes[rand() % objProbSol.iNrNodesLargestClique]; 
				
				iTempPos=(rand() % (objProbSol.iNrOfNodes));
				iNodeOldPosition=objProbSol.iNodePosInOrdering[iTempNode]; 
                
				
				if(iTempPos != iNodeOldPosition)
					dTempFitness=objProbSol.calculateFitnessFromSwap(iTempNode,iTempPos);
				else 
					dTempFitness=fOldFitness; 

				
				if (dTempFitness < fFitnessOfBestSol+DEV)
					{
						if (fFitnessOfBestSol<dTempFitness) 
							objProbSol.Swap(iTempNode,iTempPos);
						else
					   {   
							objProbSol.Swap(iTempNode,iTempPos);
							
							//objProbSol.bHypertreeAlg =true;
							objProbSol.calculateFitness(); 
							//objProbSol.bHypertreeAlg =false;
							
							if (fFitnessOfBestSol>objProbSol.fFitness) 
							{
								for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
								fFitnessOfBestSol=objProbSol.fFitness;
						
								time(&end);
		      					bestSolTime=(int)(difftime(end, start));
								iInsideNoImprove=0;
							
								cout<<endl<<"IMPROVED...."<<endl<<objProbSol.fFitness<<endl<<endl; 
							}
							
							//calculate again the fitness to fill approprietly the data structures for tree decompositions ...
							objProbSol.calculateFitness(); 
		    
					   }
					}
				else
					{
						
						iInsideNoImprove++;
						
					}			

			}

			

	}
        
		objProbSol.setNewOrdering(bestSetSolution);
        return bestSolTime;

}

























//tabu search procedure ...
int TabuSearch::fiduciaSearchAlg(probSol& objProbSol,bool initSolGiven)  
{

    time_t start, end;
    int bestSolTime=0;

	int i,j,k;
	int intRandom;
	int iTempNode=0;
	int iTempPos=0;

	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fFitnessOfBestSol;
	double dBestIterationFitness;
 

	double dTempFitness;
	bool bSolutionImproved;

    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
    int sucPosition, prePosition;

    time(&start);
	

    dOldBestIterationFitness=lLargestFitness+1;
	

    objProbSol.randomIntialSolution(); 
    
	objProbSol.calculateFitness(); 

   for(i=0;i<objProbSol.iNrOfNodes;i++) 
	   bestSetSolution[i]=objProbSol.iElimOrdering[i]; 
   
   fFitnessOfBestSol=objProbSol.fFitness;
   
   intTabuLength=objProbSol.iNrOfNodes;
   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   iTabuList = new int[intTabuLength];
   iTabuListPos = new int[intTabuLength];

    
 //  if (iTabuList == NULL)
 //	   writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

  // lFrequencyBasedMemory=new int[objProbSol.iNrOfNodes];
 //  if (lFrequencyBasedMemory == NULL)
//			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
  // for(i=0;i<objProbSol.iNrOfNodes;i++) 
   //lFrequencyBasedMemory[i]=0;
   

    iIteratWithNoImprove=0;

while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
		    iIterationNumber++;

            if (iIterationNumber%objProbSol.iNrOfNodes==0)
			{  
				objProbSol.setNewOrdering(bestSetSolution);
				resetTabuList();
				
			}


			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=(rand() % 100);

			//cout<<"rand nr..."<<intRandom<<endl;
			if (intRandom< intRandomProb)
			{  	
  						//Select randomly the set to be removed or added

				        iTempNode=(rand() % (objProbSol.iNrOfNodes));
				        iTempPos=(rand() % (objProbSol.iNrOfNodes));
					
						cout<<endl<<endl<<iTempNode<<"   "<<iTempPos<<endl<<endl; 
                       
			          
						/*
						objProbSol.DoMove(iTempNode,iTempPos);
						
					
						
						  	lNumberOfEval++;
				
                        this->updateTabuList(iTempNode); 
						*/
		
			}
		else 
		{
			for(i=0; i< objProbSol.iNrOfNodes; i++)   
			{
				objProbSol.nearestNodes(i,&prePosition,&sucPosition); 

				if (objProbSol.iCliqueSizes[i]>objProbSol.iLargestClique-objProbSol.iLargestClique)
				{
				//for(j=0;j<objProbSol.iNrOfNodes;j++)
				
				for(k=0;k<2;k++)
				{
					if (k==0) j=prePosition;
					else j=sucPosition;

						dTempFitness=lLargestFitness+1; 	

						dTempFitness=objProbSol.calculateFitnessFromMove(i,j);
				
		//				cout<<endl<<"Fitness...:   "<<dTempFitness;
		
						
						lNumberOfEval++;
						if (isSolutionTabu(i)==0)   
						{
							if (dBestIterationFitness>dTempFitness)
							{
								dBestIterationFitness=dTempFitness;
								iTempNode=i;
								iTempPos=j;

						
							}

							else if (dBestIterationFitness==dTempFitness )
							{	
							   /*
								// use of long term memory considering movement of each set...
								// diversification...
								else if (iIteratWithNoImprove>10)
								{
									if (lFrequencyBasedMemory[i]<lFrequencyBasedMemory[iTempNode])
									{	iTempNode=i;
										 

									
									
									}
								}

								 */
								if (bTSDeterministic==false)
								{
									intRandom=(rand() % 10 )+1;
									if (intRandom<2)  
									{
										iTempNode=i;
										iTempPos=j;
										
									}
								}

							}
						}

						else if (isSolutionTabu(i)==1)   
						{
							//aspiration criteria...
							if (dTempFitness<fFitnessOfBestSol)
							{
								dBestIterationFitness=dTempFitness;
						
								iTempNode=i;
								iTempPos=j;
							
						  }

						}
				}
				}		
			}
	
		}
		
             
		    //set new fitness 
		    if (iTempNode>=0)
			{	
			   // register sets which 
			   objProbSol.DoMove(iTempNode,iTempPos);
			   lNumberOfEval++;
			
			}
            
           // cout<<endl<<"Fitness...:   "<<objProbSol.iLargestClique<<"        "<< objProbSol.fFitness;
			
			if (fFitnessOfBestSol>objProbSol.fFitness)
			{  

				for(i=0;i<objProbSol.iNrOfNodes;i++) bestSetSolution[i]=objProbSol.iElimOrdering[i];
				fFitnessOfBestSol=objProbSol.fFitness;

				time(&end);
		      	bestSolTime=(int)(difftime(end, start));

				bSolutionImproved=true;

			}

			updateTabuList(iTempNode);
			
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}

    	    if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;

	}
	


		
	        objProbSol.setNewOrdering(bestSetSolution);

	
			delete [] iTabuList;
			iTabuList=NULL;
 
		    delete [] iTabuListPos;
			iTabuListPos=NULL;
			
			return bestSolTime;

}










//NM: function which returns 1 if the solution is tabu, otherwise 0
	int TabuSearch::isSolutionTabu(int iNode, int pos)
{ 
   int i;

   if (pos ==-1)
   {  for (i=0;i<intTabuLength;i++)
		{
			if (iNode==iTabuList[i]) {return 1;}
		}
   }
	else
	{  
	
		for (i=0;i<intTabuLength;i++)
		{
			if (iNode==iTabuList[i] && pos==iTabuListPos[i]) {return 1;}
   
	   }

	}

        return 0;
}


	//returns 1 if one of node in swap is ta bu, otherwise true ...
int TabuSearch:: isSwapTabu(int iNode1, int iNode2)
	{


		for (int i=0;i<intTabuLength;i++)
		{
			if (iNode1==iTabuList[i] || iNode2==iTabuList[i]) {return 1;}
		}
		return 0; 

	}







//NM: updates tabu list: adds new  tabu move based on the position
void TabuSearch::updateTabuList(int iNode, int pos)
{  
   static long intPosition=0;
   intPosition=intPosition %  intTabuLength;
   iTabuList[intPosition]=iNode;
   iTabuListPos[intPosition]=pos;

   intPosition++;
}



// Resets all elements of tabu list to -1
void TabuSearch:: resetTabuList()
{
  int i;

    for (i=0;i<intTabuLength;i++)
	{
		iTabuList[i]=-1;
		iTabuListPos[i]=-1;
	}
		
}




	//returns 1 if one a node is Tabu, otherwise true ...
int TabuSearch::isNodeTabu(int iNode1, int iIteration)
	{


		if (iIteration-iTabuList[iNode1] <= intTabuLength && iTabuList[iNode1]!=0) {return 1;}
		else return 0; 

	}



//NM: updates tabu list: marks the iteration in whcih the node was swapped
void TabuSearch::updateNodeTabuList(int iNode, int iIteration)
{  
   iTabuList[iNode]=iIteration;
 
}





