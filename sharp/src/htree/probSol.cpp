//THis class is written by NM




#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;

#include <fstream>

#include <Globals.hpp>
#include <probSol.hpp>
#include <Hypergraph.hpp>
#include <Node.hpp>
#include <CompSet.hpp>
#include  <Hyperedge.hpp>
#include  <SetCover.hpp>
#include  <SetCoverLS.hpp>

using namespace sharp;

probSol::probSol()
{

     iNrOfNodes=0;

     iGraph=NULL;
	 Hyp=NULL;
	 bHypertreeAlg=false;

     iZeroOneGraph=NULL;

     iLargestCliqueNode=0;

     iLargestClique=0;

     iNrOfNeighboors=NULL;
     iTempNrOfNeighboors=NULL;
     iElimOrdering=NULL;
     iNodePosInOrdering=NULL;
	 bNodeEliminated=NULL;
	 iMaxCliqueNodes=NULL;


	 iNrOfNodeEdges=NULL;
	 iNodesEdges=NULL;


	iTempNodes=NULL; 
	iEdgeSelected=NULL;


}
	  
probSol::~probSol()
{ 
	
	  
	for(int i=0;i<iNrOfNodes;i++)
	{
	  delete []	iGraph[i];
	  delete [] iZeroOneGraph[i];
	  delete [] iNodesEdges[i];

	}  

     delete []	iGraph;
	 delete [] iZeroOneGraph;
	 delete [] iNrOfNeighboors;
     delete [] iTempNrOfNeighboors;
     delete [] iElimOrdering;
     delete [] iNodePosInOrdering;
	 delete [] bNodeEliminated;
     delete [] iMaxCliqueNodes; 
	 
	 delete [] iNrOfNodeEdges;
	 delete [] iNodesEdges;

	 delete [] iTempNodes; 
	 delete [] iEdgeSelected;



}



// returns the position of the nearest nodes of node iNode (nodes from neighboorhod) in the ordering _
// predcessor - node before iNode in ordering and succesor - the node after iNode in the ordering...
void probSol::nearestNodes(int iNode, int* iPrePos, int* iSucPos)
{
    int j;
	int iNeighboorNode;
	*iPrePos=0;
	*iSucPos=iNrOfNodes;
		
	    for(j=0;j<iTempNrOfNeighboors[iNode];j++)
		{
			iNeighboorNode=iGraph[iNode][j];

			if (bNodeEliminated[iNeighboorNode]==false)
			{
                // which is the position of this node in ordering...
                if (iNodePosInOrdering[iNeighboorNode]< *iSucPos) 
				{*iSucPos= iNodePosInOrdering[iNeighboorNode];}
			}
			else if (bNodeEliminated[iNeighboorNode]==true)
			{
                // which is the position of this node in ordering...
                if (iNodePosInOrdering[iNeighboorNode]> *iPrePos) 
				{*iPrePos= iNodePosInOrdering[iNeighboorNode];}

			}



		}


}





//Calculates fitness of current solution based on move of node in another position...    
double probSol::calculateFitnessFromMove(int iNode, int iPosition)
{
    double dTempFitness, iOldPosition;
	int i;
    //position of each node in ordering ...
    unsigned short int *iTNodePosInOrdering;

   	unsigned short int *iTCliqueSizes;


    // Elimination ordering 
	unsigned short int *iTElimOrdering;

	double fTFitness;
	int iTLargestClique;
	int iTLargestCliqueNode;


    iTNodePosInOrdering=new unsigned short int[iNrOfNodes];
    iTCliqueSizes =new unsigned short int[iNrOfNodes];
	iTElimOrdering=new unsigned short int[iNrOfNodes];

	for(i =0;i<iNrOfNodes;i++)
	{
      iTNodePosInOrdering[i]=iNodePosInOrdering[i];
      iTCliqueSizes[i] =iCliqueSizes[i];
      iTElimOrdering[i]=iElimOrdering[i];
	}
    
    fTFitness=fFitness;
	iTLargestClique=iLargestClique;
	iTLargestCliqueNode=iLargestCliqueNode;


	iOldPosition=iNodePosInOrdering[iNode];

	DoMove(iNode, iPosition);
    
    dTempFitness=fFitness;
    
	//inverse move...
 //   DoMove(iNode, iOldPosition);
   

//reset move...
	for(i =0;i<iNrOfNodes;i++)
	{
      iNodePosInOrdering[i]=iTNodePosInOrdering[i];
      iCliqueSizes[i] =iTCliqueSizes[i];
      iElimOrdering[i]=iTElimOrdering[i];
	}

	fFitness=fTFitness;
	iLargestClique=iTLargestClique;
	iLargestCliqueNode=iTLargestCliqueNode;

	
     delete [] iTNodePosInOrdering;
	 delete [] iTCliqueSizes;
     delete [] iTElimOrdering;
     
	return dTempFitness;
 
}
 


//Calculates fitness of current solution based on move of node in another position...    
double probSol::calculateFitnessFromSwap(int iNode, int iPosition)
{
    
	double dTempFitness;
	int i;

   	unsigned short int *iTCliqueSizes;
	unsigned short int *iTMaxCliqueNodes;


	double fTFitness;
	int iTLargestClique;
	int iTLargestCliqueNode;
	int iTNrNodesLargestClique;

	int iTempNode1=iElimOrdering[iPosition] ;
    int iTempPos1=iNodePosInOrdering[iNode] ;

    iTCliqueSizes =new unsigned short int[iNrOfNodes];
	iTMaxCliqueNodes=new unsigned short int[iNrNodesLargestClique];


	for(i =0;i<iNrOfNodes;i++)
	{
      iTCliqueSizes[i] =iCliqueSizes[i];
  	}
    
  
	for(i =0;i<iNrNodesLargestClique;i++)
	{
      iTMaxCliqueNodes[i] =iMaxCliqueNodes[i];
  	}
  
	
	fTFitness=fFitness;
	iTLargestClique=iLargestClique;
	iTLargestCliqueNode=iLargestCliqueNode;
	iTNrNodesLargestClique=iNrNodesLargestClique;


	Swap(iNode, iPosition);
 
    dTempFitness=fFitness;
  
    // reset values to the state before the swap...
    iElimOrdering[iTempPos1]=iNode; 
	iNodePosInOrdering[iTempNode1]=iPosition;
    
	iElimOrdering[iPosition]=iTempNode1;
	iNodePosInOrdering[iNode]=iTempPos1;
		
	
	fFitness=fTFitness;
	iLargestClique=iTLargestClique;
	iLargestCliqueNode=iTLargestCliqueNode;
	iNrNodesLargestClique=iTNrNodesLargestClique;


	for(i =0;i<iNrOfNodes;i++)
	{
      iCliqueSizes[i] =iTCliqueSizes[i];
 	}

  
	for(i =0;i<iNrNodesLargestClique;i++)
	{
      iMaxCliqueNodes[i] =iTMaxCliqueNodes[i];
  	}
  
	
	delete [] iTCliqueSizes;
	delete [] iTMaxCliqueNodes;
    return dTempFitness;
 
}


/*
//swaps Inode with the node in the iPosition
void probSol::Swap(int iNode, int iPosition)
{
    int iTempNode1=this->iElimOrdering[iPosition] ;
    int iTempPos1=this->iNodePosInOrdering[ iNode] ;
	DoMove(iNode, iPosition);
    DoMove( iTempNode1,iTempPos1);

}

*/


//swaps Inode with the node in the iPosition
void probSol::doubleSwap(int iNode, int iPosition, int iNode2, int iPosition2)
{
    int iLowestPos=0;
	int iHighestPosition=iNrOfNodes-1;
	
    int iTempNode1;
	int iTempPos1;

	iTempNode1=iElimOrdering[iPosition] ;
    iTempPos1=iNodePosInOrdering[ iNode] ;

	iElimOrdering[iPosition]=iNode; 
	iNodePosInOrdering[iNode]=iPosition;
    
	iElimOrdering[iTempPos1]=iTempNode1;
	iNodePosInOrdering[iTempNode1]=iTempPos1;
    
    
    // check this ...
	if (iPosition<=iTempPos1)
	{
		iLowestPos=iPosition;
		iHighestPosition=iTempPos1;

	}
	else
	{
		iLowestPos=iTempPos1;
		iHighestPosition=iPosition;
	}
    

   
	//second swapp...
	iTempNode1=iElimOrdering[iPosition2] ;
    iTempPos1=iNodePosInOrdering[iNode2] ;

	iElimOrdering[iPosition2]=iNode2; 
	iNodePosInOrdering[iNode2]=iPosition2;
    
	iElimOrdering[iTempPos1]=iTempNode1;
	iNodePosInOrdering[iTempNode1]=iTempPos1;
   

     // check this ...
	if (iPosition2<=iTempPos1)
	{
		if (iPosition2<iLowestPos) 	iLowestPos=iPosition2;
		if (iTempPos1> iHighestPosition) iHighestPosition=iTempPos1;

	}
	else
	{
		if (iTempPos1<iLowestPos) 	iLowestPos=iTempPos1;
		if (iPosition2> iHighestPosition) iHighestPosition=iPosition2;

	}
    

	calculateFitness(iLowestPos,iHighestPosition );

}



//swaps Inode with the node in the iPosition
void probSol::Swap(int iNode, int iPosition)
{
    int iLowestPos;
	int iHighestPosition;
	
	int iTempNode1=iElimOrdering[iPosition] ;
    int iTempPos1=iNodePosInOrdering[ iNode] ;

	iElimOrdering[iPosition]=iNode; 
	iNodePosInOrdering[iNode]=iPosition;
    
	iElimOrdering[iTempPos1]=iTempNode1;
	iNodePosInOrdering[iTempNode1]=iTempPos1;
    
	if (iPosition<=iTempPos1)
	{
		iLowestPos=iPosition;
		iHighestPosition=iTempPos1;

	}
	else
	{
		iLowestPos=iTempPos1;
		iHighestPosition=iPosition;
	}


	calculateFitness(iLowestPos,iHighestPosition );


}





//moves node iNode in iPosition in the node elimination ordering...
void probSol::DoMove(int iNode, int iPosition)
{
   int iOldPosition;
   int i,k;
   int temp;
   unsigned short int* tempElim=new unsigned short int[iNrOfNodes];

   iOldPosition=iNodePosInOrdering[iNode];
   
   for(i=0;i<iNrOfNodes;i++) tempElim[i]=iElimOrdering[i];
   
   k=0;
   
	if (iPosition!=iOldPosition)
	{
 
	   for(i=0;i<iNrOfNodes;i++)
	   { 
		   if (tempElim[k]==iNode)
		   {
			   i--;
		   }
		   else
		   {
			  if (i==iPosition) 
			  {iElimOrdering[i]=iNode; k--;}
			  else    
			   iElimOrdering[i]=tempElim[k];
			  
			  temp=iElimOrdering[i];
			  iNodePosInOrdering[temp]=i;
		   }

		   k++;
	   }
   
	}
   
	calculateFitness();

	delete [] tempElim;
}



//generates randomly an initial solution
void probSol::randomIntialSolution()
{

   	int i;
	
    // for now the ordering
	for(i=0;i<iNrOfNodes;i++)
	{
		iElimOrdering[i]=i;
		iNodePosInOrdering[i]=i;

	}
	
 }







//sets new ordering...
void probSol::setNewOrdering(unsigned short int* iNewOrdering)
{
   int i;
   int iNode;
   for(i=0;i<iNrOfNodes;i++)
   {
		iElimOrdering[i]= iNewOrdering[i];
        iNode=iElimOrdering[i];
		iNodePosInOrdering[iNode]=i; 
	}

   calculateFitness(); 

}



//performs swaping of randomMoveNr nodes, which are selected randomly  

void probSol::performNRandSwaps(int randomMoveNr)
{
	int i,iFirstNode,iFirstPos, iSecondNode, iSecondPosition;
    // int iMaxPosition, iMinPosition;     


	for(i=1;i<randomMoveNr;i++)
	{
         iFirstNode=(rand() % (iNrOfNodes));
		 iFirstPos=	iNodePosInOrdering[iFirstNode];
         
         iSecondNode=(rand() % (iNrOfNodes));
		 iSecondPosition=iNodePosInOrdering[iSecondNode];
         
         iElimOrdering[iSecondPosition]=iFirstNode; 
		 iNodePosInOrdering[iFirstNode]=iSecondPosition;
    
		 iElimOrdering[iFirstPos]=iSecondNode;
		 iNodePosInOrdering[iSecondNode]=iFirstPos;
	}


	calculateFitness();
}




//performs moving of randomMoveNr nodes, which are selected randomly  

