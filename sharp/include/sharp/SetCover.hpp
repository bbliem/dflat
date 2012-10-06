/*
***Description***
Class SetCover models set covering algorithms.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_SETCOVER)
#define CLS_SETCOVER

namespace sharp
{

	class CompSet;
	
	class SetCover  
	{
	private:
		// Checks whether a set of nodes can be covered by a set of hyperedges
		bool isCoverable(CompSet *Nodes, CompSet *HEdges);
	
		// Covers a set of nodes by a set of hyperedges
		CompSet *NodeCover1(CompSet *Nodes, CompSet *HEdges, bool bDeterm);
	
		// Covers a set of nodes by a set of hyperedges
		CompSet *NodeCover2(CompSet *Nodes, CompSet *HEdges, bool bDeterm);
	
	public:
		// Constructor
		SetCover();
	
		// Destructor
		virtual ~SetCover();
	
		// Covers a set of nodes by a set of hyperedges
		CompSet *cover(CompSet *Nodes, CompSet *HEdges);
	};

} // namespace sharp

#endif // !defined(CLS_SETCOVER)
