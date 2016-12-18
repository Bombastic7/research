#pragma once


namespace mjon661 {
	
	/*
	 * A simple memory pool implementation.
	 * 
	 * malloc() returns a void pointer to a region of aligned, uninitialised memory of Chunk_Size bytes.
	 * free(void*) returns those regions to the pool. Note: does not check that the chunk was allocated.
	 * 
	 * clear() returns all allocated chunks to the pool without deallocating the backing memory.
	 * purge() returns all allocated chunks, and deallocates backing memory.
	 * shrink() deallocates all unused backing memory, and does not invalidate existing chunks.
	 * 
	 */
	
	template<unsigned Chunk_Size, unsigned Alignment, unsigned Chunks_Per_Block = 1024>
	class MemPool {

		union alignas(Alignment) Chunk_t {
			unsigned char e[Chunk_Size];
			Chunk_t* next;
		};

		struct Block_t {
			Block_t() : next(nullptr) {}
			
			Chunk_t chunks[Chunks_Per_Block];
			Block_t* next;
		};
		
		public:

		MemPool() : mHeadBlock(), mTopBlock(&mHeadBlock), mFreeHead(nullptr), mTopChunkIndex(0) {
		}
		
		~MemPool() {
			purge();
		}
		
		void* malloc() {
			Chunk_t* ret = nullptr;
			
			if(mFreeHead) {
				ret = mFreeHead;
				mFreeHead = mFreeHead->next;
			} else if(mTopChunkIndex == Chunks_Per_Block) {
				
				if(!mTopBlock->next)
					mTopBlock->next = new Block_t;
				
				mTopBlock = mTopBlock->next;
			
				ret = mTopBlock->chunks;
				mTopChunkIndex = 1;
			} else {
				ret = mTopBlock->chunks + mTopChunkIndex++;
			}
			
			return ret;
		}
		
		void free(void* ptr) {
			Chunk_t* chunk = reinterpret_cast<Chunk_t*>(ptr);
			chunk->next = mFreeHead;
			mFreeHead = chunk;
		}
		
		void clear() {
			mTopBlock = &mHeadBlock;
			mTopChunkIndex = 0;
			mFreeHead = nullptr;
		}
		
		void purge() {
			Block_t* next = nullptr;
			for(Block_t* block = mHeadBlock.next; block; block = next) {
				next = block->next;
				delete block;
			}
			mHeadBlock.next = nullptr;
			clear();
		}
		
		void shrink() {
			Block_t* next = nullptr;
			for(Block_t* block = mTopBlock->next; block; block = next) {
				next = block->next;
				delete block;
			}
			mTopBlock->next = nullptr;
		}
		
		private:

		Block_t mHeadBlock;
		Block_t* mTopBlock;
		Chunk_t* mFreeHead;
		unsigned mTopChunkIndex;
	};
}
