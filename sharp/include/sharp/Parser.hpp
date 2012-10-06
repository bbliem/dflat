/*
***Description***
Class Parser models a parser for parsing the hypergraph given in an appropriate file format 
and stores the information in preliminary data structures.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_PARSER)
#define CLS_PARSER


#include <iostream>
#include <fstream>
#include <vector>
#include <set>

using namespace std;


#define IF_BUFFER_SIZE 1024
#define MAX_IDENTIFIER_SIZE 128

//#pragma warning(push)
//#pragma warning(disable:4786)

namespace sharp
{

	class Parser 
	{
	private:
		// Input file stream
	    ifstream MyFile;
	
		// Internal buffer for file data
		char cMyBuffer[IF_BUFFER_SIZE];
	
		// Current position in the internal buffer cMyBuffer
		int iMyBufferPos;
	
		// Current line number in the input file
		int iMyLineNumber;
	
		// Current column number in the input file
		int iMyColumnNumber;
	
		// Line number at which end-token was found (initially 0)
		int iMyEndOfAtoms;
	
		// Counts the number of syntax warnings
		int iMyNbrOfWarnings;
	
		// Boolean value indicating whether end of file was found
		bool bMyEOF;
	
		// Boolean value indicating whether definitions have to be checked
		bool bMyDef;
	
		// Pointers to the atom identifiers
		vector<char *> MyDefAtoms;
	
		// Pointers to the variable identifiers
		vector<char *> MyDefVariables;
	
		// Arities of the atoms
		vector<int> MyArities;
	
		// Boolean value indicating if the corresponding atom was already used
		vector<bool> MyDefAtomUsed;
	
		// Boolean value indicating if the corresponding variable was already used
		vector<bool> MyDefVarUsed;
	
		// Pointers to the atom identifiers
		vector<char *> MyAtoms;
	
		// Pointers to the variable identifiers
		vector<char *> MyVariables;
	
		// Pointers to sets of variables belonging to an atom;
		// the numbers in the sets refer to variables in MyVariables
		vector<set<int> *> MyAtomVars;
	
		// Pointers to sets of neighbour atoms belonging to an atom;
		// the numbers in the sets refer to atoms in MyAtoms
		vector<set<int> *> MyAtomNeighbours;
	
		// Pointers to sets of atoms to which a variable belongs;
		// the numbers in the sets refer to atoms in MyAtoms
		vector<set<int> *> MyVarAtoms;
	
		// Pointers to sets of neighbour variables belonging to a variable;
		// the numbers in the sets refer to variables in MyVariables
		vector<set<int> *> MyVarNeighbours;
	
	   	// Writes a standardized syntax error message to the standard output and aborts the program
		void SyntaxError(const char *cMsg, bool bLineNbr = false);
	
		// Writes a standardized syntax warning message to the standard output and continues the program
		void SyntaxWarning(const char *cMsg, bool bLineNbr = false);
	
		// Fills the internal file buffer cMyBuffer
		void fillBuffer();
	
		// Reads text from the input file that can be ignored (e.g., space, newline, ...)
		void readIgnoreText();
	
		// Reads an identifier (i.e., name of an atom or a variable) from the input file
		char *readIdentifier();
	
		// Reads an identifier given as argument
		bool readIdentifier(const char *cIdent);
	
		// Reads the arity of an atom
		int readArity();
	
		// Reads variable definitions
		void readVarDefs();
	
		// Reads atom definitions
		void readAtomDefs();
	
		// Reads token to open definition
		bool readOpenDef();
	
		// Reads token to close definition
		void readCloseDef();
	
		// Reads token to open an atom body
		void readOpenBody();
	
		// Reads token to close an atom body
		void readCloseBody();
	
		// Reads separator of a list of tokens
		bool readSeparator();
		
		// Reads end-token indicating the end of the atom list
		void readEnd();
	
		// Reads definitions of variables and atoms
		void readDefinitions();
	
		// Reads an atom (including its variables) from the file and stores the information in the above data structures
		void readAtom();
	
		// Reads a variable from the file and stores the information in the above data structures
		void readVariable();
	
	public:
		// Constructor
		Parser(bool bDef);
	
		// Destructor
		virtual~Parser();
	
		// Parses the hypergraph in a given file and stores the information in internal data structures
		void parseFile(const char *cNameOfFile);
	
		// Returns the number of atoms currently stored in the data structures
		int getNbrOfAtoms();
	
		// Returns the number of variables currently stored in the data structures
		int getNbrOfVars();
		
		// Returns the number of variables in a given atom
		int getNbrOfVars(int iAtom);
		
		// Returns the number of atoms to which a given variable belongs
		int getNbrOfAtoms(int iVar);
	
		// Returns the number of atoms that are neighbours (i.e., have common variables) of a given atom
	    int getNbrOfAtomNeighbours(int iAtom);
	
		// Returns the number of variables that are neighbours (i.e., have common atoms) of a given variable
	    int getNbrOfVarNeighbours(int iVar);
	
		// Returns the number of syntax warnings
		int getNbrOfWarnings();
		
		// Returns the name of the iAtom-th atom
		char *getAtom(int iAtom);
	
		// Returns the name of the iVar-th variable
		char *getVariable(int iVar);
	
		// Returns successively the variables belonging to atoms in the following order:
		// V0 of A0, V1 of A0, V2 of A0, ..., Vn0 of A0, V0 of A1, V1 of A1, ..., Vn1 of A1, V0 of A2, ...
		int getNextAtomVar(bool bReset = false);
	
		// Returns successively the atoms to which the variables belong in the following order:
		// A0 of V0, A1 of V0, A2 of V0, ..., An0 of V0, A0 of V1, A1 of V1, ..., An1 of V1, A0 of V2, ...
		int getNextVarAtom(bool bReset = false);
	
		// Returns successively the neighbours belonging to atoms in the following order:
		// N0 of A0, N1 of A0, N2 of A0, ..., Nn0 of A0, N0 of A1, N1 of A1, ..., Nn1 of A1, N0 of A2, ...
		int getNextAtomNeighbour(bool bReset = false);
	
		// Returns successively the neighbours belonging to variables in the following order:
		// N0 of V0, N1 of V0, N2 of V0, ..., Nn0 of V0, N0 of V1, N1 of V1, ..., Nn1 of V1, N0 of V2, ...
		int getNextVarNeighbour(bool bReset = false);
	};

} // namespace sharp

//#pragma warning(pop)

#endif // !defined(CLS_PARSER);