void probSol::performNRandMoves(int randomMoveNr)
{
	int i,iTempNode,iTempPos;
         

	for(i=1;i<randomMoveNr;i++)
	{
         iTempNode=(rand() % (iNrOfNodes));
		 iTempPos=(rand() % (iNrOfNodes));
         
		 DoMove(iTempNode,iTempPos); 

	}

}



void probSol::destroyPart(int randomMoveNr)
{
	int i,iTempNode,iTempPos;
         
    
    iTempNode=(rand() % (iNrOfNodes));
	for(i=1;i<randomMoveNr;i++)
	{
		 
		 iTempPos=(rand() % (iNrOfNodes));
         
		 swap(iTempNode,iTempPos); 
         iTempNode++;
		 if (iTempNode==iNrOfNodes) break; 
	}

}

 
//swap largest cliques with some other nodes
void probSol::moveLargestCliques(int NumberOfMoves)
{

	int N =0; 
    int i,iFirstNode,iFirstPos, iSecondNode, iSecondPosition;

    for(i=0;i<iNrNodesLargestClique;i++)
		{
			    
				 iFirstNode=iMaxCliqueNodes[i];
				 iFirstPos=	iNodePosInOrdering[iFirstNode];
         
				 iSecondNode=(rand() % (iNrOfNodes));
				 iSecondPosition=iNodePosInOrdering[iSecondNode];
         
				 iElimOrdering[iSecondPosition]=iFirstNode; 
				 iNodePosInOrdering[iFirstNode]=iSecondPosition;
    
				 iElimOrdering[iFirstPos]=iSecondNode;
				 iNodePosInOrdering[iSecondNode]=iFirstPos;

				 N++;

        		 if (N>NumberOfMoves) break;
	
	}


	calculateFitness();
}


/*
//JUST TEST OF FITBESS FUNCTION
//Fitness is calculated based on the assumption for the upper bound on the hypertree decomposition ... 
// calculates fintess based on the given ordering...
void probSol::calculateFitness()
{ 
    int i,j,k,m,e;
	int iNode;
	int iNeighboorNode;
	int iNextNode;
	int iCountN;
	int iCountNHyp;
	int iLargestPosition;
	int iTemp;
	int iClique;
    int intRandom;
	int iTempNrOfNodes=0;
	int iTempNrOfEdges=0;
    
	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;

	
    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
...	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

    // current state for number of edges for each node, and the common edges between two nodes ...
	for (i=0;i<iNrOfNodes;i++) 
	{
		for (j=0;j<iNrOfNodes;j++) 
		{
			iTempNrCommonHyperedges[i][j]=iNrCommonHyperedges[i][j];
		}

        iTempNrOfHyperdeges[i]=iNrOfHyperdeges[i];
		
	}
	

 	for (i=0;i<iNrOfNodes-1;i++)
	{

		if (iClique>iNrOfNodes-i) 
	    	   break;	
			

		 iNode=iElimOrdering[i];


		iCountN=0;
		iCountNHyp=0;
		iLargestPosition=iNrOfNodes;
		
		//find the number of neighboors (not yet eliminated) of node and the next neighborhood node 
		// in the elimination ordering
        for( j=0;j<iTempNrOfNeighboors[iNode];j++)
		{
			iNeighboorNode=iGraph[iNode][j];

			if (bNodeEliminated[iNeighboorNode]==false)
			{
                iCountN++;
                // which is the position of this node in ordering...
                if (iNodePosInOrdering[iNeighboorNode]< iLargestPosition) 
				{iNextNode=iNeighboorNode;iLargestPosition= iNodePosInOrdering[iNeighboorNode];}

			}

		}

        
		iCountNHyp=iTempNrOfHyperdeges[iNode];
		        

		//Add the neighboors (except iNextNode) of iNode in the neighboors of iNextNode 
        for( j=0;j<iTempNrOfNeighboors[iNode];j++)
		{	
		
			iNeighboorNode=iGraph[iNode][j];
            //Add the neighboor only if it is not eliminated yet, and if it is not already registed as e neighboor before 
			if (bNodeEliminated[iNeighboorNode]==false && iNeighboorNode != iNextNode && iZeroOneGraph[iNextNode][iNeighboorNode]!=1)
			{
			    iTemp=iTempNrOfNeighboors[iNextNode]; 	
				iGraph[iNextNode][iTemp]=iNeighboorNode;
				iTempNrOfNeighboors[iNextNode]++;
                iZeroOneGraph[iNextNode][iNeighboorNode]=1;
			}
		


			
			if (bNodeEliminated[iNeighboorNode]==false)
			{
				iTempNrOfHyperdeges[iNeighboorNode]=iTempNrOfHyperdeges[iNeighboorNode]+iTempNrOfHyperdeges[iNode]- iTempNrCommonHyperedges[iNeighboorNode][iNode];	
				iTempNrCommonHyperedges[iNeighboorNode][iNextNode]= iTempNrCommonHyperedges[iNeighboorNode][iNextNode]+iTempNrOfHyperdeges[iNode];
					iTempNrCommonHyperedges[iNextNode][iNeighboorNode]=iTempNrCommonHyperedges[iNeighboorNode][iNextNode];
			}
		
			


		}

        		
		bNodeEliminated[iNode]=true;
 
        if (iClique<iCountN) 
//		if (iClique<iCountNHyp ) 
		{
			iClique=iCountN;
            //iClique=iCountNHyp; 
			iLargestCliqueNode=iElimOrdering[i];
			iNrNodesLargestClique=1;
		}
	    else if (iClique==iCountN) 
		{
			iNrNodesLargestClique++;
			//Gives to all nodes with the same clique to be selected for the node with the largest 
			// clique... the node with largest clique can be used then in search and this 
			// makes possible to select this node randomly among the nodes with the same clique size...
			intRandom=(rand() % iNrNodesLargestClique )+1;
			if (intRandom==iNrNodesLargestClique)  
				iLargestCliqueNode=iElimOrdering[i];
		}
      
        

		//second method for calculation of fitness..
		//fFitness=fFitness+iCountN*iCountN;

		iCliqueSizes[i]=iCountNHyp;
	}
    
    //second way for calculation of fitness
    //fFitness =fFitness+iNrOfNodes*iNrOfNodes *iClique*iClique;
	fFitness =iClique;
	iLargestClique=iClique;


   //reset iZeroOneGraph.  iGraph needs not to be reseted as the information for it is stored in iNrOfNeighboors;
   for(i=0;i<iNrOfNodes;i++)
   {
	   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
	   {            
          iNode=iGraph[i][j];
          iZeroOneGraph[i][iNode]=0;
	   }

   }


   	for (i=0;i<iNrOfNodes;i++) 
	{
		bNodeEliminated[i]=false;
	}
	

	
}
 

*/


//CALCULATION OF FITNESS BASED ON SET COVERING FOR EACH CLIQUE IF GENERATEHYPERTREE is TRUE
// calculates fintess based on the given ordering...
void probSol::calculateFitnessTest(int iLowestPosition, int iHighPosition)
{ 
    int i,j,k,m,e;
	int iNode;
	int iNeighboorNode;
	int iTempNeighborNood;
	int iNextNode=0;
	int iCountN;
	int iLargestPosition;
	int iTemp;
	int iClique;
    int intRandom;
	int iTempNrOfEdges=0;
    int *iTempNodes; 
	int iNumSingleEdges=0;
	int iMaxNumSingleEdges=0;
	int iNumberOfOnes=0;
    int iMinNumberOfOnes=0;
    int iNumSingleNodes=0;
	int iMaxNumSingleNodes=0;



	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;

	iTempNodes=new int[iNrOfNodes];

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

    Hyp->resetEdgeLabels(-1);


	for (i=0;i<iNrOfNodes-1;i++)
	{
		
		iTempNrOfEdges=0;


		if (i>iHighPosition && iHighPosition !=-1)

			iCountN=iCliqueSizes[i];

		else
		{
			    if (iClique>iNrOfNodes-i) 
				    break;	
				

				iNode=iElimOrdering[i];

				iCountN=0;
				iLargestPosition=iNrOfNodes;
				
				//find the number of neighboors (not yet eliminated) of node and the next neighborhood node 
				// in the elimination ordering
				for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
						iCountN++;
						// which is the position of this node in ordering...
						if (iNodePosInOrdering[iNeighboorNode]< iLargestPosition) 
						{iNextNode=iNeighboorNode;iLargestPosition= iNodePosInOrdering[iNeighboorNode];}

     					///////////////////////////////////////////////////////////////////////////////////////////////

					}

				}



				if (bHypertreeAlg==true && (iCountN>iClique)) 
				{
					//find number of nodes in this clique that apear alone in some hyperedge. 
					iNumSingleNodes=0;
					k=0;
				    // in the elimination ordering
					for( j=0;j<iNrOfNeighboors[iNode];j++)
						{    
							
							iNeighboorNode=iGraph[iNode][j];

							if (bNodeEliminated[iNeighboorNode]==false) 
							{
								k++;
							
								e=0;
								
								for( m=0;m<iNrOfNeighboors[iNeighboorNode];m++)
									{    
										iTempNeighborNood=iGraph[iNeighboorNode][m];
										if (bNodeEliminated[iTempNeighborNood]==false) 
											e++;
										
										/*
										if (e>0) 
											break;  */
															
									}


									//if (e<1)
								//		iNumSingleNodes++;
								
								iNumSingleNodes=iNumSingleNodes+e;
							}
						
						}

					/*
					    if (k<1) 
							iNumSingleNodes++;*/

					iNumSingleNodes=iNumSingleNodes+k;
					
				}
			
				///////////////////////////////////////////////////////////////////////////////////////////
				// apply set cover, only if the clique is almost maximal clique ...
				
				/*
				if (bHypertreeAlg==true && (iCountN>iClique-iBoundForSC))
					{ 
               			iNumberOfOnes=0;
						iNumSingleEdges=0;
						
						int iCountForNode=0;	
						for(k=0;k<iTempNrOfNodes;k++)
						   {
							  iCountForNode=0;	

							  for(m=0; m < Hyp->getNode(iTempNodes[k])->getNbrOfEdges(); m++)
							  {
										
									  if (Hyp->getNode(iTempNodes[k])->getEdge(m)->getLabel() ==-10)
										{Hyp->getNode(iTempNodes[k])->getEdge(m)->setLabel(-20);
										iNumSingleEdges--;}
										else if (Hyp->getNode(iTempNodes[k])->getEdge(m)->getLabel()!=-20)
										{
											iNumSingleEdges++;
											Hyp->getNode(iTempNodes[k])->getEdge(m)->setLabel(-10);
										}
										iNumberOfOnes++;

							  
								

							  }
						   }

					
					}

				*/
				
				//////////////////////////////////////////////////////////////////////////////////////////

				//Add the neighboors (except iNextNode)  of iNode in the neighboors of iNextNode 
				for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];
					//Add the neighboor only if it is not eliminated yet, and if it is not already registed as e neighboor before 
					if (bNodeEliminated[iNeighboorNode]==false && iNeighboorNode != iNextNode && iZeroOneGraph[iNextNode][iNeighboorNode]!=1)
					{
						iTemp=iTempNrOfNeighboors[iNextNode]; 	
						iGraph[iNextNode][iTemp]=iNeighboorNode;
						iTempNrOfNeighboors[iNextNode]++;

						iZeroOneGraph[iNextNode][iNeighboorNode]=1;

					}
				}


			 bNodeEliminated[iNode]=true;
 
			 iCliqueSizes[i]=iCountN; 
			
		}
    
			if (iClique<iCountN) 
			{
				iClique=iCountN;
				iLargestCliqueNode=iElimOrdering[i];
				iNrNodesLargestClique=1;

				iMinNumberOfOnes=iNumberOfOnes;
				iMaxNumSingleEdges=iNumSingleEdges;
				iMaxNumSingleNodes=iNumSingleNodes;
			}
			else if (iClique==iCountN) 
			{
				iNrNodesLargestClique++;
				//Gives to all nodes with the same clique to be selected for the node with the largest 
				// clique... the node with largest clique can be used then in search and this 
				// makes possible to select this node randomly among the nodes with the same clique size...
				intRandom=(rand() % iNrNodesLargestClique )+1;

				if (intRandom==iNrNodesLargestClique)  
					iLargestCliqueNode=iElimOrdering[i];

   				if (iMinNumberOfOnes>iNumberOfOnes || iMinNumberOfOnes==0 )
					iMinNumberOfOnes=iNumberOfOnes;

				if (iMaxNumSingleEdges<iNumSingleEdges || iMaxNumSingleEdges==0)
					iMaxNumSingleEdges=iNumSingleEdges;
				
				if (iMaxNumSingleNodes>iNumSingleNodes || iMaxNumSingleNodes==0)
				{iMaxNumSingleNodes=iNumSingleNodes;
				  //cout<<iMaxNumSingleNodes<<endl;
				}

			
			}
		
	}

    fFitness=iClique; 
	iLargestClique=iClique;

	fFitness=fFitness-((float)(iMaxNumSingleNodes)/1000);
	
	//fFitness=fFitness+ (1/(float)iMinNumberOfOnes);
	//	fFitness=fFitness+ ((float)(iMaxNumSingleEdges)/100);
    


   //reset iZeroOneGraph.  iGraph needs not to be reseted as the information for it is stored in iNrOfNeighboors;
   for(i=0;i<iNrOfNodes;i++)
   {
	   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
	   {            
          iNode=iGraph[i][j];
          iZeroOneGraph[i][iNode]=0;
	   }

   }

   	for (i=0;i<iNrOfNodes;i++) 
	{
		bNodeEliminated[i]=false;
	}
	

	delete [] iTempNodes;

}
 

