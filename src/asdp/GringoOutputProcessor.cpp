#include "GringoOutputProcessor.h"

namespace asdp {

GringoOutputProcessor::GringoOutputProcessor(const sharp::Problem& problem)
	: problem(problem)
{
}

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	::GringoOutputProcessor::printSymbolTableEntry(atom, arity, name);

	if(arity == 2 && name == "map") {
		std::pair<std::ostringstream,std::ostringstream> args; // The two arguments
		ValVec::const_iterator k = vals_.begin() + atom.second;
		(k++)->print(s_, args.first);
		k->print(s_, args.second);

		map.push_back(MappingAndSymbolTableKey(StringPair(args.first.str(), args.second.str()), atom.first));
	}
}

} // namespace asdp
