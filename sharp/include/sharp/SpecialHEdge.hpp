/*
***Description***
Class SpecialHEdge models a special hyperedge that can be both a hyperedge and a set of hyperedges.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_SPECIALHEDGE)
#define CLS_SPECIALHEDGE


#include <sharp/Hyperedge.hpp>
#include <sharp/CompSet.hpp>

namespace sharp
{

	class SpecialHEdge : public Hyperedge, public CompSet  
	{
	public:
		// Constructor
		SpecialHEdge(int iNbrOfEdges);
	
		// Destructor
		virtual ~SpecialHEdge();
	};

} // namespace sharp

#endif // !defined(CLS_SPECIALHEDGE)