//CALCULATION OF FITNESS BASED ON SET COVERING FOR EACH CLIQUE IF GENERATEHYPERTREE is TRUE
// calculates fintess based on the given ordering...
void probSol::calculateFitness(int iLowestPosition, int iHighPosition)
{ 
    int i,j,k,m,e;
	int iNode;
	int iNeighboorNode;
	int iNextNode=0;
	int iCountN;
	int iLargestPosition;
	int iTemp;
	int iClique;
	int iTempNrOfNodes=0;
	int iTempNrOfEdges=0;
    int iHypSize =0;
	int iMaxHypSize=0;
	int iNumberOfOnes=0;
    int iTotalEdges=0;
    int iAddedEdges=0;

    ///////////////////////////
	int iBoundForSC=15;
    ////////////////////////////
	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;


    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

    if (bHypertreeAlg==true) 
	   {
		    for (i=0;i<iNrOfEdges;i++) {iEdgeSelected[i]=-1;}
	   }	
	  

	for (i=0;i<iNrOfNodes-1;i++)
	{
	   
		iTempNrOfEdges=0;
	
		if (i>iHighPosition && iHighPosition !=-1)
		{
			iCountN=iCliqueSizes[i];
			iHypSize=iCliqueSizes[i];
		}   

		else
		{
			       
			iAddedEdges=0;
			       if (iClique>iNrOfNodes-i) 
					   break;
				   
	
			       iNode=iElimOrdering[i];

	                /////////////////////////////////////////////////////////////////////////////////////////////
				   	if (bHypertreeAlg==true )
					{ 		
						if ((i<iLowestPosition && iLowestPosition!=-1)||iLowestPosition==-1)
						{	iTempNrOfNodes=1;
							
	   						iTempNodes[iTempNrOfNodes-1]=iNode;
							
							for(k=0;k<iNrOfNodeEdges[iNode] ;k++)
							{
								if (iEdgeSelected[iNodesEdges[iNode][k]] < iTotalEdges)
								{
									iTempNrOfEdges++;
									iEdgeSelected[iNodesEdges[iNode][k]] =iTotalEdges+iTempNrOfEdges-1;
							
 								}
								
							}

						}
					
					}
						
			////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////	

			iCountN=0;
			iLargestPosition=iNrOfNodes;
			
			//find the number of neighboors (not yet eliminated) of node and the next neighborhood node 
			// in the elimination ordering
			for( j=0;j<iTempNrOfNeighboors[iNode];j++)
			{
				iNeighboorNode=iGraph[iNode][j];

				if (bNodeEliminated[iNeighboorNode]==false)
				{
					/*
					if (j>iNrOfNeighboors[iNode]-1)
						iAddedEdges++;
					*/


				    iCountN++;
					// which is the position of this node in ordering...
					if (iNodePosInOrdering[iNeighboorNode]< iLargestPosition) 
					{iNextNode=iNeighboorNode;iLargestPosition= iNodePosInOrdering[iNeighboorNode];}

						
					    ////////////////////////////////////////////////////////////////////////////////////////////////
						// used only if hypertree decompostions should be generated
						if (bHypertreeAlg==true)
						{
							if ((i<iLowestPosition && iLowestPosition!=-1)||iLowestPosition==-1)
							{
								iTempNrOfNodes++;
								iTempNodes[iTempNrOfNodes-1]=iNeighboorNode;
							
								for(k=0;k<iNrOfNodeEdges[iNeighboorNode];k++)
								{
									if (iEdgeSelected[iNodesEdges[iNeighboorNode][k]] <iTotalEdges)
									{
										iTempNrOfEdges++;
										iEdgeSelected[iNodesEdges[iNeighboorNode][k]] =iTotalEdges+iTempNrOfEdges-1;
										
 									}
									
								}

							}							
							
    					}
						

					///////////////////////////////////////////////////////////////////////////////////////////////

				}

			}



		///////////////////////////////////////////////////////////////////////////////////////////
		// apply set cover, only if the clique is almost maximal clique ...
		if (bHypertreeAlg==true && (iCountN>iClique-iBoundForSC) && ((i<iLowestPosition && iLowestPosition!=-1)||iLowestPosition==-1))
			{ 
				if ((i<iLowestPosition && iLowestPosition!=-1)||iLowestPosition==-1)
					{
               			iNumberOfOnes=0;
						SetCoverLS SC;  
						
						bool *setCovMatrix;
				
						setCovMatrix = new bool[iTempNrOfNodes*iTempNrOfEdges];
						if( setCovMatrix == NULL)
							writeErrorMsg("Error assigning memory.", "probSol/calculateFitness");

					   for(k=0;k<iTempNrOfNodes;k++)
					   {
						   for(int m=0;m<iTempNrOfEdges;m++) setCovMatrix[m+k*iTempNrOfEdges]=0;
					   }

         


					   for(k=0;k<iTempNrOfNodes;k++)
					   {
							
						  for(m=0; m < iNrOfNodeEdges[iTempNodes[k]]; m++)
						  {
									e=iEdgeSelected[iNodesEdges[iTempNodes[k]][m]]-iTotalEdges;  
    								setCovMatrix[e+k*iTempNrOfEdges]=1;	

						  }
					   }

						SC.setTSParametres(1000000000,1000,0,10,false,true); 	
						SC.minimalSC(setCovMatrix,iTempNrOfEdges, iTempNrOfNodes,2);
					
						/*
						if  (iHypSize>iMaxHypSize-2) 
						   SC.minimalSC(setCovMatrix,iTempNrOfEdges, iTempNrOfNodes,1); 
						else 
							SC.minimalSC(setCovMatrix,iTempNrOfEdges, iTempNrOfNodes,2);
						*/

						//iCountN=SC.getNrOfSelectedSets(); 
						iHypSize=SC.getNrOfSelectedSets();
						delete [] setCovMatrix;
				}
			}
		    else if (bHypertreeAlg==true)
				{
					if (i<iLowestPosition && iLowestPosition !=-1)
						iHypSize=iCliqueSizes[i];	
				    else 
					{//register some hypertree width, which is smaller than the largest hypertree width ...
						iHypSize=iMaxHypSize-2; if (iHypSize<1) iHypSize=1;
					}

				}


			iTotalEdges=iTotalEdges+iTempNrOfEdges;

		//////////////////////////////////////////////////////////////////////////////////////////

			//Add the neighboors (except iNextNode)  of iNode in the neighboors of iNextNode 
			for( j=0;j<iTempNrOfNeighboors[iNode];j++)
			{	
				iNeighboorNode=iGraph[iNode][j];
				//Add the neighboor only if it is not eliminated yet, and if it is not already registed as e neighboor before 
				if (bNodeEliminated[iNeighboorNode]==false && iNeighboorNode != iNextNode && iZeroOneGraph[iNextNode][iNeighboorNode]!=1)
				{
					iTemp=iTempNrOfNeighboors[iNextNode]; 	
					iGraph[iNextNode][iTemp]=iNeighboorNode;
					iTempNrOfNeighboors[iNextNode]++;

					iZeroOneGraph[iNextNode][iNeighboorNode]=1;
					iZeroOneGraph[iNeighboorNode][iNextNode]=1;


				}
			}


		 bNodeEliminated[iNode]=true;

		 //iCountN=iTempNrOfNeighboors[iNode]-iNrOfNeighboors[iNode];
 		 //iCountN=iCountN+iAddedEdges++;
		  
 		 if (bHypertreeAlg==true) iCliqueSizes[i]=iHypSize; 
		 else iCliqueSizes[i]=iCountN;
	}
    
	
	if (bHypertreeAlg==false)
		{
			if (iClique<iCountN) 
			{
				iClique=iCountN;
				iLargestCliqueNode=iElimOrdering[i];
				iNrNodesLargestClique=1;
				iMaxCliqueNodes[iNrNodesLargestClique-1]=iElimOrdering[i];
			}
			else if (iClique==iCountN) 
			{
				iNrNodesLargestClique++;
				iMaxCliqueNodes[iNrNodesLargestClique-1]=iElimOrdering[i];
			}

			/*
				if (iMaxAddedEdges<iAddedEdges) 
				{
					iMaxAddedEdges=iAddedEdges;
					iNrNodesLargestClique++;
					iMaxCliqueNodes[iNrNodesLargestClique-1]=iElimOrdering[i];
				}
			*/		

		}
	
	else  if (bHypertreeAlg==true)
		{
			//store information for the treewidth ...
	  	   if (iClique<iCountN) iClique=iCountN;
	 
	        if (iMaxHypSize<iHypSize) 
			{
				iMaxHypSize=iHypSize;
				iLargestCliqueNode=iElimOrdering[i];
				iNrNodesLargestClique=1;
				iMaxCliqueNodes[iNrNodesLargestClique-1]=iElimOrdering[i];
			}
			else if (iMaxHypSize==iHypSize) 
			{
				iNrNodesLargestClique++;
				iMaxCliqueNodes[iNrNodesLargestClique-1]=iElimOrdering[i];

			}
    
    
		}

}




    //second way for calculation of fitness
    //fFitness =fFitness+iNrOfNodes*iNrOfNodes *iClique*iClique;
	
	if (bHypertreeAlg==true)
	{
		// prefer more the small hypertrees which have also small tree (possibly we can manage faster some solutions...)
		
		fFitness=iClique + ((float)iMaxHypSize)/100 ;
		//fFitness = iMaxHypSize + ((float)iClique)/100 ;

		iLargestClique=iMaxHypSize;

		//cout<< iClique << "     " << iMaxHypSize<<endl;
	}
	else
	{
		fFitness =iClique;
		iLargestClique=iClique;
    }




   //reset iZeroOneGraph.  iGraph needs not to be reseted as the information for it is stored in iNrOfNeighboors;
   for(i=0;i<iNrOfNodes;i++)
   {
	   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
	   {            
          iNode=iGraph[i][j];
          iZeroOneGraph[i][iNode]=0;
		  iZeroOneGraph[iNode][i]=0;
	
	   }

   		bNodeEliminated[i]=false;

   }


	

}
 




