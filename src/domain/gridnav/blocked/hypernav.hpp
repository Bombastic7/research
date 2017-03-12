#pragma once




namespace mjon661 { namespace gridnav { namespace cube_blocked {



	enum struct Cell_t {
		Open, Blocked, Null
	};



	template<typename = void>
	class CellMap {
		
		public:
		
		class OpenCellIterator {
			public:
			
			OpenCellIterator& operator++() {
				if(mIdx == mInst.getSize())
					return *this;
				++mIdx;
				while(true) {
					bool v = mIdx < mInst.getSize();
					if(!v)
						break;
					
					bool o = mInst.isOpen(mIdx);
					if(o)
						break;
					++mIdx;
				}
				/*
				do {
					++mIdx;
				} while(mIdx < mInst.getSize() && !mInst.isOpen(mIdx));
				*/
				return *this;
			}
			
			bool operator==(OpenCellIterator const& o) {
				return mIdx == o.mIdx;
			}
			
			bool operator!=(OpenCellIterator const& o) {
				return mIdx != o.mIdx;
			}
			
			unsigned operator*() {
				return mIdx;
			}
			
			private:
			friend CellMap<void>;
			
			OpenCellIterator(CellMap<void> const& pInst, bool pAtEnd) :
				mInst(pInst),
				mIdx(0)
			{
				slow_assert(pInst.mCells.size() == pInst.getSize());
				if(pAtEnd)
					mIdx = mInst.getSize();
				else
					while(mIdx < mInst.getSize() && !mInst.isOpen(mIdx))
						++mIdx;
			}
			
			CellMap<void> const& mInst;
			unsigned mIdx;
		};
		
		
		CellMap(unsigned pSize, std::string const& pMapFile) :
			mSize(pSize),
			mCells(mSize)
		{
			if(pMapFile[0] == '.') {
				unsigned seed = std::strtol(pMapFile.c_str()+1, nullptr, 10);
				initRandomMap(seed);
			}
			else {
				std::ifstream ifs(pMapFile);
				
				if(!ifs)
					throw std::runtime_error("Could not open map file");

				for(unsigned i=0; i<mSize; i++) {
					int v;
					Cell_t c;
					ifs >> v;
					c = (Cell_t)v;
					
					gen_assert(c == Cell_t::Open || c == Cell_t::Blocked);
					mCells[i] = c;
				}
			}
		}
		
		
		std::vector<Cell_t> const& cells() const {
			return mCells;
		}
		
		unsigned getSize() const {
			return mSize;
		}
		
		bool isOpen(unsigned i) const {
			slow_assert(i < mSize, "%u %u", i, mSize);
			return cells()[i] == Cell_t::Open;
		}
		
		OpenCellIterator begin() const {
			return OpenCellIterator(*this, false);
		}
		
		OpenCellIterator end() const {
			return OpenCellIterator(*this, true);
		}
		
		private:
		
		void initRandomMap(unsigned seed) {
			std::mt19937 gen(5489u + seed);
			std::uniform_real_distribution<double> d(0.0,1.0);
			
			for(unsigned i=0; i<mSize; i++) {
				mCells[i] = d(gen) <= 0.35 ? Cell_t::Blocked : Cell_t::Open;
			}
			
			logDebugStream() << "Random CellMap init. seed=" << seed << ", blockedprob=0.35" << "\n";
		}
		
		const unsigned mSize;
		std::vector<Cell_t> mCells;
	};
	
	
	
	//CellMap is filled by column (x=[0...width-1]), then row (y=[0...height-1]), then depth (z=[0...depth-1]).
	//(0,0,0) is left-most, top-most, shallowest cell.
	
	//up(x,y,z) = (x, y-1, z)
	//down(x,y,z) = (x, y+1, z)
	//left(x,y,z) = (x-1, y, z)
	//right(x,y,z) = (x+1, y, z)
	//shallower(x,y,z) = (x, y, z-1)
	//deeper(x,y,z) = (x, y, z+1)
	
	//That's 3 categories of 6 base directions.
	//Diag-2 moves are formed by combining two base directions from different categories. That 12 more directions. cost=sqrt(2).
	//Diag-3 moves are formed by combining three base directions from diffent categories. That's another 8. cost=sqrt(3).
	
	//cellmap(x,y,z) = cellmap(x + width*y * width*height*z)
	
	

