#pragma once

#include <boost/dynamic_bitset.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

template<typename Set>
class PowerSet
{
public:
	typedef std::pair<Set,Set> Subset;

	PowerSet(const Set& base) : base(base) {}

	class const_iterator
	{
	public:
		const_iterator(const Set& base, unsigned long pos = 0) : base(base), elements(base.size()+1, pos) {}

		std::pair<Set,Set> operator*() const;
		const_iterator& operator++();
		bool operator!=(const const_iterator&) const;
	private:
		const Set& base;
		typedef boost::dynamic_bitset<> bitset;
		bitset elements;
	};

	const_iterator begin() const { return const_iterator(base); }
	const_iterator end() const { return const_iterator(base, 1<<base.size()); }
private:
	const Set& base;
};

template<typename Set>
inline std::pair<Set,Set> PowerSet<Set>::const_iterator::operator*() const {
	assert(elements.to_ulong() < 1ul<<base.size());
	std::pair<Set,Set> posAndNeg;
	boost::dynamic_bitset<>::size_type i = 0;
	foreach(const typename Set::value_type& element, base) {
		if(elements.test(i++))
			posAndNeg.first.insert(element);
		else
			posAndNeg.second.insert(element);
	}
	return posAndNeg;
}

template<typename Set>
inline typename PowerSet<Set>::const_iterator& PowerSet<Set>::const_iterator::operator++() {
	elements = bitset(elements.size(), elements.to_ulong()+1);
	return *this;
}

template<typename Set>
inline bool PowerSet<Set>::const_iterator::operator!=(const typename PowerSet<Set>::const_iterator& it) const {
	return elements != it.elements || &base != &it.base;
}
