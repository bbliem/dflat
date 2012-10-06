#include <gringo/grounder.h>
#include <gringo/parser.h>

#include "ClaspInputReader.h"

ClaspInputReader::ClaspInputReader(Streams& streams, GringoOutputProcessor& output)
	: output(output)
{
	config.incBase = false;
	config.incBegin = 1;
	config.incEnd = config.incBegin + 1;

	termExpansion.reset(new TermExpansion());
	grounder.reset(new Grounder(&output, false /* verbosity */, termExpansion));
	parser.reset(new Parser(grounder.get(), config, streams, false));
}

ClaspInputReader::~ClaspInputReader()
{
}

ClaspInputReader::Format ClaspInputReader::format() const
{
	return SMODELS;
}

bool ClaspInputReader::read(ApiPtr api, int)
{
	output.setProgramBuilder(api.api);
	output.initialize();

	parser->parse();
	grounder->analyze();
	grounder->ground();

	output.finalize();

	return true;	
}

void ClaspInputReader::addMinimize(Clasp::MinimizeBuilder&, ApiPtr)
{
}

void ClaspInputReader::getAssumptions(Clasp::LitVec&)
{
}
