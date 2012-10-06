#include <SetCoverLS.hpp>
#include <Globals.hpp>
#include <iostream>
#include <ctime>
#include <cstdlib>

const int DET_GREEDY=1;
const int RND_GREEDY=2;
const int MAX_GREEDY=3;

using namespace std;
using namespace sharp;

float PENALTY_FOR_VAR=1;
float PENALTY_FOR_SETS=1;
const float PENALTY_FOR_SINGLECOVER=0.000;
int uperBound=100000;
const int randomOption=1;

SetCoverLS::SetCoverLS()
{
	lProblem=NULL;
	iNrOfVar=0;
	iNrOfSets=0;  
	iCoveredVar=NULL; 
    iNrOfCoveredVar=0;
    iNrVarCovBySingleSet=0;
	iSelectedSets=NULL;
	iNumberOfSelectedSets=0;
	iUVarInSets=NULL;
    iVarInSets=NULL;

	iSetsVar=NULL;
	iSetsVarRange=NULL;
	iVarSets=NULL;
	iVarSetsRange=NULL;

	lTabuList=NULL;
	lFrequencyBasedMemory=NULL;
	intRandomProb=0;
	lNumberOfEvaluations=1000000000;
    iMaxNrNotImproves=700000;
	intTabuLength=10;
    bTSDeterministic=false;
	bInitialSolGreedy=true;

    dFitness=0;
}



SetCoverLS::~SetCoverLS()
{   
	//delete [] lProblem;
    lProblem=NULL;
	delete [] iCoveredVar; 
    delete [] iSelectedSets;
    delete [] iUVarInSets;
	delete [] iVarInSets;

    delete [] iSetsVar;
	delete [] iSetsVarRange;
	delete [] iVarSets;
	delete [] iVarSetsRange;

    delete [] lTabuList;
    delete [] lFrequencyBasedMemory;	
}