/*

//Min fill heuristic
void probSol::MinFillOrdering()
{ 
    int i,j,k,m,e,g,f;
	int iNode;
	int iNeighboorNode;
	int iSecNeighboorNode;
	int iNextNode;
	int iCountN;
	int iLargestPosition;
	int iTemp;
	int iClique;
    int intRandom;
	int iTempNrOfNodes=0;
	int iTempNrOfEdges=0;
    int iHypSize =0;
	int iMaxHypSize=0;
	int iSingleNodes=0;
	int iNumberOfOnes=0;
    int iTotalEdges=0;
    int iAddedEdges=0;
	int iMaxAddedEdges=0;
	int iNrEdgesToAdd=0;
	int iMinNrEdgesToAdd=0;
	int iNrOfSameNodes=0;

    ///////////////////////////
	int iBoundForSC=15;
    ////////////////////////////
	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;




//    srand(100);



    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

 
	for (i=0;i<iNrOfNodes;i++)
	{
	   
		////////////////////////////////////////////////////////////////
		// find the node to be eliminated based on min fill heuristics...
		iNrOfSameNodes=0;
		iMinNrEdgesToAdd=iNrOfNodes;
		for(f=0;f<iNrOfNodes;f++)
		{
			if (bNodeEliminated[f]==false)
			{
					iNrEdgesToAdd=0;
					
					for( j=0;j<iTempNrOfNeighboors[f];j++)
					{
						iNeighboorNode=iGraph[f][j];
						
						if (bNodeEliminated[iNeighboorNode]==false)
						{
							for(g=j+1;g<iTempNrOfNeighboors[f];g++)
							{
								iSecNeighboorNode=iGraph[f][g];
								if (bNodeEliminated[iSecNeighboorNode]==false) 
								{	
									if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
											iNrEdgesToAdd++;

								}
							}
						}
					}
				
					//select this node with the probability which is based in the number of nodes which add the largest number of edges 

					if (iNrEdgesToAdd<iMinNrEdgesToAdd)
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iNrEdgesToAdd;
						iNode=f;
					}
					else if (iNrEdgesToAdd==iMinNrEdgesToAdd)
					{
					
						iNrOfSameNodes++;
						if (((rand() % iNrOfSameNodes )+1)==iNrOfSameNodes)
							iNode=f;
					
					}
						
			
			}
		
			
		}
		


		iElimOrdering[i]=iNode;
		iNodePosInOrdering[iNode]=i;
	
//		if(iMinNrEdgesToAdd>0)
//			cout<<i<<"     "  << iNode<<"        "<<iMinNrEdgesToAdd<<endl;

//////////////////////////////////////////////////////////////////////////////////////////////

	//	cout<<endl<<i<<"       "<<iNode;

	//	iNode=iElimOrdering[i];

		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
			{	
				iNeighboorNode=iGraph[iNode][j];

				if (bNodeEliminated[iNeighboorNode]==false)
				{
					for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
					{
						iSecNeighboorNode=iGraph[iNode][g];
						
						if (bNodeEliminated[iSecNeighboorNode]==false && iZeroOneGraph[iSecNeighboorNode][iNeighboorNode] != 1) 
						{	
							iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
							iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
							iTempNrOfNeighboors[iSecNeighboorNode]++;

							iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
							iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
							iTempNrOfNeighboors[iNeighboorNode]++;

							iZeroOneGraph[iSecNeighboorNode][iNeighboorNode]=1;
							iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]=1;
						
						}
					}

				}
			}

	
		bNodeEliminated[iNode]=true;

	}
    


	   //reset iZeroOneGraph.  iGraph needs not to be reseted as the information for it is stored in iNrOfNeighboors;
	   for(i=0;i<iNrOfNodes;i++)
	   {
		   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
		   {            
			  iNode=iGraph[i][j];
			  iZeroOneGraph[i][iNode]=0;
			  iZeroOneGraph[iNode][i]=0;
		   }

   			bNodeEliminated[i]=false;

	   }

	   

}


*/




//MORE EFFICIENT VARIANT ...
void probSol::MinDegreeOrdering()
{ 
    int i,j,g;
	int iNode=0;
	int iNeighboorNode;
	int iSecNeighboorNode;
	int iTemp;
	int iMinNrEdgesToAdd=0;
	int iNrOfSameNodes=0;
    short int* iNrMinDegree;
    
  // srand(100);
   iNrMinDegree=new short int[iNrOfNodes];

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

    

    //determine min-fills for each node...
	for (i=0;i<iNrOfNodes;i++)
	{
			
            //set the minFill for each node
			iNrMinDegree[i]=iTempNrOfNeighboors[i];
			
		//	cout<<i<<"    "<<iTempNrOfNeighboors[i]<<"      "<<iNrMinDegree[i]<<endl;
	
	}


	

	//construct the elimination ordering

	for (i=0;i<iNrOfNodes;i++)
	{
		
		iMinNrEdgesToAdd=-1;
		iNrOfSameNodes=0;
	
		
	    // select the node with min-fill
	
		for (j=0;j<iNrOfNodes;j++)
		{
            if(bNodeEliminated[j]==false)
			{
				// ask if node is already iliminated ...

				//select this node with the probability which is based in the number of nodes which add the largest number of edges 

				iTemp=iNrMinDegree[j];

				if (iMinNrEdgesToAdd != -1)
				{
						
					if (iTemp<iMinNrEdgesToAdd)
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iTemp;
						iNode=j;
					}
					else if (iTemp==iMinNrEdgesToAdd)
					{
						//break ties randomly...
						iNrOfSameNodes++;
						if ((rand() % iNrOfSameNodes +1)==iNrOfSameNodes)
							iNode=j;

					//	cout<<(rand() % iNrOfSameNodes +1)<<endl;
					
					}
				}
				else
				{
					iNrOfSameNodes=1;
					iMinNrEdgesToAdd=iTemp;
					iNode=j;
				}
				
				
			}	
		}


      //  cout<<iNode<<"       "<<iNrMinDegree[iNode]<<endl;

		
		iElimOrdering[i]=iNode;
			
		iNodePosInOrdering[iNode]=i;


		
	 	bNodeEliminated[iNode]=true;	 

  
		  
		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
						//update the min degree
						iNrMinDegree[iNeighboorNode]--;
								
						for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
						{
							iSecNeighboorNode=iGraph[iNode][g];
							
							if (bNodeEliminated[iSecNeighboorNode]==false ) 
							{			
							
								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
								{
									
									iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
									iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
									iTempNrOfNeighboors[iSecNeighboorNode]++;



									iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
									iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
									iTempNrOfNeighboors[iNeighboorNode]++;
                                    
									

									iZeroOneGraph[iSecNeighboorNode][iNeighboorNode]=1;
									iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]=1;
									
									//update the min degree
									iNrMinDegree[iNeighboorNode]++;
									iNrMinDegree[iSecNeighboorNode]++;

									
								    
								
								}
							
							}
						}

					}
				
	
		
			}

	
		
		    

		

	}


    
	
 for(i=0;i<iNrOfNodes;i++)
	   {
		   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
		   {            
			  iNode=iGraph[i][j];
			  iZeroOneGraph[i][iNode]=0;
			  iZeroOneGraph[iNode][i]=0;
		   }

   			bNodeEliminated[i]=false;

	   }

	


delete [] iNrMinDegree;

}



 
	
    
	


//Min Fill min degree ordering together
void probSol::MinFillAndDegreeOrdering()
{ 
    int i,j,k,g;
	int iNode=0;
	int iNeighboorNode;
	int iNeighboorNode1;
	int iSecNeighboorNode;
	int iTemp;
	int iTempDegree;
	int iNrEdgesToAdd=0;
	int iMinNrEdgesToAdd=0;
	int iMinDegree=0;

	int iNrOfSameNodes=0;
    short int* iNrMinFill;
	 short int* iNrMinDegree;
    
  // srand(100);
   iNrMinFill=new short int[iNrOfNodes];
   iNrMinDegree=new short int[iNrOfNodes];

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	


    
    //determine min-fills for each node...
	for (i=0;i<iNrOfNodes;i++)
	{

		        iNrEdgesToAdd=0;
				iTemp=iTempNrOfNeighboors[i];

				
				for( j=0;j<iTemp;j++)
				{
					iNeighboorNode=iGraph[i][j];
					
						for(g=j+1;g<iTemp;g++)
						{
								iSecNeighboorNode=iGraph[i][g];

								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
									iNrEdgesToAdd++;						 

						}
				}

            //set the minFill for each node
			iNrMinFill[i]=iNrEdgesToAdd;
			
		
			iNrMinDegree[i]=iTempNrOfNeighboors[i];
	
			//	cout<<i<<"    "<<iTempNrOfNeighboors[i]<<"      "<<iNrMinFill[i]<<endl;
	
	}




	//construct the elimination ordering

	for (i=0;i<iNrOfNodes;i++)
	{
		
		iMinNrEdgesToAdd=-1;
		iNrOfSameNodes=0;
	
		
	    // select the node with min-fill
	
		for (j=0;j<iNrOfNodes;j++)
		{
            if(bNodeEliminated[j]==false)
			{
				// ask if node is already iliminated ...

				//select this node with the probability which is based in the number of nodes which add the largest number of edges 

				iTemp=iNrMinFill[j];
				iTempDegree=iNrMinDegree[i];


				if (iMinNrEdgesToAdd != -1)
				{
						
					if (iTemp<iMinNrEdgesToAdd)
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iTemp;
						iMinDegree=iTempDegree;
						iNode=j;
					}
					else if (iTemp==iMinNrEdgesToAdd)
					{
						if (iTempDegree==iMinDegree) 
						{//break ties randomly...
							iNrOfSameNodes++;
							if ((rand() % iNrOfSameNodes +1)==iNrOfSameNodes)
							iNode=j;
						}
						else if (iTempDegree<iMinDegree) 
						{
							iNode=j;
							iNrOfSameNodes=1;
							iMinDegree=iTempDegree;
						}

					//	cout<<(rand() % iNrOfSameNodes +1)<<endl;
					
					}
				}
				else
				{
					iNrOfSameNodes=1;
					iMinNrEdgesToAdd=iTemp;
					iMinDegree=iTempDegree;
					iNode=j;
				}
				
				
			}	
		}




//        cout<<iNode<<"       "<<iTempNrOfNeighboors[iNode]<<endl;

		
		iElimOrdering[i]=iNode;
			
		iNodePosInOrdering[iNode]=i;


		
	 	bNodeEliminated[iNode]=true;	 

  
		  
		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
						
						//update the min degree
						iNrMinDegree[iNeighboorNode]--;
						

						for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
						{
							iSecNeighboorNode=iGraph[iNode][g];
							
							if (bNodeEliminated[iSecNeighboorNode]==false ) 
							{	
						
								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
								{
									
									iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
									iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
									iTempNrOfNeighboors[iSecNeighboorNode]++;

									
									iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
									iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
									iTempNrOfNeighboors[iNeighboorNode]++;



										iZeroOneGraph[iSecNeighboorNode][iNeighboorNode]=1;
										iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]=1;
										
										//update the min degree
										iNrMinDegree[iNeighboorNode]++;
										iNrMinDegree[iSecNeighboorNode]++;

							
								
								    
								
								}
							
							}
						}

					}
				
	
		
			}

	
		
			//update min fills of affected nodes...
		    
		    //determine min-fills for each node...
			for(j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];
					
					if (bNodeEliminated[iNeighboorNode]==false)
					{
							iNrEdgesToAdd=0;
							
							iTemp=iTempNrOfNeighboors[iNeighboorNode];
							for( k=0;k<iTemp;k++)
							{
									iNeighboorNode1=iGraph[iNeighboorNode][k];
									if(bNodeEliminated[iNeighboorNode1]==false)
									{
										for(g=k+1;g<iTemp;g++)
										{
												iSecNeighboorNode=iGraph[iNeighboorNode][g];

												if (bNodeEliminated[iSecNeighboorNode]==false)
												{
													if (iZeroOneGraph[iNeighboorNode1][iSecNeighboorNode]==0)
													iNrEdgesToAdd++;
												}

										}
									}
								
							}

						//set the minFill for each node
						iNrMinFill[iNeighboorNode]=iNrEdgesToAdd;
					}					
				
				}
		    

	

	}




    
	
 for(i=0;i<iNrOfNodes;i++)
	   {
		   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
		   {            
			  iNode=iGraph[i][j];
			  iZeroOneGraph[i][iNode]=0;
			  iZeroOneGraph[iNode][i]=0;
		   }

   			bNodeEliminated[i]=false;

	   }

	

delete [] iNrMinFill;
delete [] iNrMinDegree;


}








