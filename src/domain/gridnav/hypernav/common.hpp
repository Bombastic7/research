#pragma once


#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <cmath>
#include <random>
#include <utility>
#include <tuple>
#include <algorithm>


#include "util/math.hpp"


namespace mjon661 { namespace gridnav { namespace hypernav {


	template<unsigned N>
	using StateN = std::array<unsigned, N>;
	
	using PackedStateN = unsigned;
	
	template<long unsigned N>
	void doPrettyPrintState(StateN<N> const& s, std::ostream& out) {
			out << "[ ";
			for(unsigned i=0; i<N; i++)
				out << s[i] << " ";
			out << "]";
	}

	
	
	template<long unsigned N>
	PackedStateN doPackState(StateN<N> const& s, std::array<unsigned,N> const& pDimSz) {
		PackedStateN pkd = 0;
		unsigned rdx = 1;
		
		for(unsigned i=0; i<N; i++) {
			pkd += s[i] * rdx;
			rdx *= pDimSz[i];
		}
		
		return pkd;
	}
	
	template<long unsigned N>
	StateN<N> doUnpackState(PackedStateN pkd, std::array<unsigned,N> const& pDimSz) {
		StateN<N> s;
		
		for(unsigned i=0; i<N; i++) {
			s[i] = pkd % pDimSz[i];
			pkd /= pDimSz[i];
		}
		
		return s;
	}
	
	template<long unsigned N>
	inline unsigned manhat(StateN<N> const& s, StateN<N> const& g) {
		unsigned d = 0;
		
		for(unsigned i=0; i<N; i++)
			d += std::abs((int)s[i] - (int)g[i]);
		
		return d;
	}
	
	
	template<long unsigned N>
	inline double euclid_dist(StateN<N> const& s, StateN<N> const& g) {
		double acc = 0;
		
		for(unsigned i=0; i<N; i++)
			acc += std::pow((double)s[i] - g[i], 2);
		
		return std::sqrt(acc);
	}
	
	
	template<unsigned K>
	struct CostType {
		using type = double;
	};
	
	template<>
	struct CostType<1> {
		using type = unsigned;
	};	
	
	
	
	
	//Edge iterator base. Provides iterator framework (except cost()), and ignores out-of-bounds adjacent positions.
	template<unsigned N, unsigned MaxK>
	struct AdjEdgeIterator_base {
		
		using Mv_t = std::array<std::pair<unsigned, bool>, MaxK>;
		
		
		AdjEdgeIterator_base(	std::array<unsigned, N>& pState,
								mathutil::HypergridMoveSet<N, MaxK> const& pMvSet,
								std::array<unsigned,N> const& pDimsSz) :
			mMvSet(pMvSet),
			mDimsSz(pDimsSz),
			mAdjState(pState),
			mLastOrd(0),
			mLastK(0),				//Invalid here, set properly in tryApplyMv().
			mTest_origState(pState)
		{
			for(; mLastOrd<mMvSet.size(); mLastOrd++) {
				if(tryApplyMv(mLastOrd))
					break;
			}
		}
		
		~AdjEdgeIterator_base() {
			if(!finished())
				undoMv();
			slow_assert(mTest_origState == mAdjState);
		}
		
		
		void next() {
			slow_assert(!finished());
			undoMv();
			for(; mLastOrd<mMvSet.size(); mLastOrd++) {
				if(tryApplyMv(mLastOrd))
					break;
			}
		}
		
		bool finished() const {
			bool b = mLastOrd == mMvSet.size();
			if(b)
				slow_assert(mTest_origState == mAdjState);
			return b;
		}
		
		std::array<unsigned, N>& state() const {
			slow_assert(!finished());
			return mAdjState;
		}
		
	
		private:
		
		void undoMv() {
			unsigned k;
			Mv_t const& mv = mMvSet.getMove(mLastOrd, k);
			
			for(unsigned j=0; j<k; j++) {
				if(mv[j].second)
					mAdjState[mv[j].first]--;
				else
					mAdjState[mv[j].first]++;
			}
		}
			
		
		bool tryApplyMv(unsigned i) {
			unsigned k;
			Mv_t const& mv = mMvSet.getMove(i, k);
			
			for(unsigned j=0; j<k; j++) {
				if(mAdjState[mv[j].first] == 0 && !mv[j].second)
					return false;
				if(mAdjState[mv[j].first] == mDimsSz[mv[j].first]-1 && mv[j].second)
					return false;
			}
			
			for(unsigned j=0; j<k; j++) {
				if(mv[j].second)
					mAdjState[mv[j].first]++;
				else
					mAdjState[mv[j].first]--;
			}
			
			if(k != mLastK)
				mLastK = k;
			
			mLastOrd = i;
			return true;
		}

		protected:
		
		mathutil::HypergridMoveSet<N, MaxK> const& mMvSet;
		std::array<unsigned,N> const& mDimsSz;
		
		std::array<unsigned, N>& mAdjState;
		unsigned mLastOrd;
		unsigned mLastK;
		
		const std::array<unsigned, N> mTest_origState;
	};
}}}