/*
***Description***
Checks if generated solution for set covering is a legal solution
  
INPUT: 
OUTPUT: The message is given   

***History***
Written: (xx.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void SetCoverLS::iCheckSolution()
 {int iSets=0;
  int i,j;
  bool bCovered;
  bool bCheckPassed=true;
  int iNotCovVar=0;



  for (i=0;i<iNrOfVar;i++)
  {   
	  bCovered =false;
	  for(j=0;j<iNrOfSets;j++)
	  {
		  if (iSelectedSets[j]==1)
		  { 
			  if (lProblem[j+i*iNrOfSets]==1) {bCovered=true;break;}
		  }
	  }

	  if (bCovered==false) {iNotCovVar++;bCheckPassed=false;}
     
  }
  
  for(i=0;i<iNrOfSets;i++)
  {
	  if (iSelectedSets[i]==1) iSets++;
  }

  if (bCheckPassed) 
  {cout<<"SOLUTION IS LEGAL";
      cout<<"   Sets:  "<<iSets<<endl;
  }
  else
  {cout<<endl<<"SOLUTION IS NOT LEGAL"<<endl<< " NOT COV VAR "<< iNotCovVar;
      cout<<"      Sets:  "<<iSets<<endl;

  }

 }



/*
***Description***
Generates the arrays which contains information for sets covering each variable, 
and the variables wach set covers 
  
INPUT: 
OUTPUT: The result is given in private variable of class   

***History***
Written: (xx.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void SetCoverLS::setVarSets()
 {
	int iNrOfOnes=0;
    int i,j;
	int iPosition;
    int iFrom;

	for(i=0;i<iNrOfVar;i++)
	{
		for(j=0;j<iNrOfSets;j++)
		{
			if (lProblem[i*iNrOfSets+j]==1 ) iNrOfOnes++;
		}
	}
   

   iSetsVar=new int[iNrOfOnes];
   iVarSets=new int[iNrOfOnes];
   iSetsVarRange=new int[2*iNrOfSets];
   iVarSetsRange=new int[2*iNrOfVar];

    if (iSetsVar == NULL || iVarSets == NULL || iSetsVarRange == NULL || iVarSetsRange == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:setVarSets");
 

    //generate var sets...
	iPosition=0;
   	for(i=0;i<iNrOfVar;i++)
	{   iFrom=iPosition;
		for(j=0;j<iNrOfSets;j++)
		{
			if (lProblem[i*iNrOfSets+j]==1 )
			{
				iVarSets[iPosition]=j;
                iPosition++;  
			}
		}

        iVarSetsRange[2*i]=iFrom;
		if (iFrom==iPosition) iVarSetsRange[2*i+1]=-1; 
		else iVarSetsRange[2*i+1]=iPosition-1;
	}
   
    //generate var sets...
	iPosition=0;
   	for(i=0;i<iNrOfSets;i++)
	{   iFrom=iPosition;
		for(j=0;j<iNrOfVar;j++)
		{
			if (lProblem[j*iNrOfSets+i]==1 )
			{
				iSetsVar[iPosition]=j;
                iPosition++;  
			}
		}

        iSetsVarRange[2*i]=iFrom;
		if (iFrom==iPosition) iSetsVarRange[2*i+1]=-1; 
		else iSetsVarRange[2*i+1]=iPosition-1;
	}


 }



// sets relevant parametres for Tabu Search
 void SetCoverLS::setTSParametres(long lMaxNrOfEvaluations, int iNrOfMaxIterationsNoImprovment, int iProbForRandomMove, int iLengthOfTabuList,bool bIsTSDeterministic, bool bGenerateGreedyInitialSol)
{
	lNumberOfEvaluations=lMaxNrOfEvaluations;
    iMaxNrNotImproves=iNrOfMaxIterationsNoImprovment;
	intRandomProb=iProbForRandomMove;
	intTabuLength=iLengthOfTabuList;
    bTSDeterministic=bIsTSDeterministic;
	bInitialSolGreedy=bGenerateGreedyInitialSol;
}





/*
***Description***
Calls other procedures for calculating of miniml set cover
  
INPUT: setCovMatrix - set cover problem given in a matrix (rows represent variables, whereas columns
       represents sets.  
	   iNumberOfSets -- number of sets
	   iNumberOfVariables - number of variables to be covered 
	   iSearchOption: 1-local search procedure is called, 2-greedy algorithm for set covering is applied  
OUTPUT: bestSolTime: time for which the best cover is found
        The result is given in private variable of class: iSelectedSets   

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

int SetCoverLS::minimalSC(bool* setCovMatrix, int iNumberOfSets, int iNumberOfVariables, int iSearchOption)
{  
     int bestSolTime=0;
	 
	 iNrOfVar=iNumberOfVariables;
	 iNrOfSets=iNumberOfSets;  

     lProblem=setCovMatrix;
     // sets information for the relations between sets and variables...
	 setVarSets(); 
	 	 
	 if (iSearchOption==1)
	 {	// calls the procedure which repairs the solution obtained by set cover)
		 bestSolTime=localSearchSC2(); 
	 }
	 else if (iSearchOption==2)
	 {// calls only procedure for greedy set cover 
		greedySetCover();	    
 	 }	

     //time for which the solution is found... 
	 return bestSolTime;
}


/*
***Description***
Greedy algorithm for set cover 
  
INPUT: It uses private variables of the class...
iGreedyType -- type of greedy algorithm: deterministic greedy (DET_GREEDY) 1 (Optional), random greedy (RND_GREEDY) 2
and greedy based also on the number of elements of the sets (MAX_GREEDY) 3

OUTPUT: The result is given in private variable of class: iSelectedSets  
 

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void SetCoverLS::greedySetCover(int iGreedyType)
{ 	
	int i,j;
    int iVar;
	int iMySet;

	iCoveredVar=new int[iNrOfVar];

    if (iCoveredVar == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
 
    for (i=0;i<iNrOfVar;i++)
	{iCoveredVar[i]=0;}
	iNrVarCovBySingleSet=0;

    iNrOfCoveredVar=0;

    iSelectedSets=new int[iNrOfSets];
    if (iSelectedSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
     for (i=0;i<iNrOfSets;i++)
	 {iSelectedSets[i]=0;}	
   
     // calculate nr of variables which are covered by each set
	 iUVarInSets=new int[iNrOfSets];
     if (iUVarInSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
     for (i=0;i<iNrOfSets;i++)
	 {iUVarInSets[i]=0;
	 }

     for (i=0;i<iNrOfSets;i++)
	 {
		 for(j=iSetsVarRange[2*i];j<=iSetsVarRange[2*i+1];j++)
		 {
			 iVar=iSetsVar[j];
			 if (lProblem[i+iVar*iNrOfSets]==1)
			 {iUVarInSets[i]++;}
		 }

	 }


     iVarInSets=new int[iNrOfSets];
     if (iVarInSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
	 for (i=0;i<iNrOfSets;i++)
		{iVarInSets[i]=iUVarInSets[i];}
  

  //greedy algorithm for Set Cover...
  bool bGreedyFinish=false;
  while ( bGreedyFinish==false)
  {
     	
	 // find the set with maximum number of uncovered varibales (in case of equality 
	 // break ties randomly, or apply another greedy option...

  	 int itempMax=0;
  	 int itempSet=0;
  	 for (i=0;i<iNrOfSets;i++)
	 {
		 {
			 if (iUVarInSets[i]>itempMax && iSelectedSets[i]!=1)
			 {
				 itempSet=i;itempMax=iUVarInSets[i];
			 }
             else if (iUVarInSets[i]==itempMax && iSelectedSets[i]!=1)
			 {   
				 if (iGreedyType== MAX_GREEDY)
				 {
					//favors the sets which have larger number of variables -- could be important in case of further improves with LS
					if (iVarInSets[itempSet]<iVarInSets[i])
					{itempSet=i;itempMax=iUVarInSets[i];}
				 }
				 else if (iGreedyType== RND_GREEDY)
				 {   //selest randomly the next set
					 if (random_range(0, 100)<30) {itempSet=i;itempMax=iUVarInSets[i];}
				 } 
			}
		 }

	 }
     
     
     if (itempMax==0 || iNrOfCoveredVar==iNrOfVar)
	 {//either all variables are covered, or there is no more sets which can cover variables
		 bGreedyFinish=true;
	 }
     else
	 {
		 iNumberOfSelectedSets++;
		 // Add set itempSet in the selected sets
		 iSelectedSets[itempSet]=1;
		 // update covered variables and the maximum of uncovered variables in each set 
       	
	   for(i=iSetsVarRange[2*itempSet];i<=iSetsVarRange[2*itempSet+1];i++)
	   {     
	   	   iVar=iSetsVar[i];
			if (lProblem[itempSet+iVar*iNrOfSets]==1 )
			{	  
				if (iCoveredVar[iVar]==0)
				{
				  iNrOfCoveredVar++;
				  
				  //variable is covered, update iUVarInSets ...
				  for(j=iVarSetsRange[2*iVar];j<=iVarSetsRange[2*iVar+1];j++)
				  {
					  iMySet=iVarSets[j];
					  if (lProblem[iMySet+iVar*iNrOfSets] ==1)
					  {if(iUVarInSets[iMySet]!=0) iUVarInSets[iMySet]--;}
				  }

				}

				iCoveredVar[iVar]++;
			    if (iCoveredVar[iVar]==1) iNrVarCovBySingleSet++;
			    else if (iCoveredVar[iVar]==2) iNrVarCovBySingleSet--;
				
				
			 }
				  		 
		 }
 
	 }

  }

}








/*
***Description***
Generates empty solution 
  
INPUT: It uses private variables of the class...

OUTPUT: The result is given in private variable of class: iSelectedSets  
 

***History***
Written: (20.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void SetCoverLS::emptySetCover()
{ 	
	int i;

	iCoveredVar=new int[iNrOfVar];

    if (iCoveredVar == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:emptySetCover");
 
    for (i=0;i<iNrOfVar;i++)
	{iCoveredVar[i]=0;}	

    iNrOfCoveredVar=0;
	iNrVarCovBySingleSet=0;

    iSelectedSets=new int[iNrOfSets];
    if (iSelectedSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:emptySetCover");
   
     
     for (i=0;i<iNrOfSets;i++)
	 {iSelectedSets[i]=0;}	
   
}



//changes penalties for number of sets and not cov. variables...
void SetCoverLS::changePenalties()
{
   static bool increasePenalty=true;
   
   if (increasePenalty==false)
   {
        PENALTY_FOR_VAR=1;
		PENALTY_FOR_SETS=1;
		increasePenalty=true;
   }
   else
   {    PENALTY_FOR_VAR=1;
		PENALTY_FOR_SETS=2;
		increasePenalty=false;
   }
}




//NM: function which returns 1 if the solution is tabu, otherwise 0
int SetCoverLS:: isSolutionTabu(int iSet)
{ 
   int i;

   for (i=0;i<intTabuLength;i++)
	{
		if (iSet==lTabuList[i]) {return 1;}
   
   }
        return 0;
}




//NM: updates tabu list: adds new  tabu move based on the position
void SetCoverLS:: updateTabuList(int iSet)
{  
   static long intPosition=0;
   intPosition=intPosition %  intTabuLength;
   lTabuList[intPosition]=iSet;
   intPosition++;
}


//NM:Calculates fitness of current solution   
void SetCoverLS::calculateFitness()
{
	//calculates fitness based on number of Sets and number of covered variables...
    //  dFitness=(iNrOfVar-iNrOfCoveredVar)*1+iNrOfSets*1;
	dFitness=(iNrOfVar-iNrOfCoveredVar)*PENALTY_FOR_VAR+iNumberOfSelectedSets*PENALTY_FOR_SETS;

	dFitness=dFitness +PENALTY_FOR_SINGLECOVER*iNrVarCovBySingleSet;

}



/*
***Description***
Calculates fitness of current solution based on move (remove or add new set for cov. variables)  
INPUT: iSet -- set which is removed or added from C (sets used to cover variables)
		//iOption=1 =>  set is added in the sets which covers variables
		//iOption =-1 => set is removed from the sets which covers variables 


OUTPUT: New fitness  
 

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
double SetCoverLS::calculateFitnessFromMove(int iSet, int iOption)
{
	int i;
	int iTemp=0;
    int iTempCovVar;
    double dTempFitness;
	int iTempNrVarCovBySingleSet;
    int iVar;

    iTempCovVar=iNrOfCoveredVar;
	iTempNrVarCovBySingleSet=iNrVarCovBySingleSet;

	if (iOption==1)
	{
	
      iTemp=iNumberOfSelectedSets+1;
	  //Changes in number of covered variables     
      for(i=iSetsVarRange[2*iSet];i<=iSetsVarRange[2*iSet+1];i++)
	  {   iVar=iSetsVar[i];

		  if (lProblem[iSet+iVar*iNrOfSets]==1) 
		  {if (iCoveredVar[iVar]==0) iTempCovVar++;

		    	  if (iCoveredVar[iVar]==1) 
		   	      iTempNrVarCovBySingleSet--;
        		  else if (iCoveredVar[iVar]==0) iTempNrVarCovBySingleSet++;
		  } 
	  }
      

	  /*

	  //ADDED on 29.09.2005...
	  // check is any set will ged redundant in case this set is added...
      for(i=0;i<iNrOfSets;i++)
	  { if (iSelectedSets[i]==1)
		{// will the variables of this set be covered by the other sets and new added set...
		  
		}
	  }

      */
    
	
	}
	else if (iOption==-1) 
    {
		iTemp=iNumberOfSelectedSets-1;


        //Changes in number of covered variables     
	for(i=iSetsVarRange[2*iSet];i<=iSetsVarRange[2*iSet+1];i++)
	  {   iVar=iSetsVar[i];

			if (lProblem[iSet+iVar*iNrOfSets]==1) 
			{
		    	  if (iCoveredVar[iVar]==1)
				  {
		   			iTempNrVarCovBySingleSet--;
					iTempCovVar--;
				  }
        		  else if (iCoveredVar[iVar]==2) iTempNrVarCovBySingleSet++;
		
			} 
		
		}

	
	}


	dTempFitness=(iNrOfVar-iTempCovVar)*PENALTY_FOR_VAR+iTemp*PENALTY_FOR_SETS;
//    if (iNrOfVar-iTempCovVar==0) dTempFitness=dTempFitness-2;

		dTempFitness=dTempFitness +PENALTY_FOR_SINGLECOVER*iTempNrVarCovBySingleSet;

 	return dTempFitness;
}