//Min degree and fill together
void probSol::MinDegreeAndFillOrdering()
{ 
    int i,j,k,g;
	int iNode=0;
	int iNeighboorNode;
	int iNeighboorNode1;
	int iSecNeighboorNode;
	int iTemp;
	int iTempDegree;
	int iNrEdgesToAdd=0;
	int iMinNrEdgesToAdd=0;
	int iMinDegree=0;

	int iNrOfSameNodes=0;
    short int* iNrMinFill;
	 short int* iNrMinDegree;
    
  // srand(100);
   iNrMinFill=new short int[iNrOfNodes];
   iNrMinDegree=new short int[iNrOfNodes];

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	


    
    //determine min-fills and min degree for each node...
	for (i=0;i<iNrOfNodes;i++)
	{

		        iNrEdgesToAdd=0;
				iTemp=iTempNrOfNeighboors[i];

				
				for( j=0;j<iTemp;j++)
				{
					iNeighboorNode=iGraph[i][j];
					
						for(g=j+1;g<iTemp;g++)
						{
								iSecNeighboorNode=iGraph[i][g];

								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
									iNrEdgesToAdd++;						 

						}
				}

            //set the minFill for each node
			iNrMinFill[i]=iNrEdgesToAdd;
			
		
			iNrMinDegree[i]=iTempNrOfNeighboors[i];
	
			//	cout<<i<<"    "<<iTempNrOfNeighboors[i]<<"      "<<iNrMinFill[i]<<endl;
	
	}




	//construct the elimination ordering

	for (i=0;i<iNrOfNodes;i++)
	{
		
		iMinNrEdgesToAdd=-1;
		iNrOfSameNodes=0;
	
		
			// select the node with min degree and min-fill
		for (j=0;j<iNrOfNodes;j++)
			{
				if(bNodeEliminated[j]==false)
				{
					// ask if node is already iliminated ...

					//select this node with the probability which is based in the number of nodes which add the largest number of edges 

					iTemp=iNrMinFill[j];
					iTempDegree=iNrMinDegree[i];


					if (iMinNrEdgesToAdd != -1)
					{
							
						if (iTempDegree<iMinDegree)
						{
							iNrOfSameNodes=1;
							iMinNrEdgesToAdd=iTemp;
							iMinDegree=iTempDegree;
							iNode=j;
						}
						else if (iTemp==iMinNrEdgesToAdd && iTempDegree==iMinDegree )
						{
								iNrOfSameNodes++;
								if ((rand() % iNrOfSameNodes +1)==iNrOfSameNodes)
									iNode=j;
							
						//	cout<<(rand() % iNrOfSameNodes +1)<<endl;
						
						}
					}
					else
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iTemp;
						iMinDegree=iTempDegree;
						iNode=j;
					}
					
					
				}	
			}




//        cout<<iNode<<"       "<<iTempNrOfNeighboors[iNode]<<endl;

		
		iElimOrdering[i]=iNode;
			
		iNodePosInOrdering[iNode]=i;


		
	 	bNodeEliminated[iNode]=true;	 

  
		  
		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
						
						//update the min degree
						iNrMinDegree[iNeighboorNode]--;
						

						for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
						{
							iSecNeighboorNode=iGraph[iNode][g];
							
							if (bNodeEliminated[iSecNeighboorNode]==false ) 
							{	
						
								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
								{
									
									iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
									iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
									iTempNrOfNeighboors[iSecNeighboorNode]++;

									
									iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
									iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
									iTempNrOfNeighboors[iNeighboorNode]++;



										iZeroOneGraph[iSecNeighboorNode][iNeighboorNode]=1;
										iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]=1;
										
										//update the min degree
										iNrMinDegree[iNeighboorNode]++;
										iNrMinDegree[iSecNeighboorNode]++;

							
								
								    
								
								}
							
							}
						}

					}
				
	
		
			}

	
		
			//update min fills of affected nodes...
		    
		    //determine min-fills for each node...
			for(j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];
					
					if (bNodeEliminated[iNeighboorNode]==false)
					{
							iNrEdgesToAdd=0;
							
							iTemp=iTempNrOfNeighboors[iNeighboorNode];
							for( k=0;k<iTemp;k++)
							{
									iNeighboorNode1=iGraph[iNeighboorNode][k];
									if(bNodeEliminated[iNeighboorNode1]==false)
									{
										for(g=k+1;g<iTemp;g++)
										{
												iSecNeighboorNode=iGraph[iNeighboorNode][g];

												if (bNodeEliminated[iSecNeighboorNode]==false)
												{
													if (iZeroOneGraph[iNeighboorNode1][iSecNeighboorNode]==0)
													iNrEdgesToAdd++;
												}

										}
									}
								
							}

						//set the minFill for each node
						iNrMinFill[iNeighboorNode]=iNrEdgesToAdd;
					}					
				
				}
		    

	

	}




    
	
 for(i=0;i<iNrOfNodes;i++)
	   {
		   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
		   {            
			  iNode=iGraph[i][j];
			  iZeroOneGraph[i][iNode]=0;
			  iZeroOneGraph[iNode][i]=0;
		   }

   			bNodeEliminated[i]=false;

	   }

	

delete [] iNrMinFill;
delete [] iNrMinDegree;


}







//MORE EFFICIENT VARIANT ...
void probSol::MinFillOrdering()
{ 
    int i,j,g,z;
	int iNode=0;
	int iNeighboorNode;
	int iSecNeighboorNode;
	int iTemp;
	int iNrEdgesToAdd=0;
	int iMinNrEdgesToAdd=0;
	int iNrOfSameNodes=0;
    short int* iNrMinFill;
    
  // srand(100);
   iNrMinFill=new short int[iNrOfNodes];

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	
   

    //determine min-fills for each node...
	for (i=0;i<iNrOfNodes;i++)
	{

		        iNrEdgesToAdd=0;
				iTemp=iTempNrOfNeighboors[i];

				
				for( j=0;j<iTemp;j++)
				{
					iNeighboorNode=iGraph[i][j];
					
						for(g=j+1;g<iTemp;g++)
						{
								iSecNeighboorNode=iGraph[i][g];

								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
									iNrEdgesToAdd++;						 

						}
				}

            //set the minFill for each node
			iNrMinFill[i]=iNrEdgesToAdd;
			
		//	cout<<i<<"    "<<iTempNrOfNeighboors[i]<<"      "<<iNrMinFill[i]<<endl;
	
	}


	

	//construct the elimination ordering

	for (i=0;i<iNrOfNodes;i++)
	{
		
		iMinNrEdgesToAdd=-1;
		iNrOfSameNodes=0;
	
		
	    // select the node with min-fill
	
		for (j=0;j<iNrOfNodes;j++)
		{
            if(bNodeEliminated[j]==false)
			{
				// ask if node is already eliminated ...

				//select this node with the probability which is based in the number of nodes which add the largest number of edges 

				iTemp=iNrMinFill[j];

				if (iMinNrEdgesToAdd != -1)
				{
						
					if (iTemp<iMinNrEdgesToAdd)
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iTemp;
						iNode=j;
					}
					else if (iTemp==iMinNrEdgesToAdd)
					{
						//break ties randomly...
						iNrOfSameNodes++;
						if ((rand() % iNrOfSameNodes +1)==iNrOfSameNodes)
							iNode=j;

					//	cout<<(rand() % iNrOfSameNodes +1)<<endl;
					
					}
				}
				else
				{
					iNrOfSameNodes=1;
					iMinNrEdgesToAdd=iTemp;
					iNode=j;
				}
				
				
			}	
		}


        
//        cout<<iNode<<"       "<<iMinNrEdgesToAdd<<endl;
		
	
		iElimOrdering[i]=iNode;
			
		iNodePosInOrdering[iNode]=i;

		
	 	bNodeEliminated[iNode]=true;	 
  

		//-------------------------------------------------------
		//update the min fill of iNeighboorNode after elimination of the iNode
			
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
		{	
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
				
						//---------------------------------------------------------------------
						//update the min fill of iNeighboorNode after elimination of the iNode
						for(g=0;g<iTempNrOfNeighboors[iNeighboorNode];g++)
						{	
							iSecNeighboorNode=iGraph[iNeighboorNode][g];
							if(bNodeEliminated[iSecNeighboorNode]==false )
							{
								if (iZeroOneGraph[iNode][iSecNeighboorNode]==0)
									iNrMinFill[iNeighboorNode]--;

						
									// min fill can also be decreased if we have two neighbors of iNode that were not connected previously, because they will be connected later
									if (iZeroOneGraph[iNode][iSecNeighboorNode]==1)
									{
										for (z=g+1;z<iTempNrOfNeighboors[iNeighboorNode];z++)
										{
											iTemp=iGraph[iNeighboorNode][z]; 
											if (bNodeEliminated[iTemp]==false && iZeroOneGraph[iTemp][iNode]==1)
											{
												if (iZeroOneGraph[iTemp][iSecNeighboorNode]==0)
													iNrMinFill[iNeighboorNode]--;


											}
										}
																
									}
								
							}
						}
					}

		}

		//----------------------------------------------------------------------


		  
		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
				/*
						//---------------------------------------------------------------------
						//update the min fill of iNeighboorNode after elimination of the iNode
						for(g=0;g<iTempNrOfNeighboors[iNeighboorNode];g++)
						{	
							iSecNeighboorNode=iGraph[iNeighboorNode][g];
							if(bNodeEliminated[iSecNeighboorNode]==false )
							{
								if (iZeroOneGraph[iNode][iSecNeighboorNode]==0)
									iNrMinFill[iNeighboorNode]--;

							}

						}
						//----------------------------------------------------------------------
                      */
		
						for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
						{
							iSecNeighboorNode=iGraph[iNode][g];
							
							if (bNodeEliminated[iSecNeighboorNode]==false ) 
							{	
						
								if (iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]==0)
								{
									
									iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
									iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
									iTempNrOfNeighboors[iSecNeighboorNode]++;

									
									iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
									iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
									iTempNrOfNeighboors[iNeighboorNode]++;


    								iZeroOneGraph[iSecNeighboorNode][iNeighboorNode]=1;
									iZeroOneGraph[iNeighboorNode][iSecNeighboorNode]=1;



									
									//-------------------------------------------------
									//update min fills due to adding of the new edge (min fills are increased)
									for (z=0;z<iTempNrOfNeighboors[iNeighboorNode]-1;z++)
									{
										iTemp=iGraph[iNeighboorNode][z];
										if (bNodeEliminated[iTemp]==false)
										{
											if (iZeroOneGraph[iSecNeighboorNode][iTemp]==0 && iZeroOneGraph[iNode][iTemp]==0)
													iNrMinFill[iNeighboorNode]++;


										}
									
									}

									for (z=0;z<iTempNrOfNeighboors[iSecNeighboorNode]-1;z++)
									{
										iTemp=iGraph[iSecNeighboorNode][z];
										if (bNodeEliminated[iTemp]==false)
										{
											if (iZeroOneGraph[iNeighboorNode][iTemp]==0 && iZeroOneGraph[iNode][iTemp]==0)
													iNrMinFill[iSecNeighboorNode]++;


										}
									
									}


									//update min fill in case when the two nodes share same neighbor (which was not a neighboor of iNode)
									// in that case the min fill of that node should be decreased, beacuse the two nodes are connected here
									for (z=0;z<iTempNrOfNeighboors[iNeighboorNode]-1;z++)
									{
										iTemp=iGraph[iNeighboorNode][z];
										if (bNodeEliminated[iTemp]==false)
										{
											if (iZeroOneGraph[iSecNeighboorNode][iTemp]==1 && iZeroOneGraph[iNode][iTemp]==0)
												iNrMinFill[iTemp]--;


										}
									
									}





									//-------------------------------------------------
											
								    
								
								}
							
							}
						}

					}
				
	
		
			}

	



			
				
			/*
				//update min fills of affected nodes...
		    
		    
					
				//determine min-fills for each node...
			//	for(j=0;j<iTempNrOfNeighboors[iNode];j++)
			for(iNeighboorNode=0;iNeighboorNode<iNrOfNodes;iNeighboorNode++)
				{	
				//		iNeighboorNode=iGraph[iNode][j];
						
						if (bNodeEliminated[iNeighboorNode]==false)
						{
								iNrEdgesToAdd=0;
								
								iTemp=iTempNrOfNeighboors[iNeighboorNode];
								for( k=0;k<iTemp;k++)
								{
										iNeighboorNode1=iGraph[iNeighboorNode][k];
										if(bNodeEliminated[iNeighboorNode1]==false)
										{
											for(g=k+1;g<iTemp;g++)
											{
													iSecNeighboorNode=iGraph[iNeighboorNode][g];

													if (bNodeEliminated[iSecNeighboorNode]==false)
													{
														if (iZeroOneGraph[iNeighboorNode1][iSecNeighboorNode]==0)
														iNrEdgesToAdd++;
													}

											}
										}
									
								}

							//set the minFill for each node
							iNrMinFill[iNeighboorNode]=iNrEdgesToAdd;
						}					
					
					}


				*/		


	

	}


    
	
 for(i=0;i<iNrOfNodes;i++)
	   {
		   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
		   {            
			  iNode=iGraph[i][j];
			  iZeroOneGraph[i][iNode]=0;
			  iZeroOneGraph[iNode][i]=0;
		   }

   			bNodeEliminated[i]=false;

	   }

	

delete [] iNrMinFill;



}





