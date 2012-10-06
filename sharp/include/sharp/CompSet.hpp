/*
***Description***
Class CompSet models a set of components, i.e., hyperedges or nodes.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_COMPSET)
#define CLS_COMPSET

namespace sharp
{

	class Component;
	
	class CompSet
	{
	protected:
		// Maximum number of components contained in the set
		int iMyMaxNbrOfComps;
	
		// Number of components currently contained in the set
		int iMyNbrOfComps;
	
		// Array of pointers to components representing the set
		Component **MyComponents;
	
		// Special flag
		bool bMySpecial;
	
	public:
		// Constructor
		CompSet(int iNbrOfComps);
	
		// Destructor
		virtual ~CompSet();
	
		// Inserts a component into the set
		bool insComp(Component *Comp);
	
		// Remove the component at position iPos
		void remComp(int iPos);
	
		// Remove the component Comp
		bool remComp(Component *Comp);
	
		// Get number of components currently in the set
		int size();
	
		// Returns true if the set is empty; otherwise false
		bool empty();
	
		// Removes all elements from the set
		void clear();
	
		// Returns true if Comp is contained in the set; otherwise false
		bool find(Component *Comp);
	
		// Get the component at position iPos
		Component *operator[](int iPos);
	
		// Returns the special flag
		bool isSpecial();
	
		// Sort components according to their labels
		void sortByLabels();
	
		// Checks whether the actual set is a subset of Set
		bool isSubset(CompSet *Set);
	};

} // namespace sharp

#endif // !defined(CLS_COMPSET)