/*
***Description***
Perfoms particular move: Adds or removes a set from  C (sets used to cover variables)
and based on that changes the appropriate variables      
INPUT: iSet -- set which is removed or added from C (sets used to cover variables)
		//iOption=1 =>  set is added in the sets which covers variables
		//iOption =-1 => set is removed from the sets which covers variables 

  
OUTPUT: New fitness and change of other private variables in the class  
        !!! For now iUVarInSets is not updated...

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void SetCoverLS::DoMove(int iSet, int iOption)
{
	int i;
	int iVar;
		   
	if (iOption==1)
	{
      iSelectedSets[iSet]=1;
	  iNumberOfSelectedSets++;
	  //Changes in covered variables     

	for(i=iSetsVarRange[2*iSet];i<=iSetsVarRange[2*iSet+1];i++)
	  {   iVar=iSetsVar[i];

 		  if (lProblem[iSet+iVar*iNrOfSets]==1) 
		  {
			   if (iCoveredVar[iVar]==0) iNrOfCoveredVar++;
		   		   
			   iCoveredVar[iVar]=iCoveredVar[iVar]+1;
			   
			   if (iCoveredVar[iVar]==1) 
				{iNrVarCovBySingleSet++;}
			   else if (iCoveredVar[iVar]==2) 
			   {iNrVarCovBySingleSet--;}
			 
		  } 
	  }

	}
	else if (iOption==-1) 
    {
		iNumberOfSelectedSets--;
        iSelectedSets[iSet]=0;
		
      //Changes in covered variables  
	  for(i=iSetsVarRange[2*iSet];i<=iSetsVarRange[2*iSet+1];i++)
	  {   
		    iVar=iSetsVar[i];
	    	if (lProblem[iSet+iVar*iNrOfSets]==1) 
			{
			  //is this variable covered only from this selected set
			  iCoveredVar[iVar]=iCoveredVar[iVar]-1;	
			  
			  if (iCoveredVar[iVar]==1) 
				{iNrVarCovBySingleSet++;}
			  else if (iCoveredVar[iVar]==0) 
				{	iNrVarCovBySingleSet--; 
					iNrOfCoveredVar--;
				}
			
			} 
		}
	}

    lFrequencyBasedMemory[iSet]++;
	dFitness=(iNrOfVar-iNrOfCoveredVar)*PENALTY_FOR_VAR+iNumberOfSelectedSets*PENALTY_FOR_SETS;
	dFitness=dFitness +PENALTY_FOR_SINGLECOVER*iNrVarCovBySingleSet;
}



/*
***Description***
Perfoms particular move: Removes iSet1 and adds iSet2 C (sets used to cover variables)
and based on that changes the appropriate variables      
INPUT: iSet1 -- set which should be removed from C (sets used to cover variables)
	   iSet2 -- set which should be added in set C (sets used to cover variables)
	
  
OUTPUT: New fitness and change of other privae variables in the class  
        !!! For now iUVarInSets is not updated...

***History***
Written: (02.10.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void SetCoverLS::DoSwapMove(int iSet1, int iSet2)
{
	int i;
	int iVar;



	    //remove iSet1
    	iNumberOfSelectedSets--;
        iSelectedSets[iSet1]=0;
		
        //Changes in covered variables     
	    for(i=iSetsVarRange[2*iSet1];i<=iSetsVarRange[2*iSet1+1];i++)
		{  iVar=iSetsVar[i];


			if (lProblem[iSet1+iVar*iNrOfSets]==1) 
			{//is this variable covered only from this selected set
			   
			  iCoveredVar[iVar]=iCoveredVar[iVar]-1;	
			  
			  if (iCoveredVar[iVar]==1) 
				{iNrVarCovBySingleSet++;}
			  else if (iCoveredVar[iVar]==0) 
				{	iNrVarCovBySingleSet--; 
					iNrOfCoveredVar--;
				}

			
			} 
		
		}


	  //Add iSet2
      iSelectedSets[iSet2]=1;
	  iNumberOfSelectedSets++;
	  //Changes in covered variables     
     for(i=iSetsVarRange[2*iSet2];i<=iSetsVarRange[2*iSet2+1];i++)
		{  iVar=iSetsVar[i];
		  if (lProblem[iSet2+iVar*iNrOfSets]==1) 
		  {
		   if (iCoveredVar[iVar]==0) iNrOfCoveredVar++;
		   	   
		   iCoveredVar[iVar]++;
		   
		   if (iCoveredVar[iVar]==1) 
			      iNrVarCovBySingleSet++;
		   else if (iCoveredVar[iVar]==2) iNrVarCovBySingleSet--;
		 
		  } 
	  }


    lFrequencyBasedMemory[iSet1]++;
    lFrequencyBasedMemory[iSet2]++;


	dFitness=(iNrOfVar-iNrOfCoveredVar)*PENALTY_FOR_VAR+iNumberOfSelectedSets*PENALTY_FOR_SETS;
 
	dFitness=dFitness +PENALTY_FOR_SINGLECOVER*iNrVarCovBySingleSet;
 
}



/*
***Description***
Calculates fitness of current solution based on swap move      
INPUT: iSet1 -- set which should be removed from C (sets used to cover variables)
	   iSet2 -- set which should be added in set C (sets used to cover variables)


OUTPUT: New fitness  
 

***History***
Written: (xx.10.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/


double SetCoverLS::calculateFitnessFromSwapMove(int iSet1, int iSet2)
{

	int i;
	int iVar;
	int iTemp;
    int iTempCovVar;

    double dTempFitness;
	int iTempNrVarCovBySingleSet;


    iTempCovVar=iNrOfCoveredVar;
	iTempNrVarCovBySingleSet=iNrVarCovBySingleSet;

     //GO through both sets in the same time...

     iTemp=iNumberOfSelectedSets;

	  //Removing Set1: Changes in number of covered variables     
      for(i=iSetsVarRange[2*iSet1];i<=iSetsVarRange[2*iSet1+1];i++)
	  { 
		iVar=iSetsVar[i];
         
		if (lProblem[iSet1+iVar*iNrOfSets]==1 && lProblem[iSet2+iVar*iNrOfSets]!=1)
		  {	    
			     if (iCoveredVar[iVar]==1)
				  {
		   			iTempNrVarCovBySingleSet--;
					iTempCovVar--;
				  }
        		  else if (iCoveredVar[iVar]==2) iTempNrVarCovBySingleSet++;
		  }
		
	  }
      
  	  //Adding of iSet2: Changes in number of covered variables     
      for(i=iSetsVarRange[2*iSet2];i<=iSetsVarRange[2*iSet2+1];i++)
	  { 
		  iVar=iSetsVar[i];
		  if (lProblem[iSet2+iVar*iNrOfSets]==1 && lProblem[iSet1+iVar*iNrOfSets]!=1)
          {
			  if (iCoveredVar[iVar]==0) iTempCovVar++;

		      if (iCoveredVar[iVar]==1) 
		   	  iTempNrVarCovBySingleSet--;
        	  else if (iCoveredVar[iVar]==0) iTempNrVarCovBySingleSet++;

		  }

	  }
      
	dTempFitness=(iNrOfVar-iTempCovVar)*PENALTY_FOR_VAR+iTemp*PENALTY_FOR_SETS;
    //if (iNrOfVar-iTempCovVar==0) dTempFitness=dTempFitness-2;

	dTempFitness=dTempFitness +PENALTY_FOR_SINGLECOVER*iTempNrVarCovBySingleSet;

 	return dTempFitness;

}




/*
***Description***
Procedure based on local search (tabu search) to improve greedy algorithm for set cover 
  
INPUT: It uses private variables of the class...

OUTPUT: The result is given in private variable of class: iSelectedSets  
 

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

int SetCoverLS::localSearchSC2()
{

    time_t start, end;
    int bestSolTime=0;

	int i;
	int intRandom;
	int iTempSet;

	const long lLargestFitness = 100000000; 
	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	
	// needed for aspiration criteria
	double fBestFitness;
	double fFitnessOfBestSol;
	double dBestIterationFitness;
    int tempSolSetsNr;
	int tempBestSolNrCovVar;

	double dTempFitness;
	bool bSolutionImproved;

    int *bestSetSolution;
    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
      
	bool bFinalPhase=false;
    bool bLegalSolutionFound=false;
	int *iConnectedSets;
    
    time(&start);
	
    iConnectedSets=new int[iNrOfSets];
		
	if (iConnectedSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC1");

	//int *tempSelectedSets=new int[iNrOfSets];

    dOldBestIterationFitness=lLargestFitness+1;
	
	bestSetSolution=new int[iNrOfSets];
	if (bestSetSolution == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

 
	
    //calculate initial solution

	if (bInitialSolGreedy){
		greedySetCover();bLegalSolutionFound=true;
				    time(&end);
		      	bestSolTime=(int)(difftime(end, start));
		}
    else emptySetCover();
    this->calculateFitness(); 


    intTabuLength=(int)(iNumberOfSelectedSets*((float)intTabuLength/100))+1;

    if (iNumberOfSelectedSets>0) uperBound=iNumberOfSelectedSets-1; else uperBound=10000 ; 

   // print best solution
   // cout<<endl<<endl<<"****Greedy SOLUTION***** Fitness: "<<dFitness<<endl<<endl;
   // cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<endl<<endl; 


   for(i=0;i<iNrOfSets;i++) bestSetSolution[i]=iSelectedSets[i];
   fFitnessOfBestSol=dFitness;
   tempSolSetsNr=iNumberOfSelectedSets;
   tempBestSolNrCovVar=iNrOfCoveredVar;

   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   lTabuList = new int[intTabuLength];
    
   if (lTabuList == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

   lFrequencyBasedMemory=new int[iNrOfSets];
   if (lFrequencyBasedMemory == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
   for(i=0;i<iNrOfSets;i++) lFrequencyBasedMemory[i]=0;
   
    fBestFitness=dFitness;

    iIteratWithNoImprove=0;
	//main loop...
//	bool bRemoveSets=false;
//	int iNrOfRemoves=0;


while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{

//	if (iIteratWithNoImprove % 100==0) cout<<iIteratWithNoImprove<<endl; 
		    
			iTempSet=-1;

		    iIterationNumber++;

			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=random_range(0, 100);

			//cout<<"rand nr..."<<intRandom<<endl;
			if (intRandom< intRandomProb)
			{  	
				//RANDOM NOISE
				if (randomOption==1)
				{
                   if (this->isSolutionTabu(iTempSet)==0)
				   {	   
						//Select randomly the set to be removed or added
		  				iTempSet=random_range(0, iNrOfSets-1);
					
					
						/*
						if (iSelectedSets[iTempSet]==1)
						{
							//do move
							DoMove(iTempSet,-1);
						}
						else
						{
							//do move
							if (iNumberOfSelectedSets<uperBound) DoMove(iTempSet,1);
						}
		
						lNumberOfEval++;
				
						if (dFitness<fBestFitness) 
						{
							fBestFitness=dFitness;bSolutionImproved=true;
						}
						
                        this->updateTabuList(iTempSet);
						*/
				   }
				}
		
			}
		else 
		{
			for(i=0; i< iNrOfSets; i++)   
			{   dTempFitness=lLargestFitness+1; 	

						if (iNumberOfSelectedSets<uperBound)
						{
						   if (iSelectedSets[i]==1) 
						      dTempFitness=this->calculateFitnessFromMove(i, -1);
						   else  
						   {	
							   if (iConnectedSets[i]==iIterationNumber-1 || bFinalPhase==false)
							   	{ dTempFitness=this->calculateFitnessFromMove(i,1);}
						       
						   }
						}
						else
						{
							if (iSelectedSets[i]==1)
							dTempFitness=this->calculateFitnessFromMove(i, -1);
						}

				lNumberOfEval++;
				if (isSolutionTabu(i)==0)   
				{
					if (dBestIterationFitness>dTempFitness)
					{
						dBestIterationFitness=dTempFitness;
						iTempSet=i;

						if (dTempFitness<fBestFitness) 
						{fBestFitness=dTempFitness;bSolutionImproved=true;}
					}
	
					else if (dBestIterationFitness==dTempFitness )
					{	
                       /*
						// use of long term memory considering movement of each set...
						// diversification...
						else if (iIteratWithNoImprove>10)
						{
							if (lFrequencyBasedMemory[i]<lFrequencyBasedMemory[iTempSet])
							{	iTempSet=i;
								 

							
							    if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;}
							}
						}

                         */
						if (bTSDeterministic==false)
						{
							intRandom=(rand() % 10 )+1;
							if (intRandom<2)  
							{
								iTempSet=i;
								if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;} 
							}
						}

					}
				}

				else if (isSolutionTabu(i)==1)   
				{
					//aspiration criteria...
					if (dTempFitness<fBestFitness)
					{
						dBestIterationFitness=dTempFitness;
				
						iTempSet=i;
						fBestFitness=dTempFitness; bSolutionImproved=true;
				  }

				}
						
			}
	
		}
		
            //set new fitness 
		    if (iTempSet>=0)
			{	
					if (iSelectedSets[iTempSet]==1) 
					{  if (bLegalSolutionFound==true) 
						bFinalPhase=true;
					
					   // register sets which 
						DoMove(iTempSet,-1);
						
						/////////////////////////
						// mark sets which have share the same variables with the removed sets (only uncov var) 
                        int itempVar;
						for(int k=iSetsVarRange[2*iTempSet];k<=iSetsVarRange[2*iTempSet+1];k++)
						{   itempVar=iSetsVar[k];
							if (lProblem[iTempSet+itempVar*iNrOfSets]==1 && iCoveredVar[itempVar]==0) 
							{
								int iTSet2;
								for(int m=iVarSetsRange[2*itempVar];m<=iVarSetsRange[2*itempVar+1];m++)
								{   
									iTSet2=iVarSets[m];
									if (lProblem[iTSet2+itempVar*iNrOfSets]==1) iConnectedSets[iTSet2]=iIterationNumber;
								}
							}

						}

					}
					else {DoMove(iTempSet,1);bFinalPhase=false;}
				
			}
            
	

            //cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<"    Fitness: "<<dFitness<<endl; 
			
			if (fFitnessOfBestSol>dFitness && (iNrOfVar-iNrOfCoveredVar)==0)
			{  
				bLegalSolutionFound=true;
//				iCheckSolution();
				for(i=0;i<iNrOfSets;i++) bestSetSolution[i]=iSelectedSets[i];
				fFitnessOfBestSol=dFitness;
				fBestFitness=dFitness;
	
				tempSolSetsNr=iNumberOfSelectedSets;
				tempBestSolNrCovVar=iNrOfCoveredVar;
				uperBound=iNumberOfSelectedSets-1;

     		   // cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<"  "  <<dFitness<<endl; 
		     //   cout<<endl<<endl<<fFitnessOfBestSol<<"         IMPROVEMNT in ITERATION:  "<<iIterationNumber<<endl<<endl; 

				time(&end);
		      	bestSolTime=(int)(difftime(end, start));
			}

			updateTabuList(iTempSet);
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}

    	    if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;

}

        //write the best solution...
	    for(i=0;i<iNrOfSets;i++) 
		{iSelectedSets[i]=bestSetSolution[i];
        iNumberOfSelectedSets=tempSolSetsNr;
		iNrOfCoveredVar= tempBestSolNrCovVar;
		}
		
		delete [] bestSetSolution;
		delete [] lTabuList;
		lTabuList=NULL;
       delete [] iConnectedSets;