//Min fill order based in other data structures (NM: 18.12.2010)
void probSol::NewMinFillOrder()
	{
	
    int i,j,k,m,g,f;
	int iNode=0;
	int iNeighboorNode;
	int iNeighboorNode1;
	int iSecNeighboorNode;
	int iTemp;
	int iTemp1;
	int iNrEdgesToAdd=0;
	int iMinNrEdgesToAdd=0;
	int iNrOfSameNodes=0;
    short int* iNrMinFill;
	short int* iNrLastNeighbors;
    
  // srand(100);
   iNrMinFill=new short int[iNrOfNodes];
   iNrLastNeighbors=new short int[iNrOfNodes]; 

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

    

    //determine min-fills for each node...
	for (i=0;i<iNrOfNodes;i++)
	{

		        iNrEdgesToAdd=0;
				iTemp=iTempNrOfNeighboors[i];

				
				for( j=0;j<iTemp;j++)
				{
					iNeighboorNode=iGraph[i][j];
					
						for(g=j+1;g<iTemp;g++)
						{
								iSecNeighboorNode=iGraph[i][g];

								if (!bAreNodesConnected(iNeighboorNode, iSecNeighboorNode))
									iNrEdgesToAdd++;						 

						}
				}

            //set the minFill for each node
			iNrMinFill[i]=iNrEdgesToAdd;
			
		//	cout<<i<<"    "<<iTempNrOfNeighboors[i]<<"      "<<iNrMinFill[i]<<endl;
	
	}


	

	//construct the elimination ordering

	for (i=0;i<iNrOfNodes;i++)
	{
		
		iMinNrEdgesToAdd=-1;
		iNrOfSameNodes=0;
	
		
	    // select the node with min-fill
	
		for (j=0;j<iNrOfNodes;j++)
		{
            // ask if node is already eliminated ...

			if(bNodeEliminated[j]==false)
			{
				
				//select this node with the probability which is based in the number of nodes which add the largest number of edges 

				iTemp=iNrMinFill[j];

				if (iMinNrEdgesToAdd != -1)
				{
						
					if (iTemp<iMinNrEdgesToAdd)
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iTemp;
						iNode=j;
					}
					else if (iTemp==iMinNrEdgesToAdd)
					{
						//break ties randomly...
						iNrOfSameNodes++;
						if ((rand() % iNrOfSameNodes +1)==iNrOfSameNodes)
							iNode=j;

					//	cout<<(rand() % iNrOfSameNodes +1)<<endl;
					
					}
				}
				else
				{
					iNrOfSameNodes=1;
					iMinNrEdgesToAdd=iTemp;
					iNode=j;
				}
				
				
			}	
		}


      //  cout<<iNode<<"       "<<iNrMinFill[iNode]<<endl;

		
		iElimOrdering[i]=iNode;
			
		iNodePosInOrdering[iNode]=i;


		
	 	bNodeEliminated[iNode]=true;	 

        //store the information for current number of temp neighbors...
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNrLastNeighbors[j]=iTempNrOfNeighboors[iGraph[iNode][j]];
				}

		
		  
		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];


					if (bNodeEliminated[iNeighboorNode]==false)
					{
						for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
						{
							iSecNeighboorNode=iGraph[iNode][g];
							
							if (bNodeEliminated[iSecNeighboorNode]==false ) 
							{	
						
								if (!bAreNodesConnected(iNeighboorNode, iSecNeighboorNode))
								{
									
									iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
									iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
									iTempNrOfNeighboors[iSecNeighboorNode]++;

									
									iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
									iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
									iTempNrOfNeighboors[iNeighboorNode]++;
								
								    
								
								}
							
							}
						}

					}
				
	
		
			}

	
		
			//update min fills of affected nodes...
		    
		    //determine min-fills for each node...
			for(j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];
					
					if (bNodeEliminated[iNeighboorNode]==false)
					{
							iNrEdgesToAdd=iNrMinFill[iNeighboorNode];
							
							iTemp=iTempNrOfNeighboors[iNeighboorNode];
							for( k=iTemp-1;k>iNrLastNeighbors[j]-1;k--)
							{
									iNeighboorNode1=iGraph[iNeighboorNode][k];
									if(bNodeEliminated[iNeighboorNode1]==false)
									{
										for(g=0;g<k;g++)
										{
												iSecNeighboorNode=iGraph[iNeighboorNode][g];

												if (bNodeEliminated[iSecNeighboorNode]==false)
												{
													if (!bAreNodesConnected(iNeighboorNode1, iSecNeighboorNode))
													iNrEdgesToAdd++;
												}

										}
									}
								
							}

						//set the minFill for each node
						iNrMinFill[iNeighboorNode]=iNrEdgesToAdd;
					}					
				
				}


					// we have also to cehck each pair of nodes, if they were before not connected --- 
					//and due to elimination of iNode are connected...
				    // reduce the min fill due to elimination of a node. 
				   for (f=0;f<iTempNrOfNeighboors[iNode];f++)
					{
						iTemp=iGraph[iNode][f];
									
						for(m=0;m<iNrLastNeighbors[f]; m++)
						{
							
							iTemp1=iGraph[iTemp][m];
							if (iTemp1 != iNode && bNodeEliminated[iTemp1]==false)
							{
								if (!bAreNodesConnected(iNode, iTemp1))
									iNrMinFill[iTemp]--;

								
							}

						}


					}



	    

		

	}


    
	
	for(i=0;i<iNrOfNodes;i++)
	   {

   			bNodeEliminated[i]=false;

			//cout<<i<<"       "<<iElimOrdering[i]<<endl;

	   }

	

 delete [] iNrMinFill;
 delete [] iNrLastNeighbors;



}




/*

//Min fill order based in other data structures (NM: 18.12.2010)
void probSol::NewMinFillOrder()
	{
	
    int i,j,k,m,e,g,f;
	int iNode;
	int iNeighboorNode;
	int iSecNeighboorNode;
	int iNextNode;
	int iCountN;
	int iLargestPosition;
	int iTemp;
	int iTemp1;
    int intRandom;
	int iTempNrOfNodes=0;
	int iTempNrOfEdges=0;
    int iHypSize =0;
	int iMaxHypSize=0;
	int iSingleNodes=0;
	int iNumberOfOnes=0;
    int iTotalEdges=0;
    int iAddedEdges=0;
	int iMaxAddedEdges=0;
	int iNrEdgesToAdd=0;
	int iMinNrEdgesToAdd=0;
	int iNrOfSameNodes=0;
    short int* iNrMinFill;
	bool bNodeInside;
    
//    srand(200);
   iNrMinFill=new short int[iNrOfNodes];

    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	

    

    //determine min-fills for each node...
	for (i=0;i<iNrOfNodes;i++)
	{

		        iNrEdgesToAdd=0;
				iTemp=iTempNrOfNeighboors[i];

				
				for( j=0;j<iTemp;j++)
				{
					iNeighboorNode=iGraph[i][j];
					
						for(g=j+1;g<iTemp;g++)
						{
								iSecNeighboorNode=iGraph[i][g];

								if (!bAreNodesConnected(iNeighboorNode, iSecNeighboorNode))
									iNrEdgesToAdd++;						 

						}
				}

            //set the minFill for each node
			iNrMinFill[i]=iNrEdgesToAdd;
			
		//	cout<<i<<"    "<<iTempNrOfNeighboors[i]<<"      "<<iNrMinFill[i]<<endl;
	
	}


	

	//construct the elimination ordering

	for (i=0;i<iNrOfNodes;i++)
	{
		
		iMinNrEdgesToAdd=-1;
		iNrOfSameNodes=0;
	
		
	    // select the node with min-fill
	
		for (j=0;j<iNrOfNodes;j++)
		{
            if(bNodeEliminated[j]==false)
			{
				// ask if node is already iliminated ...

				//select this node with the probability which is based in the number of nodes which add the largest number of edges 

				iTemp=iNrMinFill[j];

				if (iMinNrEdgesToAdd != -1)
				{
						
					if (iTemp<iMinNrEdgesToAdd)
					{
						iNrOfSameNodes=1;
						iMinNrEdgesToAdd=iTemp;
						iNode=j;
					}
					else if (iTemp==iMinNrEdgesToAdd)
					{
						//break ties randomly...
						iNrOfSameNodes++;
						if ((rand() % iNrOfSameNodes +1)==iNrOfSameNodes)
							iNode=j;

					//	cout<<(rand() % iNrOfSameNodes +1)<<endl;
					
					}
				}
				else
				{
					iNrOfSameNodes=1;
					iMinNrEdgesToAdd=iTemp;
					iNode=j;
				}
				
				
			}	
		}


        

		iElimOrdering[i]=iNode;
			
		iNodePosInOrdering[iNode]=i;


	
		



	//	if(iMinNrEdgesToAdd>0)
	//		cout<<i<<"     "  << iNode<<"        "<<iMinNrEdgesToAdd<<endl;


  
    
	 	bNodeEliminated[iNode]=true;	 

		// reduce the min fill due to elimination of a node. 
       for (f=0;f<iTempNrOfNeighboors[iNode];f++)
		{
			iTemp=iGraph[iNode][f];
						
			for(m=0;m<iTempNrOfNeighboors[iTemp]; m++)
			{
				
				iTemp1=iGraph[iTemp][m];
				if (iTemp1 != iNode && bNodeEliminated[iTemp1]==false)
				{
					if (!bAreNodesConnected(iNode, iTemp1))
						iNrMinFill[iTemp]--;

					
				}

			}


		}

  
		  
		//Add the neighboors for nodes that should be connected
		for( j=0;j<iTempNrOfNeighboors[iNode];j++)
				{	
					iNeighboorNode=iGraph[iNode][j];

					if (bNodeEliminated[iNeighboorNode]==false)
					{
						for(g=j+1;g<iTempNrOfNeighboors[iNode];g++)
						{
							iSecNeighboorNode=iGraph[iNode][g];
							
							if (bNodeEliminated[iSecNeighboorNode]==false ) 
							{	
						
								if (!bAreNodesConnected(iNeighboorNode, iSecNeighboorNode))
								{
									
									//update information for min fill of iNeighboorNode and iSecNeighboorNode...
									for (f=0;f<iTempNrOfNeighboors[iNeighboorNode];f++)
										{
										    iTemp=iGraph[iNeighboorNode][f];
								
											if (!bAreNodesConnected(iSecNeighboorNode, iTemp) && !bAreNodesConnected(iNode, iTemp) && bNodeEliminated[iTemp]==false)
												iNrMinFill[iNeighboorNode]++;

								
										}

										for (f=0;f<iTempNrOfNeighboors[iSecNeighboorNode];f++)
										{
										    iTemp=iGraph[iSecNeighboorNode][f];
											
											if (!bAreNodesConnected(iNeighboorNode, iTemp) && !bAreNodesConnected(iNode, iTemp) && bNodeEliminated[iTemp]==false)
												iNrMinFill[iSecNeighboorNode]++;

								
										}
	

									
									iTemp=iTempNrOfNeighboors[iSecNeighboorNode]; 
									iGraph[iSecNeighboorNode][iTemp]=iNeighboorNode;
									iTempNrOfNeighboors[iSecNeighboorNode]++;

									
									iTemp=iTempNrOfNeighboors[iNeighboorNode]; 
									iGraph[iNeighboorNode][iTemp]=iSecNeighboorNode;
									iTempNrOfNeighboors[iNeighboorNode]++;
								
								//	if (iTempNrOfNeighboors[iSecNeighboorNode]>1000 || iTempNrOfNeighboors[iNeighboorNode]>1000) 
								//		cout<<"PROBLEM"<<endl;

								    
								
								}
							
							}
						}

					}
				
	
		
		}

		
		
	    
	
		
		

	}


    
	
	for(i=0;i<iNrOfNodes;i++)
	   {

   			bNodeEliminated[i]=false;

			//cout<<i<<"       "<<iElimOrdering[i]<<endl;

	   }

	




}


*/


//returns true if two nodes in a graph are connected (based on temporar neighboors)
bool probSol::bAreNodesConnected(int iNode1, int iNode2)
{
    bool bNodeInside=false;
	int m;

	//test neighborhood of the node which has less neighboors (because of the efficiency)
	if (iTempNrOfNeighboors[iNode1]<iTempNrOfNeighboors[iNode2])
	{       

	for (m=0;m<iTempNrOfNeighboors[iNode1];m++)
		{
			if(iGraph[iNode1][m]==iNode2)
			{
				bNodeInside=true; 
				break;
			}
		}
	}
	else
	{
		for (m=0;m<iTempNrOfNeighboors[iNode2];m++)
			{
				if(iGraph[iNode2][m]==iNode1)
				{
					bNodeInside=true; 
					break;
				}
			}
	}


	return bNodeInside;
}








	void probSol::NewMinDegreeOrder()
	{
	}
	