	const Null_Idx = (unsigned)-1;
	
	template<unsigned Sz>
	struct AdjacentCells {
		std::array<unsigned, Sz> adjCells;
	};
	
	enum struct CDir {
		U, D, L, R, Sh, Dp, 
		
		Sh_U, 	Sh_D, 	Sh_L, 	Sh_R,
		U_L, 	U_R,	D_L,	D_R,
		Dp_U, 	Dp_D, 	Dp_L, 	Dp_R,
	
		Sh_U_L, Sh_U_R, Sh_D_L, Sh_D_R,
		Dp_U_L, Dp_U_R, Dp_D_L, Dp_D_R,
	};
	
	
	template<unsigned Sz>
	inline void fillAdjacentCells_6(
		std::vector<Cell_t> const& pCells,
		unsigned pWidth,
		unsigned pHeight,
		unsigned pDepth,
		unsigned s,
		std::array<unsigned, Sz>& adj)
	{
		if(s >= pWidth && pCells[s-pWidth] == Cell_t::Open)
			adj[CDir::U] = s-mWidth;
			
		if(s < (pHeight-1)*pWidth && pCells[s+pWidth] == Cell_t::Open)
			adj[CDir::D] = s+mWidth;
			
		if(s%pWidth != 0 && pCells[s-1] == Cell_t::Open)
			adj[CDir::L] = s-1;

		if((s+1)%pWidth != 0 && pCells[s+1] == Cell_t::Open)
			adj[CDir::R] = s+1;
		
		if(s >= pWidth*pHeight && pCells[s-pWidth*pHeight] == Cell_t::Open)
			adj[CDir::Sh] = s-pWidth*pHeight;
	
		if(s < (pDepth-1) * (pWidth*pHeight) && pCells[s + pWidth*pHeight] == Cell_t::Open)
			adj[CDir::Dp] = s + pWidth*pHeight;
	}
	
	
	template<unsigned Sz>
	inline void fillAdjacentCells_18(
		std::vector<Cell_t> const& pCells,
		unsigned pWidth,
		unsigned pHeight,
		unsigned pDepth,
		unsigned s,
		std::array<unsigned, Sz>& adj)
	{
		fillAdjacentCells_6(pCells, pWidth, pHeight, pDepth, s, adj);
		
		const unsigned zl = pWidth*pHeight;
		
		if(adj[CDir::Sh] != Null_Idx) {
			if(adj[CDir::U] != Null_Idx && pCells[s-zl-pWidth] == Cell_t::Open)
				adj[CDir::Sh_U] = s-zl-pWidth;
			
			if(adj[CDir::D] != Null_Idx && pCells[s-zl+pHeight] == Cell_t::Open)
				adj[CDir::Sh_D] = s-zl+pWidth;
			
			if(adj[CDir::L] != Null_Idx && pCells[s-zl-1] == Cell_t::Open)
				adj[CDir::Sh_L] = s-zl-1;

			if(adj[CDir::R] != Null_Idx && pCells[s-zl+1] == Cell_t::Open)
				adj[CDir::Sh_R] = s-zl+1;
		}
		
		if(adj[CDir::Dp] != Null_Idx) {
			if(adj[CDir::U] != Null_Idx && pCells[s+zl-pWidth] == Cell_t::Open)
				adj[CDir::Dp_U] = s+zl-pWidth;
			
			if(adj[CDir::D] != Null_Idx && pCells[s+zl+pHeight] == Cell_t::Open)
				adj[CDir::Dp_D] = s+zl+pWidth;
			
			if(adj[CDir::L] != Null_Idx && pCells[s+zl-1] == Cell_t::Open)
				adj[CDir::Dp_L] = s+zl-1;

			if(adj[CDir::R] != Null_Idx && pCells[s+zl+1] == Cell_t::Open)
				adj[CDir::Dp_R] = s+zl+1;
		}
		
		if(adj[CDir::U] != Null_Idx && adj[CDir::L] != Null_Idx && pCells[s - mWidth - 1])
			adj[CDir::U_L] = s - mWidth - 1;
		
		if(adj[CDir::U] != Null_Idx && adj[CDir::R] != Null_Idx && pCells[s - mWidth + 1])
			adj[CDir::U_R] = s - mWidth + 1;
		
		if(adj[CDir::D] != Null_Idx && adj[CDir::L] != Null_Idx && pCells[s + mWidth - 1])
			adj[CDir::U_L] = s + mWidth - 1;
		
		if(adj[CDir::D] != Null_Idx && adj[CDir::R] != Null_Idx && pCells[s + mWidth + 1])
			adj[CDir::U_R] = s + mWidth + 1;
			
		if(adj[CDir::Sh] != Null_Idx && adj[CDir::U] != Null_Idx && pCells[s-zl-1] == Cell_t::Open) //up-left
			adj[CDir::U_L] = s-zl-1;		
	}
	
	
	template<unsigned Sz>
	inline void fillAdjacentCells_26(
		std::vector<Cell_t> const& pCells,
		unsigned pWidth,
		unsigned pHeight,
		unsigned pDepth,
		unsigned s,
		std::array<unsigned, Sz>& adj)
	{
		fillAdjacentCells_18(pCells, pWidth, pHeight, pDepth, s, adj);
		
		const unsigned zl = pWidth*pHeight;
		
		if(adj[CDir::Sh] != Null_Idx) {
			if(adj[CDir::U_L] != Null_Idx && pCells[s-zl-pWidth-1] == Cell_t::Open)
				adj[CDir::Sh_U_L] = s-zl-pWidth-1;
			
			if(adj[CDir::U_R] != Null_Idx && pCells[s-zl-pWidth+1] == Cell_t::Open)
				adj[CDir::Sh_U_R] = s-zl-pWidth+1;
			
			if(adj[CDir::D_L] != Null_Idx && pCells[s-zl+pWidth-1] == Cell_t::Open)
				adj[CDir::Sh_D_L] = s-zl+pWidth-1;

			if(adj[CDir::D_R] != Null_Idx && pCells[s-zl+pWidth+1] == Cell_t::Open)
				adj[CDir::Sh_D_R] = s-zl+pWidth+1;
		}
		
		if(adj[CDir::Dp] != Null_Idx) {
			if(adj[CDir::U_L] != Null_Idx && pCells[s+zl-pWidth-1] == Cell_t::Open)
				adj[CDir::Dp_U_L] = s+zl-pWidth-1;
			
			if(adj[CDir::U_R] != Null_Idx && pCells[s+zl-pWidth+1] == Cell_t::Open)
				adj[CDir::Dp_U_R] = s+zl-pWidth+1;
			
			if(adj[CDir::D_L] != Null_Idx && pCells[s+zl+pWidth-1] == Cell_t::Open)
				adj[CDir::Dp_D_L] = s+zl+pWidth-1;

			if(adj[CDir::D_R] != Null_Idx && pCells[s+zl+pWidth+1] == Cell_t::Open)
				adj[CDir::Dp_D_R] = s+zl+pHeight+1;
		}
	}
		
