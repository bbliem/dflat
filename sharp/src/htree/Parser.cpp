// Parser.cpp: implementation of the Parser class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

#include <Parser.hpp>
#include <Globals.hpp>

namespace sharp
{

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	
	Parser::Parser(bool bDef)
	{
		cMyBuffer[0] = '\0';
		iMyBufferPos = 0;
		iMyLineNumber = 1;
		iMyColumnNumber = 0;
		iMyEndOfAtoms = 0;
		iMyNbrOfWarnings = 0;
		bMyEOF = false;
		bMyDef = bDef;
	}
	
	
	Parser::~Parser()
	{
		int iDefAtom = 0, iDefVar = 0;
		vector<set<int> *>::iterator sVectorIter;
		vector<bool>::iterator bVectorIter;
	
		for (sVectorIter=MyAtomVars.begin(); sVectorIter != MyAtomVars.end(); sVectorIter++)
			delete *sVectorIter;
	
		for (sVectorIter=MyAtomNeighbours.begin(); sVectorIter != MyAtomNeighbours.end(); sVectorIter++)
			delete *sVectorIter;
	
		for (sVectorIter=MyVarAtoms.begin(); sVectorIter != MyVarAtoms.end(); sVectorIter++)
			delete *sVectorIter;
	
		for (sVectorIter=MyVarNeighbours.begin(); sVectorIter != MyVarNeighbours.end(); sVectorIter++)
			delete *sVectorIter;
	
		for (bVectorIter=MyDefAtomUsed.begin(); bVectorIter != MyDefAtomUsed.end(); bVectorIter++, iDefAtom++)
			if (!*bVectorIter)
				delete MyDefAtoms[iDefAtom];
	
		for (bVectorIter=MyDefVarUsed.begin(); bVectorIter != MyDefVarUsed.end(); bVectorIter++, iDefVar++)
			if (!*bVectorIter)
				delete MyDefVariables[iDefVar];
	}
	
	
	