void probSol::NewMCSOrder()
{
	 int i,j,f;
	int iNode=0;
	int iNeighboorNode;
	int iClique;
	int iNrEliminatedNeighbors=0;
	int iMinNrEliminatedNeighbors=0;
	int iNrOfSameNodes=0;

    ///////////////////////////
    ////////////////////////////
	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;


    // state current state for the number of eliminated neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=0;
		bNodeEliminated[i]=false;
	}
	

 
	for (i=0;i<iNrOfNodes;i++)
	{
	   
		////////////////////////////////////////////////////////////////
		// find the node to be eliminated based on max carinality search...
		iNrOfSameNodes=0;
	   	iMinNrEliminatedNeighbors=0;
		
		if (i==0)
		{
			iNode=rand() % iNrOfNodes;

		}
		else
		{
			
			for(f=0;f<iNrOfNodes;f++)
			{
				if (bNodeEliminated[f]==false)
				{
					iNrEliminatedNeighbors=iTempNrOfNeighboors[f];
						
						//select this node with the probability which is based in the number of nodes which add the largest number of edges 

						if (iNrEliminatedNeighbors>iMinNrEliminatedNeighbors)
						{
							iNrOfSameNodes=1;
							iMinNrEliminatedNeighbors=iNrEliminatedNeighbors;
							iNode=f;
						}
						else if (iNrEliminatedNeighbors==iMinNrEliminatedNeighbors)
						{
						
							iNrOfSameNodes++;
							if (((rand() % iNrOfSameNodes )+1)==iNrOfSameNodes)
								iNode=f;
						
						}
							
				
				}
			
				
			}
		
		}

		iElimOrdering[iNrOfNodes-i-1]=iNode;
		iNodePosInOrdering[iNode]=iNrOfNodes-i-1;


		//Increase the number fo eliminated neighbors for nodes that are connected with the last eliminated node
		for( j=0;j<iNrOfNeighboors[iNode];j++)
			{	
				iNeighboorNode=iGraph[iNode][j];
				iTempNrOfNeighboors[iNeighboorNode]++;

			}

		 bNodeEliminated[iNode]=true;

	}
    


	   //reset bNodeEliminated
	   for(i=0;i<iNrOfNodes;i++)
	   {

   			bNodeEliminated[i]=false;

	   }
	


}
 


 


//Maximum cardinality Search
void probSol::MCSOrdering()
{ 
    int i,j,f;
	int iNode=0;
	int iNeighboorNode;
	int iClique;
	int iNrEliminatedNeighbors=0;
	int iMinNrEliminatedNeighbors=0;
	int iNrOfSameNodes=0;

    ///////////////////////////
    ////////////////////////////
	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;


    // state current state for the number of eliminated neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=0;
		bNodeEliminated[i]=false;
	}
	

 
	for (i=0;i<iNrOfNodes;i++)
	{
	   
		////////////////////////////////////////////////////////////////
		// find the node to be eliminated based on max carinality search...
		iNrOfSameNodes=0;
	   	iMinNrEliminatedNeighbors=0;
		
		if (i==0)
		{
			iNode=rand() % iNrOfNodes;

		}
		else
		{
			
			for(f=0;f<iNrOfNodes;f++)
			{
				if (bNodeEliminated[f]==false)
				{
					iNrEliminatedNeighbors=iTempNrOfNeighboors[f];
						
						//select this node with the probability which is based in the number of nodes which add the largest number of edges 

						if (iNrEliminatedNeighbors>iMinNrEliminatedNeighbors)
						{
							iNrOfSameNodes=1;
							iMinNrEliminatedNeighbors=iNrEliminatedNeighbors;
							iNode=f;
						}
						else if (iNrEliminatedNeighbors==iMinNrEliminatedNeighbors)
						{
						
							iNrOfSameNodes++;
							if (((rand() % iNrOfSameNodes )+1)==iNrOfSameNodes)
								iNode=f;
						
						}
							
				
				}
			
				
			}
		
		}

		iElimOrdering[iNrOfNodes-i-1]=iNode;
		iNodePosInOrdering[iNode]=iNrOfNodes-i-1;


		//Increase the number fo eliminated neighbors for nodes that are connected with the last eliminated node
		for( j=0;j<iNrOfNeighboors[iNode];j++)
			{	
				iNeighboorNode=iGraph[iNode][j];
				iTempNrOfNeighboors[iNeighboorNode]++;

			}

		 bNodeEliminated[iNode]=true;

	}
    


	   //reset bNodeEliminated
	   for(i=0;i<iNrOfNodes;i++)
	   {

   			bNodeEliminated[i]=false;

	   }
	


}
 





/*

// CALCULATE FITNESS ONLY BASED ON TREE DECOMPOSITIONS
//calculates fintess based on the given ordering...
void probSol::calculateFitness()
{ 
    int i,j,k,m;
	int iNode;
	int iNeighboorNode;
	int iNextNode;
	int iCountN;
	int iLargestPosition;
	int iTemp;
	int iClique;
    int intRandom;

	fFitness=0;
    iClique=0;
	iNrNodesLargestClique=0;


    // state current state for the number of neighboors for each node...
	for (i=0;i<iNrOfNodes;i++) 
	{
		iTempNrOfNeighboors[i]=iNrOfNeighboors[i];
		bNodeEliminated[i]=false;
	}
	
	for (i=0;i<iNrOfNodes-1;i++)
	{
		
			   iNode=iElimOrdering[i];


		       ///USED only if hypertree decomp is generated...
			   CompSet *Nodes, *HEdges, *CovEdges;
			   SetCover SC;
			   Node* objNode;
			   set<Node*>			NodeList;
			   set<Hyperedge*>	candidateEdgeList;
			   
			   
			   					///////////////////////////////////////////////////////////
					// used obly if hypertree decompostions should be generated
					if (bHypertreeAlg==true)
					{   
					
						objNode=Hyp->getNode( iNode);
						NodeList.insert(objNode);
    					for(k=0;k<objNode->getNbrOfEdges();k++)
						candidateEdgeList.insert(objNode->getEdge(k)); 
					}

					////////////////////////////////////////////////////////////

			   
			   ////////////////////////////////////////	

		iCountN=0;
		iLargestPosition=iNrOfNodes;
		
		//find the number of neighboors (not yet eliminated) of node and the next neighborhood node 
		// in the elimination ordering
        for( j=0;j<iTempNrOfNeighboors[iNode];j++)
		{
			iNeighboorNode=iGraph[iNode][j];

			if (bNodeEliminated[iNeighboorNode]==false)
			{
                iCountN++;
                // which is the position of this node in ordering...
                if (iNodePosInOrdering[iNeighboorNode]< iLargestPosition) 
				{iNextNode=iNeighboorNode;iLargestPosition= iNodePosInOrdering[iNeighboorNode];}

					
					///////////////////////////////////////////////////////////
					// used only if hypertree decompostions should be generated
					if (bHypertreeAlg==true)
					{   
					
						objNode=Hyp->getNode(iNeighboorNode);
						NodeList.insert(objNode);
    					for(k=0;k<objNode->getNbrOfEdges();k++)
						candidateEdgeList.insert(objNode->getEdge(k)); 
					}

					////////////////////////////////////////////////////////////

			}

		}


		// if set cover should be applied for hypertree decomposition
        if (bHypertreeAlg==true && iCountN>iClique)
		{

			   Nodes = new CompSet(NodeList.size());
			   HEdges = new CompSet(candidateEdgeList.size());

			   if((Nodes == NULL) || (HEdges == NULL))
					writeErrorMsg("Error assigning memory.", "hypergraph::isNieghborhoodCoveredByKEdges()");


				set<Node*>::iterator	nodeListIter;
				for(nodeListIter = NodeList.begin(); nodeListIter != NodeList.end(); ++nodeListIter)
					Nodes->insComp(*nodeListIter);

				set<Hyperedge*>::iterator	edgeListIter;
				for(edgeListIter = candidateEdgeList.begin(); edgeListIter != candidateEdgeList.end(); ++edgeListIter)
					HEdges->insComp(*edgeListIter);

				// cover the nodes with minimal possible number of hyperedges...
				CovEdges = SC.cover(Nodes, HEdges);

				NodeList.clear();
				candidateEdgeList.clear();

				delete Nodes;
				delete HEdges;

				iCountN=CovEdges->size(); 
				delete CovEdges;
			}



		//Add the neighboors (except iNextNode)  of iNode in the neighboors of iNextNode 
        for( j=0;j<iTempNrOfNeighboors[iNode];j++)
		{	
			iNeighboorNode=iGraph[iNode][j];
            //Add the neighboor only if it is not eliminated yet, and if it is not already registed as e neighboor before 
			if (bNodeEliminated[iNeighboorNode]==false && iNeighboorNode != iNextNode && iZeroOneGraph[iNextNode][iNeighboorNode]!=1)
			{
			    iTemp=iTempNrOfNeighboors[iNextNode]; 	
				iGraph[iNextNode][iTemp]=iNeighboorNode;
				iTempNrOfNeighboors[iNextNode]++;
                iZeroOneGraph[iNextNode][iNeighboorNode]=1;
			}
		}

        bNodeEliminated[iNode]=true;
 
        if (iClique<iCountN) 
		{
			iClique=iCountN;
			iLargestCliqueNode=iElimOrdering[i];
			iNrNodesLargestClique=1;
		}
	    else if (iClique==iCountN) 
		{
			iNrNodesLargestClique++;
			//Gives to all nodes with the same clique to be selected for the node with the largest 
			// clique... the node with largest clique can be used then in search and this 
			// makes possible to select this node randomly among the nodes with the same clique size...
			intRandom=(rand() % iNrNodesLargestClique )+1;
			if (intRandom==iNrNodesLargestClique)  
				iLargestCliqueNode=iElimOrdering[i];
		}
      
        

		//second method for calculation of fitness..
		//fFitness=fFitness+iCountN*iCountN;

		iCliqueSizes[i]=iCountN;
	}
    
    //second way for calculation of fitness
    //fFitness =fFitness+iNrOfNodes*iNrOfNodes *iClique*iClique;
	fFitness =iClique;
	iLargestClique=iClique;


   //reset iZeroOneGraph.  iGraph needs not to be reseted as the information for it is stored in iNrOfNeighboors;
   for(i=0;i<iNrOfNodes;i++)
   {
	   for(j=iNrOfNeighboors[i];j<iTempNrOfNeighboors[i];j++)
	   {            
          iNode=iGraph[i][j];
          iZeroOneGraph[i][iNode]=0;
	   }

   }

   	for (i=0;i<iNrOfNodes;i++) 
	{
		bNodeEliminated[i]=false;
	}
	
}
 
*/



///////////////////////////////////////////////////////////////////////////////////////////////////
void probSol::createGraphFromHyp(Hypergraph *myHypergraph)
///////////////////////////////////////////////////////////////////////////////////////////////////
{

    int i,j;
	int iNodes;
	
	int node1,node2;
    int iNode1Edges,iNode2Edges ; 

    

	Hyp=myHypergraph;

    //Hyp->setMFOrder();     
    //Hyp->setMCSOrder();     

    iNrOfNodes=myHypergraph->getNbrOfNodes();  
	iNrOfEdges=myHypergraph->getNbrOfEdges();     
	
	iNodePosInOrdering=new unsigned short int[iNrOfNodes];


	iElimOrdering=new unsigned short int[iNrOfNodes];
    iMaxCliqueNodes=new unsigned short int[iNrOfNodes];	
	
	iTempNodes=new unsigned short int[iNrOfNodes]; 
	for(i=1;i<iNrOfNodes;i++) iTempNodes[i]=0;
	iEdgeSelected = new int[iNrOfEdges];

	
	//set labels of nodes ...
	for(i=0;i<iNrOfNodes;i++)
	{
	
		myHypergraph->getNode(i)->setLabel(i); 

	}

   	//set labels of nodes ...
	for(i=0;i<iNrOfEdges;i++)
	{
		myHypergraph->getEdge(i)->setLabel(i); 
	}



	/*
	for(i=0;i<iNrOfNodes;i++)
	{	
		iElimOrdering[i]=i ;
		iNodePosInOrdering[i]=i;
	
	}
	*/


   	for(i=0;i<iNrOfNodes;i++)
	{
		//MCS ordering beginning from the end ...	
		iElimOrdering[i]=iNrOfNodes-i-1 ;
		iNodePosInOrdering[iNrOfNodes-i-1]=i;
	
	}
	
	
		  iGraph=new unsigned short int*[iNrOfNodes];
		  iZeroOneGraph= new bool*[iNrOfNodes];  
		  iNodesEdges=new unsigned short int*[iNrOfNodes]; 

		   
		  for(i=0;i<iNrOfNodes;i++) 
		   {
			   iGraph[i]=new unsigned short int[iNrOfNodes];
		       iZeroOneGraph[i]= new bool[iNrOfNodes];  
			   iNodesEdges[i]=new unsigned short int[iNrOfEdges];
		       
		   }

           	   for(i=0;i<iNrOfNodes;i++) 
			   {
				   for(j=0;j<iNrOfNodes;j++) 
				   {
					   iZeroOneGraph[i][j]=0;
					   iGraph[i][j]=0;
					   
				   }

				   for(j=0;j<iNrOfEdges;j++) 
				   {
					  iNodesEdges[i][j]=0;					   
				   }
	
			   }
			
		


		   iNrOfNeighboors=new unsigned short int[iNrOfNodes];
   		   iNrOfNodeEdges = new unsigned short int[iNrOfNodes];
    		iCliqueSizes=new unsigned short int[iNrOfNodes];
		   
			for(i=0;i<iNrOfNodes; i++) 
			{
				iNrOfNeighboors[i]=0;
				iCliqueSizes[i]=0;
				iNrOfNodeEdges [i]=0;
			}

           iTempNrOfNeighboors=new unsigned short int[iNrOfNodes];

		   bNodeEliminated=new bool[iNrOfNodes];
	
	
	
	for(iNodes=0;iNodes<iNrOfNodes;iNodes++)
	{ 
   
        
	  	    //read cell of matrix which is 1 
			//supposing that the nodes begin from 0...
			//sscanf(buffer,"%c %d %d",&cLine,&node1, &node2);
	        

		node1=iNodes;
        for(i=0;i<myHypergraph->getNode(iNodes)->getNbrOfNeighbours();i++)
		{
			
    		node2=myHypergraph->getNode(iNodes)->getNeighbour(i)->getLabel();

			// add the edge if it is not added yet...
			if (iZeroOneGraph[node1][node2]==0)
			{
			   if (node1!=node2)
			   {
					iNrOfNeighboors[node1]++;
					iNrOfNeighboors[node2]++;

					//we give -1 , because the neighbors beginn from index  0...
					iNode1Edges=iNrOfNeighboors[node1]-1;
					iNode2Edges=iNrOfNeighboors[node2]-1;
            
					//we begin from node with number 0...
					iGraph[node1][iNode1Edges]=node2;
					iGraph[node2][iNode2Edges]=node1;
			   }        

			   iZeroOneGraph[node1][node2]=1;
			   iZeroOneGraph[node2][node1]=1;

			}
		    
		}
	
		for(i=0;i<myHypergraph->getNode(iNodes)->getNbrOfEdges();i++)
		{	
			iNrOfNodeEdges[iNodes]++;
			iNodesEdges[iNodes][iNrOfNodeEdges[iNodes]-1]=myHypergraph->getNode(iNodes)->getEdge(i)->getLabel();		
	
		}
	
	
	}

}