//	    iCheckSolution();
        return bestSolTime;

}





// FUNCTIONS USED JUST FOR EXPERIMENTS...

/*
***Description***
Greedy algorithm for set cover 
  
INPUT: It uses private variables of the class...
iGreedyType -- type of greedy deterministic grredy (DET_GREEDY) 1, random greedy (RND_GREEDY) 2
and grredy which is based also on the number of elemts the set has (MAX_GREEDY) 3

OUTPUT: The result is given in private variable of class: iSelectedSets  
 

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void SetCoverLS::greedySetCover1(int iGreedyType)
{ 	
	int i,j;
     // number of variables in each set
	 int *iUVarInSetsOld;

	iCoveredVar=new int[iNrOfVar];

    if (iCoveredVar == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
 
    for (i=0;i<iNrOfVar;i++)
	{iCoveredVar[i]=0;}	

    iNrOfCoveredVar=0;

    iSelectedSets=new int[iNrOfSets];
    if (iSelectedSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
     
     for (i=0;i<iNrOfSets;i++)
	 {iSelectedSets[i]=0;}	
   
     // calculate nr of variables which are covered by each set
	 iUVarInSets=new int[iNrOfSets];
     if (iUVarInSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
     for (i=0;i<iNrOfSets;i++)
	 {iUVarInSets[i]=0;
	 }

     for (i=0;i<iNrOfSets;i++)
	 {
		 for(j=0;j<iNrOfVar;j++)
		 {
			 if (lProblem[i+j*iNrOfSets]==1)
			 {iUVarInSets[i]++;}
		 }

	 }



 // calculate nr of variables which are covered by each set
	 iUVarInSetsOld=new int[iNrOfSets];
     if (iUVarInSetsOld == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
     

     iVarInSets=new int[iNrOfSets];
     if (iVarInSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:greedySetCover");
   
	 for (i=0;i<iNrOfSets;i++)
		{iVarInSets[i]=iUVarInSets[i];}
  

 //greedy algorithm for Set Cover...
 bool bGreedyFinish=false;
 while ( bGreedyFinish==false)
 {
	 for (i=0;i<iNrOfSets;i++)
		{iUVarInSetsOld[i]=iUVarInSets[i];}
  
     	
	 // find the set with maximum number of uncovered varibales (in case of equality 
	 // break ties randomly, or apply another greedy option...

  	 int itempMax=0;
  	 int itempSet=0;
  	 for (i=0;i<iNrOfSets;i++)
	 {
		 {
			 if (iUVarInSets[i]>itempMax && iSelectedSets[i]!=1)
			 {
				 itempSet=i;itempMax=iUVarInSets[i];
			 }
             else if (iUVarInSets[i]==itempMax && iSelectedSets[i]!=1)
			 {   
				 if (iGreedyType== MAX_GREEDY)
				 {
					//favors the sets which have larger number of variables -- could be important in case of further improves with LS
					if (iVarInSets[itempSet]<iVarInSets[i])
					{itempSet=i;itempMax=iUVarInSets[i];}
				 }
				 else if (iGreedyType== RND_GREEDY)
				 {   //selest randomly the next set
					 if (random_range(0, 100)<50) {itempSet=i;itempMax=iUVarInSets[i];}
				 } 
			}
		 }

	 }
     

     if (itempMax==0 || iNrOfCoveredVar==iNrOfVar)
	 {//either all variables are covered, or there is no more sets which can cover variables
		 bGreedyFinish=true;
	 }
     else
	 {
            //add all sets which cover itempMax uncovered variables... 
			 for (int c=0;c<iNrOfSets;c++)
			 { if (iSelectedSets[c]!=1 &&  iUVarInSetsOld[c]==itempMax)
				 {itempSet=c;
					 iNumberOfSelectedSets++;
					 // Add set itempSet in the selected sets
					 iSelectedSets[itempSet]=1;
					 // update covered variables and the maximum of uncovered variables in each set 
					 for (i=0;i<iNrOfVar;i++)
					 {
						if (lProblem[itempSet+i*iNrOfSets]==1 && iCoveredVar[i]==0)
						{	  iNrOfCoveredVar++;
							  iCoveredVar[i]=1;
							  //variable is covered, update iUVarInSets ...
							  for(j=0;j<iNrOfSets;j++)
							  {
								  if (lProblem[j+i*iNrOfSets] ==1)
								  {if(iUVarInSets[j]!=0) iUVarInSets[j]--;}
							  }

						 }
					 
					 
					 }
				}
		 
			 }
		}


 
 }


 delete []  iUVarInSetsOld;
}
















/*
***Description***
Procedure based on local search (tabu search) to improve greedy algorithm for set cover 
  
INPUT: It uses private variables of the class...

OUTPUT: The result is given in private variable of class: iSelectedSets  
 

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

int SetCoverLS::localSearchSC1()
{

    time_t start, end;
    int bestSolTime=0;

	int i;
	int intRandom;
	int iTempSet;
	int iTempSet2;
	bool bSwap=false;
	int iTempSecondSet=-1;
    bool tempSwap=false;
	
	const long lLargestFitness = 100000000; 
	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	int iCycleNr=0;
	
	// needed for aspiration criteria
	double fBestFitness;
	double fFitnessOfBestSol;
	double dBestIterationFitness;
    int tempSolSetsNr;
	int tempBestSolNrCovVar;

	double dTempFitness;
	bool bSolutionImproved;

    int *bestSetSolution;
    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
    
	bool bImprovmentPhase=false;
    int *ImprovmentPhaseBestSol=NULL;
	int *ImprovmentPhaseCovVar=NULL; 
	int iImprovmentPhaseSetsNr=0;
	int iImprovmentPhaseNrCovVar=0;
	int iImprovmentNrVarCovBySingleSet=0;
	double dImprovmentPhaseFitness=0; 
     
	bool bFinalPhase=false;
    bool bLegalSolutionFound=false;
	int *iConnectedSets;
    
    time(&start);
	
    iConnectedSets=new int[iNrOfSets];
		
	if (iConnectedSets == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC1");

	//int *tempSelectedSets=new int[iNrOfSets];

    dOldBestIterationFitness=lLargestFitness+1;
	
	bestSetSolution=new int[iNrOfSets];
	if (bestSetSolution == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

	ImprovmentPhaseBestSol=new int[iNrOfSets];
	if (ImprovmentPhaseBestSol == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

    ImprovmentPhaseCovVar=new int[iNrOfVar];
	if (ImprovmentPhaseCovVar == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

	
    //calculate initial solution

	if (bInitialSolGreedy){
		greedySetCover();bLegalSolutionFound=true;
				    time(&end);
		      	bestSolTime=(int)(difftime(end, start));
		}
    else emptySetCover();
    this->calculateFitness(); 


//	if (iNumberOfSelectedSets>200 && intTabuLength!=1) intTabuLength=intTabuLength+20;

    intTabuLength=(int)(iNumberOfSelectedSets*((float)intTabuLength/100))+1;

    if (iNumberOfSelectedSets>0) uperBound=iNumberOfSelectedSets-1; else uperBound=10000 ; 

	/*
   	for(i=0;i<iNrOfSets;i++) 
	{if (iSelectedSets[i]==1) 
	   tempSelectedSets[i]=1; 
	  else tempSelectedSets[i]=0;
	}
	*/

   //print best solution
   cout<<endl<<endl<<"****Greedy SOLUTION***** Fitness: "<<dFitness<<endl<<endl;


 /*     for(i=0;i<iNrOfSets;i++)
		{cout<<" " <<iSelectedSets[i]; 
		}
*/

   cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<endl<<endl; 

   for(i=0;i<iNrOfSets;i++) bestSetSolution[i]=iSelectedSets[i];
   fFitnessOfBestSol=dFitness;
   tempSolSetsNr=iNumberOfSelectedSets;
   tempBestSolNrCovVar=iNrOfCoveredVar;

   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   lTabuList = new int[intTabuLength];
    
   if (lTabuList == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

   lFrequencyBasedMemory=new int[iNrOfSets];
   if (lFrequencyBasedMemory == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
   for(i=0;i<iNrOfSets;i++) lFrequencyBasedMemory[i]=0;
   
   fBestFitness=lLargestFitness;

    iIteratWithNoImprove=0;
	//main loop...
	bool bRemoveSets=false;
	int iNrOfRemoves=0;

while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
	        if (bRemoveSets) 
			{
				iNrOfRemoves++;
				if (iNrOfRemoves%10==0) 
				{changePenalties(); 
				cout<<"***********CHANGE OF PENALTIES***********"<<endl<<endl;
				bRemoveSets=false;}
			}
            else
			{
				if (iIteratWithNoImprove % 500000==0 && iIteratWithNoImprove !=0) 
				{	changePenalties(); 
					//cout<<"***********CHANGE OF PENALTIES***********"<<endl<<endl;
					// for(i=0;i<iNrOfSets;i++) iSelectedSets[i]=bestSetSolution[i];
					bRemoveSets=true;
				}
			}
                  
		    iTempSet=-1;
			iTempSet2=-1;
			bSwap=false;
			tempSwap=false;
		    iIterationNumber++;


            //////////////////////////////////////////////////////
            if (iIterationNumber%200>200 ) 
			{
				if (bImprovmentPhase==false)
				{//set the best solution in improvement phase...
                      for(i=0;i<iNrOfSets;i++) ImprovmentPhaseBestSol[i]=iSelectedSets[i];
					  for(i=0;i<iNrOfVar;i++) ImprovmentPhaseCovVar[i]=this->iCoveredVar[i];
					  iImprovmentPhaseSetsNr=iNumberOfSelectedSets;
	                  iImprovmentPhaseNrCovVar=iNrOfCoveredVar;
					  iImprovmentNrVarCovBySingleSet=iNrVarCovBySingleSet;
	                  dImprovmentPhaseFitness=dFitness; 
				}
				else
				{//should the best solution be replaced...
				   if (dImprovmentPhaseFitness>dFitness)
				   {  for(i=0;i<iNrOfSets;i++) ImprovmentPhaseBestSol[i]=iSelectedSets[i];
				      for(i=0;i<iNrOfVar;i++) ImprovmentPhaseCovVar[i]=this->iCoveredVar[i];
					  iImprovmentPhaseSetsNr=iNumberOfSelectedSets;
	                  iImprovmentPhaseNrCovVar=iNrOfCoveredVar;
					  iImprovmentNrVarCovBySingleSet=iNrVarCovBySingleSet;
	                  dImprovmentPhaseFitness=dFitness; 
				   }
				}
				bImprovmentPhase=true;
			}
			else 
			{   if (bImprovmentPhase==true)
				{//Register best solution of improvment phase in the current solution
                  for(i=0;i<iNrOfSets;i++) iSelectedSets[i]=ImprovmentPhaseBestSol[i];
				  for(i=0;i<iNrOfVar;i++) iCoveredVar[i]=ImprovmentPhaseCovVar[i];
					  iNumberOfSelectedSets=iImprovmentPhaseSetsNr;
	                  iNrOfCoveredVar=iImprovmentPhaseNrCovVar;
					  iNrVarCovBySingleSet=iImprovmentNrVarCovBySingleSet;
	                  dFitness=dImprovmentPhaseFitness; 
				}
                
				bImprovmentPhase=false;
			}

            /////////////////////////////////////////////////////
			//change of cycle for add or remove of sets in selected sets...
			iCycleNr++;
			if (iCycleNr==3) iCycleNr=1; 
	
			//cout<<dFitness<<endl; 
        
			
			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=random_range(0, 100);

			//cout<<"rand nr..."<<intRandom<<endl;
			if (intRandom< intRandomProb)
			{  	
				//RANDOM NOISE
				if (randomOption==1)
				{
                   if (this->isSolutionTabu(iTempSet)==0)
				   {	   
						//Select randomly the set to be removed or added
		  				iTempSet=random_range(0, iNrOfSets-1);
					
						if (iSelectedSets[iTempSet]==1)
						{
							//do move
							DoMove(iTempSet,-1);
						}
						else
						{
							//do move
							if (iNumberOfSelectedSets<uperBound) DoMove(iTempSet,1);
						}
		
						lNumberOfEval++;
				
						if (dFitness<fBestFitness) 
						{
							fBestFitness=dFitness;bSolutionImproved=true;
						}
						
                        this->updateTabuList(iTempSet); 
				   }
				}
                
				//RANDOM NOISE SWAP
				else if (randomOption==2)
				{
				    //Select randomly the set to be removed or added
		  			iTempSet=random_range(0, iNrOfSets-1);
				
					if (iSelectedSets[iTempSet]==1)
					{
                        //do move
						DoMove(iTempSet,-1);
					}
					else
					{
                        //do move
					    if (iNumberOfSelectedSets<uperBound) DoMove(iTempSet,1);
					}
	
					lNumberOfEval++;
			
					if (dFitness<fBestFitness) 
					{
						fBestFitness=dFitness;bSolutionImproved=true;
					} 
				}

				//RANDOM WALK
				else if (randomOption==3)
				{
				    
					//Select randomly the set to be removed
		  			int iTempSetToRemove=random_range(0, iNumberOfSelectedSets-1);
					int position=-1;
                    iTempSecondSet=-1;
					for(i=0; i< iNrOfSets; i++)   
					{
						if (iSelectedSets[i]==1)
						{position++;
						if (position==iTempSetToRemove) {iTempSecondSet=i; break;}
						}
					}
					
					for(i=0; i< iNrOfSets; i++)   
							{   

						        dTempFitness=lLargestFitness+1; 	
								//swap move...
								if (iSelectedSets[i]==0)
								{
									dTempFitness=this->calculateFitnessFromSwapMove(iTempSecondSet,i);
									
								}
								
								
								lNumberOfEval++;
								if (dBestIterationFitness>dTempFitness)
								{
									dBestIterationFitness=dTempFitness;
								
									iTempSet=i;
									bSwap=true; iTempSet2= iTempSecondSet;
								
									if (dTempFitness<fBestFitness) 
									{fBestFitness=dTempFitness;bSolutionImproved=true;} 
								}
					
								
	
								
					
										
							}
				
					DoSwapMove(iTempSet2,iTempSet);
					iTempSet=-1;
				}

			
             

         }
		
		else 
		{
			for(i=0; i< iNrOfSets; i++)   
			{   dTempFitness=lLargestFitness+1; 	

				     /*
					   			//SWAP MOVE-- NOT really efficient
								double dSwapBestFitness;
								double dSwapTempFitness;
								dSwapBestFitness=lLargestFitness+1;
								//swap move...
								if (iSelectedSets[i]==1)
								{
									//select the best solution from swap of this set with other sets.
									 for(int k=0;k<iNrOfSets;k++)
									 {
											 if (iSelectedSets[k]==0)
											 {
												dSwapTempFitness=this->calculateFitnessFromSwapMove(i,k);
												
												
												if(dSwapBestFitness> dSwapTempFitness) 
												{
													dSwapBestFitness= dSwapTempFitness;
													iTempSecondSet=k;		
												}

											 }
										  
									 }
								}
				       */			
				
			           
 			    	/*	
						if (iCycleNr > 1)
						{   //apply only remove
					 		if (iSelectedSets[i]==1)
							dTempFitness=this->calculateFitnessFromMove(i,-1);
						}
						else   
						{   
							//apply only add of new set...	
							if (iSelectedSets[i]==0 && 	iNumberOfSelectedSets<uperBound)
							dTempFitness=this->calculateFitnessFromMove(i, 1);
						}
				
					*/	
						

						if (iNumberOfSelectedSets<uperBound)
						{
						   if (iSelectedSets[i]==1) 
						      dTempFitness=this->calculateFitnessFromMove(i, -1);
						   else  
						   {	
							   if (iConnectedSets[i]==iIterationNumber-1 || bFinalPhase==false)
							   	{ dTempFitness=this->calculateFitnessFromMove(i,1);}
						       
						   }
						}
						else
						{
							if (iSelectedSets[i]==1)
							dTempFitness=this->calculateFitnessFromMove(i, -1);
						}
							


						/*
							tempSwap=false;
							if (dTempFitness>dSwapBestFitness)
							{
								dTempFitness=dSwapBestFitness;
								tempSwap=true;
							}
						*/
					


				lNumberOfEval++;
				if (isSolutionTabu(i)==0)   
				{
					if (dBestIterationFitness>dTempFitness)
					{
						dBestIterationFitness=dTempFitness;
					
						iTempSet=i;
						if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
						else {bSwap=false;iTempSet2=-1;}  

						if (dTempFitness<fBestFitness) 
						{fBestFitness=dTempFitness;bSolutionImproved=true;}
						//if (iIterationNumber%10==0) break;
					}
	
					else if (dBestIterationFitness==dTempFitness )
					{	
                       /*
                        if (iVarInSets[iTempSet]<iVarInSets[i] && iSelectedSets[i]==0)
						{      iTempSet=i;
								if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
								else {bSwap=false;iTempSet2=-1;}  

							
							    if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;}
						
						
						}
						else if (iVarInSets[iTempSet]>iVarInSets[i] && iSelectedSets[i]==1)
						{      
							iTempSet=i;
								if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
								else {bSwap=false;iTempSet2=-1;}  

							
							    if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;}
						
						
						}
						

						// use of long term memory considering movement of each set...
						// diversification...
						else if (iIteratWithNoImprove>1000000)
						{
							if (lFrequencyBasedMemory[i]<lFrequencyBasedMemory[iTempSet])
							{	iTempSet=i;
									if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
									else {bSwap=false;iTempSet2=-1;}  

							
							    if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;}
							}
						}

                         */
						if (bTSDeterministic==false)
						{
							intRandom=(rand() % 10 )+1;
							if (intRandom<2)  
							{
								iTempSet=i;
								if (tempSwap==true) 
								{bSwap=true; iTempSet2=iTempSecondSet;}
								else {bSwap=false;iTempSet2=-1;}  

								if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;} 
							}
						}

					}
				}

				else if (isSolutionTabu(i)==1)   
				{
					//aspiration criteria...
					if (dTempFitness<fBestFitness)
					{
						dBestIterationFitness=dTempFitness;
				
						iTempSet=i;
						if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
						else {bSwap=false;iTempSet2=-1;}  

						fBestFitness=dTempFitness; bSolutionImproved=true;
				
				  }

				}
						
			}
	
		}
		
            //set new fitness 
		    if (iTempSet>=0)
			{	if (bSwap==true)
				{
					DoSwapMove(iTempSet,iTempSet2); 
				}
				else
				{
					if (iSelectedSets[iTempSet]==1) 
					{  if (bLegalSolutionFound==true) 
						bFinalPhase=true;
						// register sets which 
						DoMove(iTempSet,-1);
						
						/////////////////////////
						// mark sets which share the same variables with the removed sets (only uncov var) 
                        int itempVar;
						for(int k=iSetsVarRange[2*iTempSet];k<=iSetsVarRange[2*iTempSet+1];k++)
						{   itempVar=iSetsVar[k];
							if (lProblem[iTempSet+itempVar*iNrOfSets]==1 && iCoveredVar[itempVar]==0) 
							{
								int iTSet2;
								for(int m=iVarSetsRange[2*itempVar];m<=iVarSetsRange[2*itempVar+1];m++)
								{   
									iTSet2=iVarSets[m];
									if (lProblem[iTSet2+itempVar*iNrOfSets]==1) iConnectedSets[iTSet2]=iIterationNumber;
								}

							}

		  
						}
					}
					else {DoMove(iTempSet,1);bFinalPhase=false;}
				}
			}
            
		//	if(iIterationNumber%60==0 || iIterationNumber%61==0)
        	cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<"    Fitness: "<<dFitness<<endl; 
			if (fFitnessOfBestSol>dFitness && (iNrOfVar-iNrOfCoveredVar)==0)
			{  
				bLegalSolutionFound=true;
//				iCheckSolution();
				for(i=0;i<iNrOfSets;i++) bestSetSolution[i]=iSelectedSets[i];
				fFitnessOfBestSol=dFitness;
				tempSolSetsNr=iNumberOfSelectedSets;
				tempBestSolNrCovVar=iNrOfCoveredVar;
				uperBound=iNumberOfSelectedSets-1;

     		    cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<"  "  <<dFitness<<endl; 
		        cout<<endl<<endl<<"IMPROVEMNT in ITERATION:  "<<iIterationNumber<<endl<<endl; 
                 
			    time(&end);
		      	bestSolTime=(int)(difftime(end, start));
		
			}



			updateTabuList(iTempSet);
			if (bSwap==true) updateTabuList(iTempSet2);
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}

    	    if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;

}

        //cout<<endl<<objBestSolution.getFitness()<<endl; 

        //write the best solution...
	    for(i=0;i<iNrOfSets;i++) 
		{iSelectedSets[i]=bestSetSolution[i];
        iNumberOfSelectedSets=tempSolSetsNr;
		iNrOfCoveredVar= tempBestSolNrCovVar;
		}
	    
		
		//print best solution
		//cout<<endl<<endl<<"****BEST SOLUTION*****  Fitness: "<<fFitnessOfBestSol<<endl<<endl;
