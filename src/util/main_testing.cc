#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

#include "util/debug.hpp"
#include "util/permutation.hpp"


#include "util/static_data.hpp"
template<unsigned N>
void prepMaxPerm(std::array<int, N>& v) {

	for(unsigned i=0; i<N; i++)
		v[i] = N - 1 - i;

}

/*
template<unsigned N>
bool doTest() {
	
	std::array<int, N> permIn, permOut;
	prepMaxPerm<N>(permIn);
	
	using lehmer_t = typename mjon661::SafeLehmerType<N>::type;
	
	lehmer_t l = mjon661::toLehmerCode<N, lehmer_t>(permIn);
	
	mjon661::fromLehmerCode<N, lehmer_t>(l, permOut);
	
	
	//std::cout << N << " " << l << " " << mjon661::SafeLehmerType<N>::maxValue() << "\n";

	return permIn == permOut && l == mjon661::SafeLehmerType<N>::maxValue();
	
}

*/

#define TRACE(x) std::cout << #x << "\n";



/*
namespace mjon661 {

template<typename T>
void printArray(T const& p) {
	for(unsigned i=0; i<p.size(); i++)
		std::cout << p[i] << " ";
	std::cout << "\n";
}

template<unsigned N, unsigned Sz>
void doTest2() {
	
	auto f = static_data::Factorial<N, Sz>::getValue();
	
	if(factorial<uint64_t>(N, Sz) != f) {
		std::cout << factorial<uint64_t>(N, Sz) << "\n";
		std::cout << std::to_string(f) << "\n";
	}
	gen_assert(factorial<uint64_t>(N, Sz) == f);
	
	Permutation<N, Sz> perm;
	
	for(int i=0; i<(int)Sz; i++)
		perm[i] = N-1-i;
	
	gen_assert(perm.valid());
	
	using Rank_t = typename Permutation<N, Sz>::Rank_t;
	
	Rank_t rank = perm.getRank();
	
	Permutation<N, Sz> perm_out;
	perm_out.setRank(rank);
	
	if(perm != perm_out) {
		std::cout << std::to_string(rank) << "\n";
		std::cout << perm << "\n" << perm_out << "\n";
	}
	gen_assert(perm == perm_out);

	if(rank != factorial<Rank_t>(N, Sz) - 1) {
		std::cout << std::to_string(rank) << "\n";
		std::cout << std::to_string(factorial<Rank_t>(N, Sz) - 1) << "\n";
		std::cout << perm << "\n";
	}
	
	gen_assert(rank == factorial<Rank_t>(N, Sz) - 1);


	Permutation<N, Sz> perm2(perm.begin(), perm.end());
	gen_assert(perm == perm2);
}


template<unsigned N, unsigned S>
void doTestRec() {
	doTestRec< S == 1 ? N-1 : N,  S == 1 ? N-1 : S-1 >();
	
	doTest2<N, S>();
	
	//std::cout << N << " " << S << "\n";
	
}

template<>
void doTestRec<1, 1>() {}

}
*/

int main() {
	using namespace mjon661;
	
	//mjon661::doTestRec<5,5>();

	using Rank_t = typename Permutation<10,10>::Rank_t;
	
	Permutation<10,10> perm_in, perm_out;

	perm_in.setRank(0);

	int count = 0;
	Rank_t oldRank = 0;
	bool first = true;
	
	std::cout << perm_in << "\n";
	
	do {
		Rank_t r = perm_in.getRank();
		
		if(!first) {
			
			gen_assert(r == oldRank + 1);
		} else {
			std::cout << "first rank: " << r << "\n";
		}
		
		first = false;
		oldRank = r;
		
		perm_out.setRank(r);
		
		gen_assert(perm_in == perm_out);
		
		count++;
	} while(std::next_permutation(perm_in.begin(), perm_in.end()));


	std::cout << "last rank: " << oldRank << "\n";
	std::cout << "count: " << count << "\n";
}
