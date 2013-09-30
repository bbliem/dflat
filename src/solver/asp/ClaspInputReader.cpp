/*
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gringo/grounder.h>
#include <gringo/parser.h>

#include "ClaspInputReader.h"

namespace solver { namespace asp {

ClaspInputReader::ClaspInputReader(Streams& streams, GringoOutputProcessor& output)
	: output(output)
{
	config.incBase = false;
	config.incBegin = 1;
	config.incEnd = config.incBegin + 1;

	termExpansion.reset(new TermExpansion);
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

bool ClaspInputReader::read(ApiPtr api, uint32)
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

}} // namespace solver::asp