	template<bool Use_LifeCost>
	struct CellGraph_6 {
		public:
		using Cost_t = int;


		
		CellGraph_4(unsigned pHeight, unsigned pWidth, unsigned pDepth, std::string const& pMapFile) :
			CellMap(pHeight*pWidth*pDepth, pMapFile),
			mHeight(pHeight),
			mWidth(pWidth),
			mDepth(pDepth)
		{}

		template<unsigned Sz>
		const AdjacentCells fillAdjacentCells(std::array<unsigned, Sz>& adj) const {
			AdjacentCells adj{.n=0};
			
			Cost_t costMul = Use_LifeCost ? s/mWidth : 1;
			
			if(s >= mWidth && this->isOpen(s-mWidth))
				adj.adjCells[adj.n++] = s-mWidth;
			if(s < (mHeight-1)*mWidth && this->isOpen(s+mWidth))
				adj.adjCells[adj.n++] = s+mWidth;
			if(s%mWidth != 0 && this->isOpen(s-1))
				adj.adjCells[adj.n++] = s-1;
			if((s+1)%mWidth != 0 && this->isOpen(s+1))
				adj.adjCells[adj.n++] = s+1;
			return adj;
		}
		
		Cost_t getMoveCost(unsigned src, unsigned dst) const {
			return Use_LifeCost ? src/mWidth : 1;
		}

		unsigned getHeight() const {
			return mHeight;
		}
		
		unsigned getWidth() const {
			return mWidth;
		}
		
		private:
		const unsigned mHeight, mWidth;
		
	};






}}}