//New function , because of memory problems with createGraphFromHyp 
///////////////////////////////////////////////////////////////////////////////////////////////////
void probSol::createGraphFromHypNew(Hypergraph *myHypergraph)
///////////////////////////////////////////////////////////////////////////////////////////////////
{

    int i,j;
	int iNodes;
	
	int node1,node2;
    int iNode1Edges,iNode2Edges ; 

    

	int iMaxNrOfNeighboors=1000; 


	Hyp=myHypergraph;

 
    iNrOfNodes=myHypergraph->getNbrOfNodes();  
	iNrOfEdges=myHypergraph->getNbrOfEdges();     
	
	iNodePosInOrdering=new unsigned short int[iNrOfNodes];


	iElimOrdering=new unsigned short int[iNrOfNodes];
    iMaxCliqueNodes=new unsigned short int[iNrOfNodes];	
	
	iTempNodes=new unsigned short int[iNrOfNodes]; 
	for(i=1;i<iNrOfNodes;i++) iTempNodes[i]=0;
	iEdgeSelected = new int[iNrOfEdges];

	
	//set labels of nodes ...
	for(i=0;i<iNrOfNodes;i++)
	{
	
		myHypergraph->getNode(i)->setLabel(i); 

	}

   	//set labels of nodes ...
	for(i=0;i<iNrOfEdges;i++)
	{
		myHypergraph->getEdge(i)->setLabel(i); 
	}




   	for(i=0;i<iNrOfNodes;i++)
	{
		iElimOrdering[i]=i;
		iNodePosInOrdering[i]=i;
	}
	
	
		  iGraph=new unsigned short int*[iNrOfNodes];
//		  iZeroOneGraph= new bool*[iNrOfNodes];  
//		  iNodesEdges=new unsigned short int*[iNrOfNodes]; 

		   
		  for(i=0;i<iNrOfNodes;i++) 
		   {
			   iGraph[i]=new unsigned short int[iMaxNrOfNeighboors];
//		       iZeroOneGraph[i]= new bool[iNrOfNodes];  
//			   iNodesEdges[i]=new unsigned short int[iNrOfEdges];
		       
		   }

           	   for(i=0;i<iNrOfNodes;i++) 
			   {
				   for(j=0;j<iMaxNrOfNeighboors;j++) 
				   {
					   iGraph[i][j]=0;
					   
				   }

			   }
			
		


		   iNrOfNeighboors=new unsigned short int[iNrOfNodes];
   		   iNrOfNodeEdges = new unsigned short int[iNrOfNodes];
    		iCliqueSizes=new unsigned short int[iNrOfNodes];
		   
			for(i=0;i<iNrOfNodes; i++) 
			{
				iNrOfNeighboors[i]=0;
				iCliqueSizes[i]=0;
				iNrOfNodeEdges [i]=0;
			}

           iTempNrOfNeighboors=new unsigned short int[iNrOfNodes];

		   bNodeEliminated=new bool[iNrOfNodes];
	
	
	
	for(iNodes=0;iNodes<iNrOfNodes;iNodes++)
	{ 
   
        
	  	    //read cell of matrix which is 1 
			//supposing that the nodes begin from 0...
			//sscanf(buffer,"%c %d %d",&cLine,&node1, &node2);
	        

		node1=iNodes;
        for(i=0;i<myHypergraph->getNode(iNodes)->getNbrOfNeighbours();i++)
		{
			
    		node2=myHypergraph->getNode(iNodes)->getNeighbour(i)->getLabel();

			   if (node1!=node2 && node1<node2) //node1<node2  is added to assure that the neighboors are not added two times 
			   {
					iNrOfNeighboors[node1]++;
					iNrOfNeighboors[node2]++;

					//we give -1 , because the neighbors beginn from index  0...
					iNode1Edges=iNrOfNeighboors[node1]-1;
					iNode2Edges=iNrOfNeighboors[node2]-1;
            
					//we begin from node with number 0...
					iGraph[node1][iNode1Edges]=node2;
					iGraph[node2][iNode2Edges]=node1;
			   }        

			
		    
		}
	
	
	}




}





/* changed (see above on 30.01.2008)
void probSol::createGraphFromHyp(Hypergraph *myHypergraph)
{

    int iTempRand,i,j;
	int iNodes;
    char buffer[256];
	
	int node1,node2;
    int iNode1Edges,iNode2Edges ; 

	char *temp;

    
    char cLine;
	char* cLine1;
     

	Hyp=myHypergraph;

    iNrOfNodes=myHypergraph->getNbrOfNodes();  

   //set labels of nodes ...
	for(i=0;i<iNrOfNodes;i++)
	{
		myHypergraph->getNode(i)->setLabel(i+1); 
	
	}


		   iGraph=new unsigned short int*[iNrOfNodes];
		   iZeroOneGraph= new bool*[iNrOfNodes];  
           
		   iNrCommonHyperedges=new unsigned short int*[iNrOfNodes];
		   iTempNrCommonHyperedges=new unsigned short int*[iNrOfNodes];


		   for(i=0;i<iNrOfNodes;i++) 
		   {
			   iGraph[i]=new unsigned short int[iNrOfNodes];
		       iZeroOneGraph[i]= new bool[iNrOfNodes];  
			   iNrCommonHyperedges[i]=new unsigned short int[iNrOfNodes];
			   iTempNrCommonHyperedges[i]=new unsigned short int[iNrOfNodes];
		   }

           	   for(i=0;i<iNrOfNodes;i++) 
			   {
				   for(j=0;j<iNrOfNodes;j++) 
				   {
					   iZeroOneGraph[i][j]=0;
					   iNrCommonHyperedges[i][j]=0;
					   iTempNrCommonHyperedges[i][j]=0;
				   }


			   }
			
		
		   iNrOfHyperdeges=new unsigned short int[iNrOfNodes]; 
			iTempNrOfHyperdeges=new unsigned short int[iNrOfNodes]; 


		   iNrOfNeighboors=new unsigned short int[iNrOfNodes];
		   for(i=0;i<iNrOfNodes; i++) iNrOfNeighboors[i]=0;
           
           iCliqueSizes=new unsigned short int[iNrOfNodes];
           for(i=0;i<iNrOfNodes; i++) iCliqueSizes[i]=0;
           

           iTempNrOfNeighboors=new unsigned short int[iNrOfNodes];

		   bNodeEliminated=new bool[iNrOfNodes];
	
	
	
	for(iNodes=0;iNodes<iNrOfNodes;iNodes++)
	{ 
   
        
	  	    //read cell of matrix which is 1 
			//supposing that the nodes begin from 1...
			//sscanf(buffer,"%c %d %d",&cLine,&node1, &node2);
	        
		iNrOfHyperdeges[iNodes]=myHypergraph->getNode(iNodes)->getNbrOfEdges(); 

		node1=iNodes+1;
        for(i=0;i<myHypergraph->getNode(iNodes)->getNbrOfNeighbours();i++)
		{
			
    		node2=myHypergraph->getNode(iNodes)->getNeighbour(i)->getLabel();

			// add the edge if it is not added yet...
			if (iZeroOneGraph[node1-1][node2-1]==0)
			{
			   if (node1!=node2)
			   {
					iNrOfNeighboors[node1-1]++;
					iNrOfNeighboors[node2-1]++;

					iNode1Edges=iNrOfNeighboors[node1-1];
					iNode2Edges=iNrOfNeighboors[node2-1];
            
					//we begin from node with number 0...
					iGraph[node1-1][iNode1Edges-1]=node2-1;
					iGraph[node2-1][iNode2Edges-1]=node1-1;
			   }        
				iZeroOneGraph[node1-1][node2-1]=1;
				iZeroOneGraph[node2-1][node1-1]=1;

                iNrCommonHyperedges[node1-1][node2-1]=1;
				iNrCommonHyperedges[node2-1][node1-1]=1;

				

			}
			
		    
		}
	
	}


}


*/








void probSol::readFromFile(char *strnameoffile)
{

    int i,j;
    char buffer[256];
	
	int node1,node2;
    int iNode1Edges,iNode2Edges ; 


   ifstream myfile(strnameoffile);
 
  
   if (! myfile.is_open())
   { 
	   cout << endl<<"Error opening file...  "<<  strnameoffile;
	   cout<<endl<<"Probleme me hapjen e fajllit"<<endl<<endl;
	   exit (1); 
   }

   
    char cLine;
	char cLine1[100];
   
    while (!myfile.eof() )
	{ 
   
      myfile.getline (buffer,100);
      
	   if(buffer[0]=='p')
	   {
		   sscanf(buffer,"%c %s %d %d",&cLine, cLine1, &iNrOfNodes, &iNrOfEdges);
    	   cout<<endl<<endl<<"Nr nodes "<<iNrOfNodes<<"Nr. of edges:"<<iNrOfEdges<<endl<<endl;  

   
		   iGraph=new unsigned short int*[iNrOfNodes];
		   iZeroOneGraph= new bool*[iNrOfNodes];  
		   
		   for(i=0;i<iNrOfNodes;i++) 
		   {
			   iGraph[i]=new unsigned short int[iNrOfNodes];
               iZeroOneGraph[i]= new bool[iNrOfNodes];  
		   }

           	   for(i=0;i<iNrOfNodes;i++) 
			   {for(j=0;j<iNrOfNodes;j++) iZeroOneGraph[i][j]=0;
			   }


		   iNrOfNeighboors=new unsigned short int[iNrOfNodes];
		   for(i=0;i<iNrOfNodes; i++) iNrOfNeighboors[i]=0;
           
           iCliqueSizes=new unsigned short int[iNrOfNodes];
           for(i=0;i<iNrOfNodes; i++) iCliqueSizes[i]=0;
           

           iTempNrOfNeighboors=new unsigned short int[iNrOfNodes];

		   bNodeEliminated=new bool[iNrOfNodes];


		
        
	   }
      else if (buffer[0]=='e')	   
		{
    
     
		  //read cell of matrix which is 1 
			//supposing that the nodes begin from 1...
			sscanf(buffer,"%c %d %d",&cLine,&node1, &node2);

		
                
			// add the edge if it is not added yet...
			if (iZeroOneGraph[node1-1][node2-1]==0)
			{
			   if (node1!=node2)
			   {
					iNrOfNeighboors[node1-1]++;
					iNrOfNeighboors[node2-1]++;

					iNode1Edges=iNrOfNeighboors[node1-1];
					iNode2Edges=iNrOfNeighboors[node2-1];
            
					//we begin from node with number 0...
					iGraph[node1-1][iNode1Edges-1]=node2-1;
					iGraph[node2-1][iNode2Edges-1]=node1-1;
			   }        
				iZeroOneGraph[node1-1][node2-1]=1;
				iZeroOneGraph[node2-1][node1-1]=1;
			}        
		    
		}
	}


}