/*
		for(i=0;i<iNrOfSets;i++)
		{cout<<" " <<bestSetSolution[i]; 
		}
  */
//	cout<<endl<<endl;
		
		delete [] bestSetSolution;
		delete [] lTabuList;
		lTabuList=NULL;
		delete [] ImprovmentPhaseBestSol;
        delete [] ImprovmentPhaseCovVar; 
        delete [] iConnectedSets;

//        cout<<endl<<endl<<"Number of Eval "<<	lNumberOfEval;

	     iCheckSolution();
         return bestSolTime;

}











/*
***Description***
Procedure based on local search (tabu search) to improve greedy algorithm for set cover 
  
INPUT: It uses private variables of the class...

OUTPUT: The result is given in private variable of class: iSelectedSets  
 

***History***
Written: (12.09.05, NM)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void SetCoverLS::localSearchSC()
{
	int i;
	int intRandom;
	int iTempSet;
	int iTempSet2;
	bool bSwap=false;
	int iTempSecondSet=-1;
    bool tempSwap=false;
					
	const long lLargestFitness = 100000000; 
	long lNumberOfEval=0;
	
	int iIteratWithNoImprove=0;
    int iIterationNumber=0;
	int iCycleNr=0;
	
	// needed for aspiration criteria
	double fBestFitness;
	double fFitnessOfBestSol;
	double dBestIterationFitness;
    int tempSolSetsNr;
	int tempBestSolNrCovVar;

	double dTempFitness;
	bool bSolutionImproved;

    int *bestSetSolution;
    double dOldBestIterationFitness;
    bool bLastIteratImproved=false;
    
	bool bImprovmentPhase=false;
    int *ImprovmentPhaseBestSol=NULL;
	int *ImprovmentPhaseCovVar=NULL; 
	int iImprovmentPhaseSetsNr=0;
	int iImprovmentPhaseNrCovVar=0;
	int iImprovmentNrVarCovBySingleSet=0;
	double dImprovmentPhaseFitness=0; 
    
    //int *tempSelectedSets=new int[iNrOfSets];

    dOldBestIterationFitness=lLargestFitness+1;


	bestSetSolution=new int[iNrOfSets];
	if (bestSetSolution == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");


	ImprovmentPhaseBestSol=new int[iNrOfSets];
	if (ImprovmentPhaseBestSol == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

    ImprovmentPhaseCovVar=new int[iNrOfVar];
	if (ImprovmentPhaseCovVar == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");


	
    //calculate initial solution

	if (bInitialSolGreedy)greedySetCover();
    else emptySetCover();
    this->calculateFitness(); 

    
	/*
   	for(i=0;i<iNrOfSets;i++) 
	{if (iSelectedSets[i]==1) 
	   tempSelectedSets[i]=1; 
	  else tempSelectedSets[i]=0;
	}
	*/

   //print best solution
