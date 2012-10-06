#include "GringoOutputProcessor.h"

namespace cyclic_ordering {

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	::GringoOutputProcessor::printSymbolTableEntry(atom, arity, name);

	if(arity == 0)
		return;

	if(name == "n") {
		assert(arity == 1);
		std::stringstream arg;
		(vals_.begin() + atom.second)->print(s_, arg);
		arg >> n;
	}
	else if(name == "map") { // FIXME: I'm dirty
		assert(arity == 2);
		std::pair<std::stringstream,std::stringstream> args; // The two arguments
		ValVec::const_iterator k = vals_.begin() + atom.second;
		(k++)->print(s_, args.first);
		k->print(s_, args.second);
		VertexAndNumber vi;
		vi.first = std::strtol(args.first.str().c_str()+1, 0, 0); // +1 because of leading 'v'
		args.second >> vi.second;
		map.push_back(MappingAndSymbolTableKey(vi, atom.first));
	}
}

} // namespace cyclic_ordering
