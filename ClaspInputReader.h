#pragma once

#include <clasp/reader.h>
#include <gringo/inclit.h>

//class GringoOutputProcessor;
#include "GringoOutputProcessor.h"

class ClaspInputReader : public Clasp::Input {
public:
	ClaspInputReader(Streams&, GringoOutputProcessor&);
	virtual ~ClaspInputReader();

	// Clasp::Input interface
	virtual Format format() const;
	virtual bool read(ApiPtr, int);
	virtual void addMinimize(Clasp::MinimizeBuilder&, ApiPtr);
	virtual void getAssumptions(Clasp::LitVec&);

private:
	typedef std::auto_ptr<Grounder> GrounderPtr;
	typedef std::auto_ptr<Parser> ParserPtr;

	TermExpansionPtr termExpansion;
	GrounderPtr grounder;
	ParserPtr parser;
	IncConfig config;
	GringoOutputProcessor& output;
};