	//////////////////////////////////////////////////////////////////////
	// Class methods
	//////////////////////////////////////////////////////////////////////
	
	
	// Writes a standardized syntax error message to the standard output and aborts the program
	void Parser::SyntaxError(const char *cMsg, bool bLineNbr)
	{
		if (bLineNbr)
			cerr << "Error in Ln " << iMyLineNumber << ", Col " << iMyColumnNumber << ": " << cMsg << endl;
		else
			cerr << "Error: " << cMsg << endl;
		exit(EXIT_FAILURE);
	}
	
	
	// Writes a standardized syntax warning message to the standard output and continues the program
	void Parser::SyntaxWarning(const char *cMsg, bool bLineNbr)
	{
		if (bLineNbr)
			cerr << "Warning in Ln " << iMyLineNumber << ", Col " << iMyColumnNumber << ": " << cMsg << endl;
		else
			cerr << "Warning: " << cMsg << endl;
		++iMyNbrOfWarnings;
	}
	
	
	/*
	***Description***
	The method fills the internal file buffer cMyBuffer and aborts the program if EOF is reached
	before the end-token was found.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::fillBuffer()
	{
		if (!MyFile.eof()) {
			// Reinitialize buffer; this is necessary because of the 'read'-method
			for(int i=0; i < IF_BUFFER_SIZE; i++)
				cMyBuffer[i] = '\0';
	
			MyFile.read(cMyBuffer, IF_BUFFER_SIZE);
			iMyBufferPos = 0;
		}
		else {
			bMyEOF = true;
	
			// If end-token was not found yet
			if (iMyEndOfAtoms == 0)
				SyntaxError("Unexpected end of file.", true);
		}
	}
	
	
	/*
	***Description***
	The method reads text from the input file that can be ignored (e.g., space, newline, comments, ...).
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readIgnoreText()
	{
		bool bEndIgnore = false;
	
		do {
			switch (cMyBuffer[iMyBufferPos]) {
				case '\0':	// End of buffer reached -> refill
					fillBuffer();
					break;
				case ' ':	// Ignore spaces
					++iMyBufferPos;
					++iMyColumnNumber;
					break;
				case 9:		// Ignore tabulators
					++iMyBufferPos;
					++iMyColumnNumber;
					while(iMyColumnNumber%8 != 1)
						++iMyColumnNumber;
					break;
				case '\n':	// Ignore newlines
					++iMyBufferPos;
					++iMyLineNumber;
					iMyColumnNumber = 1;
					break;
				case '%':	// Ignore comments
					do {
						++iMyBufferPos;
						++iMyColumnNumber;
						if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
							fillBuffer();
					} while ((cMyBuffer[iMyBufferPos] != '\n') && !bMyEOF);
					// Until newline or end of file
					break;
				default:
					bEndIgnore = true;
			}
		} while (!bEndIgnore && !bMyEOF);
	}
	
	
	/*
	***Description***
	The method reads an identifier (i.e., name of an atom or a variable) from the input file and
	returns a pointer to this identifier. Identifiers may consist of letters (upper and lower case,
	case sensitive) A-Z and a-z, numbers 0-9, underscore _, and colon :.
	
	INPUT:
	OUTPUT:	return: Pointer to a character array containing the identifier
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	char *Parser::readIdentifier()
	{
		bool bEndIdentifier = false;
		char cIdentBuffer[MAX_IDENTIFIER_SIZE + 1], *cIdentifier = NULL;
		int iIdentifierSize = 0;
	
		readIgnoreText();
		do {
			if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
					fillBuffer();
			else {
				if (((cMyBuffer[iMyBufferPos] >= 'A') && (cMyBuffer[iMyBufferPos] <= 'Z')) ||
					((cMyBuffer[iMyBufferPos] >= 'a') && (cMyBuffer[iMyBufferPos] <= 'z')) ||
					((cMyBuffer[iMyBufferPos] >= '0') && (cMyBuffer[iMyBufferPos] <= '9')) ||
					(cMyBuffer[iMyBufferPos] == '_') || (cMyBuffer[iMyBufferPos] == ':')) {
					
					// Copy identifier from input file in temporary identifier buffer
					cIdentBuffer[iIdentifierSize++] = cMyBuffer[iMyBufferPos++];
					++iMyColumnNumber;
	
					if (iIdentifierSize > MAX_IDENTIFIER_SIZE)
						SyntaxError("Maximum length of identifier exceeded.", true);
				}
				else
					bEndIdentifier = true;
			}
		} while (!bEndIdentifier);
		// Until character was found that is not allowed in an identifier
	
		cIdentBuffer[iIdentifierSize] = '\0';
		if(iIdentifierSize == 0)
			SyntaxError("Illegal or missing identifier.", true);
	
		// Allocate character array for identifier and copy identifier from the identifier buffer in this array
		cIdentifier = new char[iIdentifierSize + 1];
		if (cIdentifier == NULL)
			writeErrorMsg("Error assigning memory.", "Parser::readIdentifier");
	
		return strcpy(cIdentifier, cIdentBuffer);
	}
	
	
	/*
	***Description***
	The method tries to read the identifier given as argument and returns true if the identifier 
	could be read; otherwise false.
	
	INPUT:	cIdent: Pointer to character array representing the identifier
	OUTPUT:	return: true if the identifier could be read;  otherwise false
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool Parser::readIdentifier(const char *cIdent)
	{
		int i = 0;
	
		readIgnoreText();
		while (cIdent[i] != '\0') {
			if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
				fillBuffer();
			if (cMyBuffer[iMyBufferPos] != cIdent[i++])
				return false;
			++iMyBufferPos;
			++iMyColumnNumber;
		}
	
		return true;
	}
	
	
	/*
	***Description***
	The method reads a number with at most 4 digits.
	
	INPUT:
	OUTPUT: return: Arity number
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	int Parser::readArity()
	{
		bool bEndArity = false;
		int iArity = 0, iNbrOfDigits = 0;
	
		readIgnoreText();
		do {
			if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
					fillBuffer();
			else {
				if ((cMyBuffer[iMyBufferPos] >= '0') && (cMyBuffer[iMyBufferPos] <= '9')) {
					iArity *= 10;
					iArity += (int)cMyBuffer[iMyBufferPos]-48;
					++iMyBufferPos;
					++iMyColumnNumber;
					++iNbrOfDigits;
	
					// Maximum number of digits is restricted to 4 (Arity: 1 - 9999)
					if (iNbrOfDigits == 4)
						bEndArity = true;
				}
				else
					bEndArity = true;
			}
		} while (!bEndArity);
		// Until a non-numerical character was found or the maximum number of digits was exceeded
	
		if(iArity == 0)
			SyntaxError("Illegal arity number.", true);
	
		return iArity;
	}
	
	
	/*
	***Description***
	The method reads variable definitions and stores them in the internal data structures.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readVarDefs()
	{
		char *cIdentifier = NULL, cErrorMsg[MAX_IDENTIFIER_SIZE + 32];
		vector<char *>::iterator VectorIter;
	
		do {
			cIdentifier = readIdentifier();
	
			// Check whether the variable is already defined
			VectorIter = MyDefVariables.begin();
			while ((VectorIter != MyDefVariables.end()) && (strcmp(*VectorIter, cIdentifier) != 0))
				++VectorIter;
	
			// If the variable is not defined yet
			if (VectorIter == MyDefVariables.end()) {
				// Store the variable identifier
				MyDefVariables.push_back(cIdentifier);
	
				// Initialize usage of the variable
				MyDefVarUsed.push_back(false);
			}
			else {
				strcpy(cErrorMsg, "Variable \"");
				strcat(cErrorMsg, cIdentifier);
				strcat(cErrorMsg, "\" is already defined.");
				SyntaxError(cErrorMsg, true);
			}
		} while (readSeparator());
	}
	
	
	/*
	***Description***
	The method reads atom definitions and stores them in the internal data structures.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readAtomDefs()
	{
		char *cIdentifier = NULL, cErrorMsg[MAX_IDENTIFIER_SIZE + 32];
		vector<char *>::iterator VectorIter;
	
		do {
			cIdentifier = readIdentifier();
	
			// Check whether the atom is already defined
			VectorIter = MyDefAtoms.begin();
			while ((VectorIter != MyDefAtoms.end()) && (strcmp(*VectorIter, cIdentifier) != 0))
				++VectorIter;
	
			// If the atom is not defined yet
			if (VectorIter == MyDefAtoms.end())
				// Store the variable identifier
				MyDefAtoms.push_back(cIdentifier);
			else {
				strcpy(cErrorMsg, "Atom \"");
				strcat(cErrorMsg, cIdentifier);
				strcat(cErrorMsg, "\" is already defined.");
				SyntaxError(cErrorMsg, true);
			}
	
			// Read separator between atom identifier and arity
			readIgnoreText();
			if(!readIdentifier("/"))
				SyntaxError("Illegal character.", true);
			
			// Read arity of the actual atom
			MyArities.push_back(readArity());
	
			// Initialize usage of the atom
			MyDefAtomUsed.push_back(false);
		} while (readSeparator());
	}
	
	
	/*
	***Description***
	The method tries to read a token for opening a definition and returns true if the token could 
	be read; otherwise false.
	
	INPUT:
	OUTPUT: return: true if definition token could be read; otherwise false
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool Parser::readOpenDef()
	{
		return readIdentifier("<");
	}
	
	
	/*
	***Description***
	The method tries to read a token for closing a definition and aborts the program if the 
	expected token could not be read.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readCloseDef()
	{
		if(!readIdentifier(">"))
			SyntaxError("Illegal character.", true);
	}
	
	
	/*
	***Description***
	The method tries to read a token for opening an atom body and aborts the program if the 
	expected token could not be found.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readOpenBody()
	{
		if(!readIdentifier("("))
			SyntaxError("Illegal character.", true);
	}
	
	
	/*
	***Description***
	The method tries to read a token for closing an atom body and aborts the program if the 
	expected token could not be found.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readCloseBody()
	{
		if(!readIdentifier(")"))
			SyntaxError("Illegal character.", true);
	}
	
	
	/*
	***Description***
	The method reads a separator of a list of tokens and returns true if such a separator could 
	be found; otherwise false.
	
	INPUT:
	OUTPUT:	return: true if the separator could be read; otherwise false
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool Parser::readSeparator()
	{
		return readIdentifier(",");
	}
	
	
	/*
	***Description***
	The method tries to read the end-token indicating the end of the atom list and aborts the 
	program if the expected token could not be found.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readEnd()
	{
		if(readIdentifier("."))
			// Remember the line number where the end-token was found for error messages
			iMyEndOfAtoms = iMyLineNumber;
		else
			SyntaxError("Illegal character.", true);
	}
	
	
	/*
	***Description***
	The method reads variable and atom definitions and stores the corresponding information
	in the internal data structures.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (14.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readDefinitions()
	{
		bool bVarDef, bAtomDef;
	
		// While a definition block '<...>' can be found
		while(readOpenDef()) {
			bVarDef = bAtomDef = false;
	
			if(readIdentifier("def")) {
				if(readIdentifier("Var"))  // Read "defVar" identifier
					bVarDef = true;
				else
					if(readIdentifier("Rel"))  // Read "defRel" identifier
						bAtomDef = true;
					else
						SyntaxError("Unknown identifier.", true);
			}
			else
				SyntaxError("Unknown identifier.", true);
	
			readIgnoreText();
			// Check whether the definition identifier is followed by a colon
			if(!readIdentifier(":")) 
				SyntaxError("Illegal character.", true);
	
			if(bVarDef)
				readVarDefs();  // Read variable definitions
			else
				if(bAtomDef)
					readAtomDefs();  // Read relation definitions
	
			readCloseDef();
		}
	}
	
	
	/*
	***Description***
	The method reads an atom (including its variables) from the file and stores the information 
	in the internal data structures.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (15.12.04, MS)
	Comments: Checking definitions added
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readAtom()
	{
	    char *cIdentifier = NULL;
		vector<char *>::iterator VectorIter;
		set<int> *Set = NULL;
		char cErrorMsg[MAX_IDENTIFIER_SIZE + 64];
		int iDefAtom = 0, iArity = 0;
	
		cIdentifier = readIdentifier();
	
		// Check if the same atom identifier was already used before
		for (VectorIter=MyAtoms.begin(); VectorIter != MyAtoms.end(); VectorIter++)
			if (strcmp(*VectorIter, cIdentifier) == 0) {
				strcpy(cErrorMsg, "Atom identifier \"");
				strcat(cErrorMsg, cIdentifier);
				strcat(cErrorMsg, "\" occurs the second time.");
				SyntaxError(cErrorMsg, true);
			}
	
		if (bMyDef) {
			// Check whether the atom is defined
			VectorIter=MyDefAtoms.begin();
			while ((VectorIter != MyDefAtoms.end()) && (strcmp(*VectorIter, cIdentifier) != 0)) {
				++VectorIter;
				++iDefAtom;
			}
	
			// If the atom is not defined
			if (VectorIter == MyDefAtoms.end()) {
				strcpy(cErrorMsg, "Undefined atom \"");
				strcat(cErrorMsg, cIdentifier);
				strcat(cErrorMsg, "\".");
				SyntaxError(cErrorMsg, true);
			}
	
			// Update atom usage
			MyDefAtomUsed[iDefAtom] = true;
	
			// Store the atom identifier
			MyAtoms.push_back(MyDefAtoms[iDefAtom]);
			delete cIdentifier;
		}
		else
			// Store the atom identifier
			MyAtoms.push_back(cIdentifier);
	
		// Allocate a set of variables for the atom
		Set = new set<int>;
		if (Set == NULL)
			writeErrorMsg("Error assigning memory.", "Parser::readAtom");
		MyAtomVars.push_back(Set);
	
		// Allocate a set of neighbours for the atom
		Set = new set<int>;
		if (Set == NULL)
			writeErrorMsg("Error assigning memory.", "Parser::readAtom");
		MyAtomNeighbours.push_back(Set);
	
		// Read the atom body
		readOpenBody();
		do {
			readVariable();
			++iArity;
		} while (readSeparator());
		readCloseBody();
	
		// Check whether the number of arguments equals the defined arity
		if (bMyDef && (MyArities[iDefAtom] != iArity))
			SyntaxError("Illegal number of arguments.", true);
	}
	
	
	/*
	***Description***
	The method reads a variable from the file and stores the information in the internal data 
	structures.
	
	INPUT:
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (15.12.04, MS)
	Comments: Checking definitions added
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::readVariable()
	{
		int iVar, iDefVar, iAtom;
		char *cIdentifier = NULL;
		vector<char *>::iterator VectorIter;
		set<int>::iterator SetIter;
		set<int> *Set = NULL;
		pair<set<int>::iterator, bool> SetInsertRet;
		char cErrorMsg[(2*MAX_IDENTIFIER_SIZE) + 64];
	
		iAtom = MyAtoms.size()-1;
		cIdentifier = readIdentifier();
	
		// Check if the same variable was already used before
		iVar = iDefVar = 0;
		VectorIter = MyVariables.begin();
		while ((VectorIter != MyVariables.end()) && (strcmp(*VectorIter, cIdentifier) != 0)) {
			++VectorIter;
			++iVar;
		}
	
		// If the variable did not occur previously
	    if (VectorIter == MyVariables.end()) {
	
			if (bMyDef) {
				// Check whether the variable is defined
				VectorIter = MyDefVariables.begin();
				while ((VectorIter != MyDefVariables.end()) && (strcmp(*VectorIter, cIdentifier) != 0)) {
					++VectorIter;
					++iDefVar;
				}
	
				// If the variable is not defined
				if (VectorIter == MyDefVariables.end()) {
					strcpy(cErrorMsg, "Undefined variable \"");
					strcat(cErrorMsg, cIdentifier);
					strcat(cErrorMsg, "\".");
					SyntaxError(cErrorMsg, true);
				}
	
				// Update variable usage
				MyDefVarUsed[iDefVar] = true;
	
				// Store the variable identifier
				MyVariables.push_back(MyDefVariables[iDefVar]);
				delete cIdentifier;
			}
			else
				// Store the variable identifier
				MyVariables.push_back(cIdentifier);
	
			// Allocate a set of atoms for the variable
			Set = new set<int>;
			if (Set == NULL)
				writeErrorMsg("Error assigning memory.", "Parser::readVariable");
			MyVarAtoms.push_back(Set);
	
			// Allocate a set of neighbours for the variable
			Set = new set<int>;
			if (Set == NULL)
				writeErrorMsg("Error assigning memory.", "Parser::readVariable");
			MyVarNeighbours.push_back(Set);
		}
	
		// Add variable to the set of variables of the actual atom
	    SetInsertRet = MyAtomVars[iAtom]->insert(iVar);
		// Write syntax warning if the variable occurs multiple times in the actual atom
		if (SetInsertRet.second == false) {
			strcpy(cErrorMsg, "Multiple occurrences of variable \"");
			strcat(cErrorMsg, cIdentifier);
			strcat(cErrorMsg, "\" in atom \"");
			strcat(cErrorMsg, MyAtoms.back());
			strcat(cErrorMsg, "\".");
			SyntaxWarning(cErrorMsg, true);
		}
	
		// Update the neighbourhood relation between the actual atom and the atoms already stored before
		for (SetIter=MyVarAtoms[iVar]->begin(); SetIter != MyVarAtoms[iVar]->end(); SetIter++)
			if (*SetIter != iAtom) {
				MyAtomNeighbours[*SetIter]->insert(iAtom);
				MyAtomNeighbours[iAtom]->insert(*SetIter);
			}
	
		// Update the neighbourhood relation between the actual variable and the variables already stored before
		for (SetIter=MyAtomVars[iAtom]->begin(); SetIter != MyAtomVars[iAtom]->end(); SetIter++)
			if (*SetIter != iVar) {
				MyVarNeighbours[*SetIter]->insert(iVar);
				MyVarNeighbours[iVar]->insert(*SetIter);
			}
	
		// Add actual atom to the set of atoms of the variable
	 	MyVarAtoms[iVar]->insert(iAtom);
	}
	
	
	/*
	***Description***
	The method parses the hypergraph in a given file and stores the information in the internal
	data structures. Syntax errors are written to the standard error stream and the program is 
	aborted. Syntax warnings are written to the standard error stream and the program continues.
	
	INPUT:	cNameOfFile: Name of the file to be parsed
	OUTPUT:
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (14.12.04, MS)
	Comments: Calls for reading definitions added.
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Parser::parseFile(const char *cNameOfFile)
	{
		int iDefVar = 0, iDefAtom = 0;
		char cErrorMsg[96], cLineNumber[(sizeof(int)*8)+1];
		vector<bool>::iterator bVectorIter;
		vector<char *>::iterator cVectorIter;
	
		// Open input file stream
		MyFile.open(cNameOfFile, ios::in);
	
		// Check if file opening was successful
		if(!MyFile.is_open())
			SyntaxError("Error opening file.");
	
		// Read definitions of variables and relations
		readDefinitions();
	
		// Delete all definition informations
		if (!bMyDef) {
			for (cVectorIter=MyDefAtoms.begin(); cVectorIter != MyDefAtoms.end(); cVectorIter++)
				delete *cVectorIter;
			for (cVectorIter=MyDefVariables.begin(); cVectorIter != MyDefVariables.end(); cVectorIter++)
				delete *cVectorIter;
			MyDefAtoms.clear();
			MyDefVariables.clear();
			MyArities.clear();
			MyDefAtomUsed.clear();
			MyDefVarUsed.clear();
		}
	
		// Read atoms until the separator cannot be found
		do {
			readAtom();
		} while(readSeparator());
		// Read the end-token
		readEnd();
		// Read the text after the end-token
		readIgnoreText();
	
		// Close input file stream
		MyFile.close();
	
		// Write syntax warning if EOF does not appear during reading the ignore text, i.e., there appears
		// some text after the end-token was found
		if(!bMyEOF) {
			strcpy(cErrorMsg, "Illegal character after termination symbol \".\" in line ");
			strcat(cErrorMsg, uitoa(iMyEndOfAtoms, cLineNumber));
			strcat(cErrorMsg, ".");
			SyntaxWarning(cErrorMsg, true);
		}
	
		if (bMyDef) {
			// Write a syntax warning if a defined atom was not used
			for (bVectorIter = MyDefAtomUsed.begin(); bVectorIter != MyDefAtomUsed.end(); bVectorIter++, iDefAtom++)
				if (!*bVectorIter) {
					strcpy(cErrorMsg, "Atom \"");
					strcat(cErrorMsg, MyDefAtoms[iDefAtom]);
					strcat(cErrorMsg, "\" is not used.");
					SyntaxWarning(cErrorMsg);
				}
	
			// Write a syntax warning if a defined variable was not used
			for (bVectorIter = MyDefVarUsed.begin(); bVectorIter != MyDefVarUsed.end(); bVectorIter++, iDefVar++)
				if (!*bVectorIter) {
					strcpy(cErrorMsg, "Variable \"");
					strcat(cErrorMsg, MyDefVariables[iDefVar]);
					strcat(cErrorMsg, "\" is not used.");
					SyntaxWarning(cErrorMsg);
				}
		}
	}
	
	
	// Returns the number of atoms currently stored in the data structures
	int Parser::getNbrOfAtoms()
	{
		return MyAtoms.size();
	}
	
	
	// Returns the number of variables currently stored in the data structures
	int Parser::getNbrOfVars()
	{
		return MyVariables.size();
	}
	
	
	// Returns the number of variables in a given atom
	int Parser::getNbrOfVars(int iAtom)
	{
		if (iAtom >= (int)MyAtomVars.size()) {
			writeErrorMsg("Position not available.", "Parser::getNbrOfVars");
			return 0;
		}
		else
			return MyAtomVars[iAtom]->size();
	}
	
	
	// Returns the number of atoms to which a given variable belongs
	int Parser::getNbrOfAtoms(int iVar)
	{
		if (iVar >= (int)MyVarAtoms.size()) {
			writeErrorMsg("Position not available.", "Parser::getNbrOfAtoms");
			return 0;
		}
		else
			return MyVarAtoms[iVar]->size();
	}
	
	
	// Returns the number of atoms that are neighbours (i.e., have common variables) of a given atom
	int Parser::getNbrOfAtomNeighbours(int iAtom)
	{
		if (iAtom >= (int)MyAtomNeighbours.size()) {
			writeErrorMsg("Position not available.", "Parser::getNbrOfAtomNeighbours");
			return 0;
		}
		else
			return MyAtomNeighbours[iAtom]->size();
	}
	
	
	// Returns the number of variables that are neighbours (i.e., have common atoms) of a given variable
	int Parser::getNbrOfVarNeighbours(int iVar)
	{
		if (iVar >= (int)MyVarNeighbours.size()) {
			writeErrorMsg("Position not available.", "Parser::getNbrOfVarNeighbours");
			return 0;
		}
		else
			return MyVarNeighbours[iVar]->size();
	}
	
	
	// Returns the number of syntax warnings
	int Parser::getNbrOfWarnings()
	{
		return iMyNbrOfWarnings;
	}
	
	
	// Returns the name of the iAtom-th atom
	char *Parser::getAtom(int iAtom)
	{
		if (iAtom >= (int)MyAtoms.size()) {
			writeErrorMsg("Position not available.", "Parser::getAtom");
			return NULL;
		}
		else
			return MyAtoms[iAtom];
	}
	
	
	// Returns the name of the iVar-th variable
	char *Parser::getVariable(int iVar)
	{
		if (iVar >= (int)MyVariables.size()) {
			writeErrorMsg("Position not available.", "Parser::getVariable");
			return NULL;
		}
		else
			return MyVariables[iVar];
	}
	
	
	/*
	***Description***
	The method returns successively the indices of variables V belonging to atoms A in the
	following order: V0 of A0, V1 of A0, V2 of A0, ..., Vn0 of A0, V0 of A1, V1 of A1, V2 of A1, ...,
	Vn1 of A1, V0 of A2, V1 of A2, ... Together with the method getNbrAtomVars, it can be used
	to get the variables of each atom.
	
	INPUT:	bReset: true, if the next variable should be the first variable of the first atom
	OUTPUT: return: Index of the next variable
	 
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (10.01.05, MS)
	Comments: Bug of empty sets fixed.
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	int Parser::getNextAtomVar(bool bReset)
	{
		static vector<set<int> *>::iterator VectorIter;
		static set<int>::iterator SetIter;
		static bool bFirstCall = true;
	
		// Initialize resp. reset the iterator
		if(bFirstCall || bReset) {
			VectorIter = MyAtomVars.begin();
			if(VectorIter == MyAtomVars.end())
				writeErrorMsg("Position not available.", "Parser::getNextAtomVar");
			SetIter = (*VectorIter)->begin();
			bFirstCall = false;
		}
		else
			++SetIter;  // Increment the iterator
	
		while(SetIter == (*VectorIter)->end()) {
			if(++VectorIter == MyAtomVars.end())
				writeErrorMsg("Position not available.", "Parser::getNextAtomVar");
			SetIter = (*VectorIter)->begin();
		}
	
		return *SetIter;  // Return the next neighbour index
	}
	
	
	/*
	***Description***
	The method returns successively the indices of atoms A to which the variables V belong in
	the following order: A0 of V0, A1 of V0, A2 of V0, ..., An0 of V0, A0 of V1, A1 of V1, A2 of V1, ...,
	An1 of V1, A0 of V2, A1 of V2, ... Together with the method getNbrVarAtoms, it can be used
	to get the atoms to which each variable belongs.
	
	INPUT:	bReset: true, if the next atom should be the first atom of the first variable
	OUTPUT: return: Index of the next atom
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (10.01.05, MS)
	Comments: Bug of empty sets fixed.
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	int Parser::getNextVarAtom(bool bReset)
	{
		static vector<set<int> *>::iterator VectorIter;
		static set<int>::iterator SetIter;
		static bool bFirstCall = true;
	
		// Initialize resp. reset the iterator
		if(bFirstCall || bReset) {
			VectorIter = MyVarAtoms.begin();
			if(VectorIter == MyVarAtoms.end())
				writeErrorMsg("Position not available.", "Parser::getNextVarAtom");
			SetIter = (*VectorIter)->begin();
			bFirstCall = false;
		}
		else
			++SetIter;  // Increment the iterator
	
		while(SetIter == (*VectorIter)->end()) {
			if(++VectorIter == MyVarAtoms.end())
				writeErrorMsg("Position not available.", "Parser::getNextVarAtom");
			SetIter = (*VectorIter)->begin();
		}
	
		return *SetIter;  // Return the next neighbour index
	}
	
	
	/*
	***Description***
	The method returns successively the indices of neighbours N of the atom A in the following
	order: N0 of A0, N1 of A0, N2 of A0, ..., Nn0 of A0, N0 of A1, N1 of A1, N2 of A1, ..., Nn1 of A1, 
	N0 of A2, N1 of A2, ... Together with the method getNbrOfAtomNeighbours, it can be used to get the 
	neighbours of each atom.
	
	INPUT:	bReset: true, if the next atom should be the first neighbour of the first atom
	OUTPUT: return: Index of the next neighbour atom
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (10.01.05, MS)
	Comments: Bug of empty sets fixed.
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	int Parser::getNextAtomNeighbour(bool bReset)
	{
		static vector<set<int> *>::iterator VectorIter;
		static set<int>::iterator SetIter;
		static bool bFirstCall = true;
	
		// Initialize resp. reset the iterator
		if(bFirstCall || bReset) {
			VectorIter = MyAtomNeighbours.begin();
			if(VectorIter == MyAtomNeighbours.end())
				writeErrorMsg("Position not available.", "Parser::getNextAtomNeighbour");
			SetIter = (*VectorIter)->begin();
			bFirstCall = false;
		}
		else
			++SetIter;  // Increment the iterator
	
		while(SetIter == (*VectorIter)->end()) {
			if(++VectorIter == MyAtomNeighbours.end())
				writeErrorMsg("Position not available.", "Parser::getNextAtomNeighbour");
			SetIter = (*VectorIter)->begin();
		}
	
		return *SetIter;  // Return the next neighbour index
	}
	
	
	/*
	***Description***
	The method returns successively the indices of neighbours N of the variable V in the following
	order: N0 of V0, N1 of V0, N2 of V0, ..., Nn0 of V0, N0 of V1, N1 of V1, N2 of V1, ..., Nn1 of V1, 
	N0 of V2, N1 of V2, ... Together with the method getNbrOfVarNeighbours, it can be used to get the 
	neighbours of each variable.
	
	INPUT:	bReset: true, if the next variable should be the first neighbour of the first variable
	OUTPUT: return: Index of the next neighbour variable
	
	***History***
	Written: (02.12.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (10.01.05, MS)
	Comments: Bug of empty sets fixed.
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	int Parser::getNextVarNeighbour(bool bReset)
	{
		static vector<set<int> *>::iterator VectorIter;
		static set<int>::iterator SetIter;
		static bool bFirstCall = true;
	
		// Initialize resp. reset the iterator
		if(bFirstCall || bReset) {
			VectorIter = MyVarNeighbours.begin();
			if(VectorIter == MyVarNeighbours.end())
				writeErrorMsg("Position not available.", "Parser::getNextVarNeighbour");
			SetIter = (*VectorIter)->begin();
			bFirstCall = false;
		}
		else
			++SetIter;  // Increment the iterator
	
		while(SetIter == (*VectorIter)->end()) {
			if(++VectorIter == MyVarNeighbours.end())
				writeErrorMsg("Position not available.", "Parser::getNextVarNeighbour");
			SetIter = (*VectorIter)->begin();
		}
	
		return *SetIter;  // Return the next neighbour index
	}

} // namespace sharp