//   cout<<endl<<endl<<"****Greedy SOLUTION***** Fitness: "<<dFitness<<endl<<endl;


 /*     for(i=0;i<iNrOfSets;i++)
		{cout<<" " <<iSelectedSets[i]; 
		}
*/

 //  cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<endl<<endl; 

   for(i=0;i<iNrOfSets;i++) bestSetSolution[i]=iSelectedSets[i];
   fFitnessOfBestSol=dFitness;
   tempSolSetsNr=iNumberOfSelectedSets;
   tempBestSolNrCovVar=iNrOfCoveredVar;

   if (intTabuLength==0) intTabuLength=1;
   // reserve memory for the tabu list
   lTabuList = new int[intTabuLength];
    
   if (lTabuList == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");

   lFrequencyBasedMemory=new int[iNrOfSets];
   if (lFrequencyBasedMemory == NULL)
			writeErrorMsg("Error assigning memory.", "SetCoverLS:localSearchSC");
   
   for(i=0;i<iNrOfSets;i++) lFrequencyBasedMemory[i]=0;
   
   fBestFitness=lLargestFitness;

    iIteratWithNoImprove=0;
	//main loop...
	bool bRemoveSets=false;
	int iNrOfRemoves=0;

while (lNumberOfEval<lNumberOfEvaluations && iIteratWithNoImprove<iMaxNrNotImproves) 
		{
	        if (bRemoveSets) 
			{
				iNrOfRemoves++;
				if (iNrOfRemoves%10==0) 
				{changePenalties(); 
				cout<<"***********CHANGE OF PENALTIES***********"<<endl<<endl;
				bRemoveSets=false;}
			}
            else
			{
				if (iIteratWithNoImprove % 50000==0 && iIteratWithNoImprove !=0) 
				{	changePenalties(); 
					//cout<<"***********CHANGE OF PENALTIES***********"<<endl<<endl;
					// for(i=0;i<iNrOfSets;i++) iSelectedSets[i]=bestSetSolution[i];
					bRemoveSets=true;
				}
			}
                  
		    iTempSet=-1;
			iTempSet2=-1;
			bSwap=false;
		    iIterationNumber++;


            //////////////////////////////////////////////////////
            if (iIterationNumber%200>2 ) 
			{
				if (bImprovmentPhase==false)
				{//set the best solution in improvement phase...
                      for(i=0;i<iNrOfSets;i++) ImprovmentPhaseBestSol[i]=iSelectedSets[i];
					  for(i=0;i<iNrOfVar;i++) ImprovmentPhaseCovVar[i]=this->iCoveredVar[i];
					  iImprovmentPhaseSetsNr=iNumberOfSelectedSets;
	                  iImprovmentPhaseNrCovVar=iNrOfCoveredVar;
					  iImprovmentNrVarCovBySingleSet=iNrVarCovBySingleSet;
	                  dImprovmentPhaseFitness=dFitness; 
				}
				else
				{//should the best solution be replaced...
				   if (dImprovmentPhaseFitness>dFitness)
				   {  for(i=0;i<iNrOfSets;i++) ImprovmentPhaseBestSol[i]=iSelectedSets[i];
				      for(i=0;i<iNrOfVar;i++) ImprovmentPhaseCovVar[i]=this->iCoveredVar[i];
					  iImprovmentPhaseSetsNr=iNumberOfSelectedSets;
	                  iImprovmentPhaseNrCovVar=iNrOfCoveredVar;
					  iImprovmentNrVarCovBySingleSet=iNrVarCovBySingleSet;
	                  dImprovmentPhaseFitness=dFitness; 
				   }
				}
				bImprovmentPhase=true;
			}
			else 
			{   if (bImprovmentPhase==true)
				{//Register best solution of improvment phase in the current solution
                  for(i=0;i<iNrOfSets;i++) iSelectedSets[i]=ImprovmentPhaseBestSol[i];
				  for(i=0;i<iNrOfVar;i++) iCoveredVar[i]=ImprovmentPhaseCovVar[i];
					  iNumberOfSelectedSets=iImprovmentPhaseSetsNr;
	                  iNrOfCoveredVar=iImprovmentPhaseNrCovVar;
					  iNrVarCovBySingleSet=iImprovmentNrVarCovBySingleSet;
	                  dFitness=dImprovmentPhaseFitness; 
				}
                
				bImprovmentPhase=false;
			}

            /////////////////////////////////////////////////////
			//change of cycle for add or remove of sets in selected sets...
			iCycleNr++;
			if (iCycleNr==3) iCycleNr=1; 
	
			//cout<<dFitness<<endl; 
        
			
			dBestIterationFitness=lLargestFitness;
			bSolutionImproved=false;		
		
			intRandom=random_range(0, 100);

			//cout<<"rand nr..."<<intRandom<<endl;
			if (intRandom< intRandomProb)
			{  	    
				    //Select randomly the set to be removed or added
		  			iTempSet=random_range(0, iNrOfSets-1);
				
					if (iSelectedSets[iTempSet]==1)
					{
					    //dTempFitness=this->calculateFitnessFromMove(iTempSet, -1);
                        //do move
						DoMove(iTempSet,-1);
					}
					else
					{
					    //dTempFitness=this->calculateFitnessFromMove(iTempSet,1);
                        //do move
						DoMove(iTempSet,1);
					}
			       



					lNumberOfEval++;
			
					if (dFitness<fBestFitness) 
					{
						fBestFitness=dFitness;bSolutionImproved=true;
					} 
			}
		



		else 
		{
			for(i=0; i< iNrOfSets; i++)   
			{   dTempFitness=lLargestFitness+1; 	

				if(bImprovmentPhase)
					{
						if (iCycleNr < 2)
						{   //apply only remove
					 		if (iSelectedSets[i]==1)
							dTempFitness=this->calculateFitnessFromMove(i,-1);
						}
						else   
						{   
							//apply only add of new set...	
							if (iSelectedSets[i]==0)
							dTempFitness=this->calculateFitnessFromMove(i, 1);
						}
						
						/*
						//SWAP MOVE-- NOT really efficient
                        double dSwapBestFitness;
						double dSwapTempFitness;
						dSwapBestFitness=lLargestFitness+1;
						//swap move...
						if (iSelectedSets[i]==1)
						{
							//select the best solution from swap of this set with other sets.
							 for(int k=0;k<iNrOfSets;k++)
							 {
									 if (iSelectedSets[k]==0)
									 {
									    dSwapTempFitness=this->calculateFitnessFromSwapMove(i,k);
										
										
										if(dSwapBestFitness> dSwapTempFitness) 
										{
											dSwapBestFitness= dSwapTempFitness;
											iTempSecondSet=k;		
										}

									 }
								  
							 }
						}


							tempSwap=false;
							if (dTempFitness>dSwapBestFitness)
							{
								dTempFitness=dSwapBestFitness;
								tempSwap=true;
							}
					
							*/
                    } 
					else
							{
						  
						        /*
						        //SWAP MOVE-- NOT really efficient
								double dSwapBestFitness;
								double dSwapTempFitness;
								dSwapBestFitness=lLargestFitness+1;
								//swap move...
								if (iSelectedSets[i]==1)
								{
									//select the best solution from swap of this set with other sets.
									 for(int k=0;k<iNrOfSets;k++)
									 {
											 if (iSelectedSets[k]==0)
											 {
												dSwapTempFitness=this->calculateFitnessFromSwapMove(i,k);
												
												
												if(dSwapBestFitness> dSwapTempFitness) 
												{
													dSwapBestFitness= dSwapTempFitness;
													iTempSecondSet=k;		
												}

											 }
										  
									 }
								}

							
						*/
						
						   if (iSelectedSets[i]==1)
								dTempFitness=this->calculateFitnessFromMove(i, -1);
							else dTempFitness=this->calculateFitnessFromMove(i,1);
							
						
					       /*       		
							tempSwap=false;
							if (dTempFitness>dSwapBestFitness)
							{
								dTempFitness=dSwapBestFitness;
								tempSwap=true;
							}
							*/
					
					}

				lNumberOfEval++;

				if (isSolutionTabu(i)==0)   
				{
					if (dBestIterationFitness>dTempFitness)
					{
						dBestIterationFitness=dTempFitness;
					
						iTempSet=i;
						if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
						else {bSwap=false;iTempSet2=-1;}  

						if (dTempFitness<fBestFitness) 
						{fBestFitness=dTempFitness;bSolutionImproved=true;} 
					}
	
					else if (dBestIterationFitness==dTempFitness )
					{	
						// use of long term memory considering movement of each set...
						// diversification...
						if (iIteratWithNoImprove>10)
						{
							if (lFrequencyBasedMemory[i]<lFrequencyBasedMemory[iTempSet])
							{	iTempSet=i;
									if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
									else {bSwap=false;iTempSet2=-1;}  

							
							    if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;}
							}
						}
						else if (bTSDeterministic==false)
						{
							intRandom=(rand() % 10 )+1;
							if (intRandom<5)  
							{
								iTempSet=i;
								if (tempSwap==true) 
								{bSwap=true; iTempSet2=iTempSecondSet;}
								else {bSwap=false;iTempSet2=-1;}  

								if (dTempFitness<fBestFitness) {fBestFitness=dTempFitness;} 
							}
						}

					}

					
				}

				else if (isSolutionTabu(i)==1)   
				{
					//aspiration criteria...
					if (dTempFitness<fBestFitness)
					{
						dBestIterationFitness=dTempFitness;
				
						iTempSet=i;
						if (tempSwap==true) {bSwap=true; iTempSet2= iTempSecondSet;}
						else {bSwap=false;iTempSet2=-1;}  

						fBestFitness=dTempFitness; bSolutionImproved=true;
				
				  }
	             

				}
	
	
					
			}
	
		}
		
            //set new fitness 
		    if (iTempSet>=0)
			{	if (bSwap==true)
				{
					DoSwapMove(iTempSet,iTempSet2); 
				}
				else
				{
					if (iSelectedSets[iTempSet]==1) 
						DoMove(iTempSet,-1);
					else DoMove(iTempSet,1);
				}
			}
            
               cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<"    Fitness: "<<dFitness<<endl; 
      
             
			if (fFitnessOfBestSol>dFitness && (iNrOfVar-iNrOfCoveredVar)==0)
			{  
				iCheckSolution();

				for(i=0;i<iNrOfSets;i++) bestSetSolution[i]=iSelectedSets[i];
				fFitnessOfBestSol=dFitness;
				tempSolSetsNr=iNumberOfSelectedSets;
				tempBestSolNrCovVar=iNrOfCoveredVar;

        		 cout<<endl<<"UncoveredVAr:   "<<(iNrOfVar-iNrOfCoveredVar)<<"       Sets:  "<<iNumberOfSelectedSets<<"  "  <<dFitness<<endl; 
		      cout<<endl<<endl<<"IMPROVEMNT in ITERATION:  "<<iIterationNumber<<endl<<endl; 
			}

			updateTabuList(iTempSet);
			if (!bSolutionImproved){iIteratWithNoImprove++;} else {iIteratWithNoImprove=0;}
		


		     if (dOldBestIterationFitness>dBestIterationFitness) bLastIteratImproved=true;
				else bLastIteratImproved=false;
            
             dOldBestIterationFitness=dBestIterationFitness;

}

        //cout<<endl<<objBestSolution.getFitness()<<endl; 

        //write the best solution...
	    for(i=0;i<iNrOfSets;i++) 
		{iSelectedSets[i]=bestSetSolution[i];
        iNumberOfSelectedSets=tempSolSetsNr;
		iNrOfCoveredVar= tempBestSolNrCovVar;
		}
	    
		
		//print best solution
//		cout<<endl<<endl<<"****BEST SOLUTION*****  Fitness: "<<fFitnessOfBestSol<<endl<<endl;
  
  /*
		for(i=0;i<iNrOfSets;i++)
		{cout<<" " <<bestSetSolution[i]; 
		}
  */
        //cout<<endl<<endl;
		
		delete [] bestSetSolution;
		delete [] lTabuList;
		lTabuList=NULL;
		delete [] ImprovmentPhaseBestSol;
        delete [] ImprovmentPhaseCovVar; 

}

