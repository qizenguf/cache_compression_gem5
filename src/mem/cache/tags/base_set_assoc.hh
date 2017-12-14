/*
 * Copyright (c) 2012-2014 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Erik Hallnor
 */

/**
 * @file
 * Declaration of a base set associative tag store.
 */

#ifndef __MEM_CACHE_TAGS_BASESETASSOC_HH__
#define __MEM_CACHE_TAGS_BASESETASSOC_HH__

#include <cassert>
#include <cstring>
#include <list>

#include "mem/cache/base.hh"
#include "mem/cache/blk.hh"
#include "mem/cache/tags/base.hh"
#include "mem/cache/tags/cacheset.hh"
#include "mem/packet.hh"
#include "params/BaseSetAssoc.hh"

/**
 * A BaseSetAssoc cache tag store.
 * @sa  \ref gem5MemorySystem "gem5 Memory System"
 *
 * The BaseSetAssoc tags provide a base, as well as the functionality
 * common to any set associative tags. Any derived class must implement
 * the methods related to the specifics of the actual replacment policy.
 * These are:
 *
 * BlkType* accessBlock();
 * BlkType* findVictim();
 * void insertBlock();
 * void invalidate();
 */
class BaseSetAssoc : public BaseTags
{
  public:
    /** Typedef the block type used in this tag store. */
    typedef CacheBlk BlkType;
    /** Typedef the set type used in this tag store. */
    typedef CacheSet<CacheBlk> SetType;


  protected:
    /** The associativity of the cache. */
    const unsigned assoc;
    /** The allocatable associativity of the cache (alloc mask). */
    unsigned allocAssoc;
    //by Qi
    const unsigned secSize;    
    bool secLimit;
    
    const unsigned entrySize;
    int compactLimit;
    int dictLimit;
    /** The number of sets in the cache. */
    const unsigned numSets;
    /** Whether tags and data are accessed sequentially. */
    const bool sequentialAccess;

    /** The cache sets. */
    SetType *sets;

    /** The cache blocks. */
    BlkType *blks;
    /** The data blocks, 1 per cache block. */
    uint8_t *dataBlks;

    /** The amount to shift the address to get the set. */
    int setShift;
    /** The amount to shift the address to get the tag. */
    int tagShift;
    /** The amount to shift the address to get the sec#. */
    int secShift; //by Qi
    /** Mask out all bits that aren't part of the set index. */
    unsigned setMask;
     /** Mask out all bits that aren't part of the sector index. */
    unsigned secMask; // by Qi
    std::map<uint64_t, int> sideCnt;
   

public:

    /** Convenience typedef. */
     typedef BaseSetAssocParams Params;

    /**
     * Construct and initialize this tag store.
     */
    BaseSetAssoc(const Params *p);

    /**
     * Destructor
     */
    virtual ~BaseSetAssoc();

    /**
     * Find the cache block given set and way
     * @param set The set of the block.
     * @param way The way of the block.
     * @return The cache block.
     */
    CacheBlk *findBlockBySetAndWay(int set, int way) const override;

    /**
     * Invalidate the given block.
     * @param blk The block to invalidate.
     */
    void invalidate(CacheBlk *blk) override
    {
        assert(blk);
        assert(blk->isValid());
        tagsInUse--;
        //assert(blk->srcMasterId < cache->system->maxMasters());
        //occupancies[blk->srcMasterId]--;
        blk->dictionary.clear();
        blk->dictionary2.clear();
        blk->compactSize = 1;
        blk->cells = 1;
        blk->srcMasterId = Request::invldMasterId;
        blk->task_id = ContextSwitchTaskId::Unknown;
        blk->tickInserted = curTick();
    }

    /**
     * Access block and update replacement data. May not succeed, in which case
     * nullptr is returned. This has all the implications of a cache
     * access and should only be used as such. Returns the access latency as a
     * side effect.
     * @param addr The address to find.
     * @param is_secure True if the target memory space is secure.
     * @param lat The access latency.
     * @return Pointer to the cache block if found.
     */
    CacheBlk* accessBlock(Addr addr, bool is_secure, Cycles &lat) override
    {
        Addr tag = extractTag(addr);
        int set = extractSet(addr);
        BlkType *blk = sets[set].findBlk(tag, is_secure);

        // Access all tags in parallel, hence one in each way.  The data side
        // either accesses all blocks in parallel, or one block sequentially on
        // a hit.  Sequential access with a miss doesn't access data.
        tagAccesses += allocAssoc;
        if (sequentialAccess) {
            if (blk != nullptr) {
                dataAccesses += 1;
            }
        } else {
            dataAccesses += allocAssoc;
        }

        if (blk != nullptr) {
            // If a cache hit
            lat = accessLatency;
            // Check if the block to be accessed is available. If not,
            // apply the accessLatency on top of block->whenReady.
            if (blk->whenReady > curTick() &&
                cache->ticksToCycles(blk->whenReady - curTick()) >
                accessLatency) {
                lat = cache->ticksToCycles(blk->whenReady - curTick()) +
                accessLatency;
            }
            blk->refCount += 1;
        } else {
            // If a cache miss
            lat = lookupLatency;
        } 

        return blk;
    }

    /**
     * Finds the given address in the cache, do not update replacement data.
     * i.e. This is a no-side-effect find of a block.
     * @param addr The address to find.
     * @param is_secure True if the target memory space is secure.
     * @param asid The address space ID.
     * @return Pointer to the cache block if found.
     */
    CacheBlk* findBlock(Addr addr, bool is_secure) const override;

    /**
     * Find an invalid block to evict for the address provided.
     * If there are no invalid blocks, this will return the block
     * in the least-recently-used position.
     * @param addr The addr to a find a replacement candidate for.
     * @return The candidate block.
     */
    CacheBlk* findVictim(Addr addr) override
    {
        BlkType *blk = nullptr;
        int set = extractSet(addr);

        // prefer to evict an invalid block
        for (int i = 0; i < allocAssoc; ++i) {
            blk = sets[set].blks[i];
            if (!blk->isValid())
                break;
        }

        return blk;
    }
    CacheBlk* findVictim(Addr addr, PacketPtr pkt) 
    {
        BlkType *blk = nullptr;
        int set = extractSet(addr);

        // prefer to evict an invalid block
        for (int i = 0; i < allocAssoc; ++i) {
            blk = sets[set].blks[i];
            if (!blk->isValid())
                break;
        }

        return blk;
    }
	/**
     * copy the new data into the cache.
     * @param pkt Packet holding the address to update
     * @param blk The block to update.
     */
     void copyData(PacketPtr pkt, CacheBlk *blk) override
     {
	  	 std::memcpy(blk->data, pkt->getConstPtr<uint8_t>(), blkSize);	
	 }
    /**
     * Insert the new block into the cache.
     * @param pkt Packet holding the address to update
     * @param blk The block to update.
     */
     CacheBlk * insertBlock(PacketPtr pkt, CacheBlk * &blk) override
     {
         Addr addr = pkt->getAddr();
         MasterID master_id = pkt->req->masterId();
         uint32_t task_id = pkt->req->taskId();
		 if( blk == nullptr) return nullptr;
         if (!blk->isTouched) {
             tagsInUse++;
             blk->isTouched = true;
             if (!warmedUp && tagsInUse.value() >= warmupBound) {
                 warmedUp = true;
                 warmupCycle = curTick();
             }
         }

         // If we're replacing a block that was previously valid update
         // stats for it. This can't be done in findBlock() because a
         // found block might not actually be replaced there if the
         // coherence protocol says it can't be.
         
      
         if (blk->isValid()) {				 
			 replacements[0]++;
			 totalRefs += blk->refCount;
			 ++sampledRefs;
			 blk->refCount = 0;

			 // deal with evicted block
			 assert(blk->srcMasterId < cache->system->maxMasters());
			 occupancies[blk->srcMasterId]--;
			 blk->invalidate();			 
         }

         blk->isTouched = true;

         // Set tag for new block.  Caller is responsible for setting status.
         blk->tag = extractTag(addr);

         // deal with what we are bringing in
         assert(master_id < cache->system->maxMasters());
         occupancies[master_id]++;
         blk->srcMasterId = master_id;
         blk->task_id = task_id;
         blk->tickInserted = curTick();

         // We only need to write into one tag and one data block.
         tagAccesses += 1;
         dataAccesses += 1;
         return nullptr;
     }

    /**
     * Limit the allocation for the cache ways.
     * @param ways The maximum number of ways available for replacement.
     */
    virtual void setWayAllocationMax(int ways) override
    {
        fatal_if(ways < 1, "Allocation limit must be greater than zero");
        allocAssoc = ways;
    }

    /**
     * Get the way allocation mask limit.
     * @return The maximum number of ways available for replacement.
     */
    virtual int getWayAllocationMax() const override
    {
        return allocAssoc;
    }

    /**
     * Generate the tag from the given address.
     * @param addr The address to get the tag from.
     * @return The tag of the address.
     */
    Addr extractTag(Addr addr) const override
    {
        //return (addr >> tagShift);
        return (addr >> setShift); // modified by Qi, sector indexing
    }

    /**
     * Calculate the set index from the address.
     * @param addr The address to get the set from.
     * @return The set index of the address.
     */
    int extractSet(Addr addr) const override
    {
        //return ((addr >> setShift) & setMask);
        return ((addr >> (setShift + secShift)) & setMask); // modified by Qi, sector indexing
    }

    /**
     * Regenerate the block address from the tag.
     * @param tag The tag of the block.
     * @param set The set of the block.
     * @return The block address.
     */
    Addr regenerateBlkAddr(Addr tag, unsigned set) const override
    {
        //return ((tag << tagShift) | ((Addr)set << setShift));
        return (tag << setShift); // by Qi
    }

    /**
     * Called at end of simulation to complete average block reference stats.
     */
    void cleanupRefs() override;

    /**
     * Print all tags used
     */
    std::string print() const override;

    /**
     * Called prior to dumping stats to compute task occupancy
     */
    void computeStats() override;

    /**
     * Visit each block in the tag store and apply a visitor to the
     * block.
     *
     * The visitor should be a function (or object that behaves like a
     * function) that takes a cache block reference as its parameter
     * and returns a bool. A visitor can request the traversal to be
     * stopped by returning false, returning true causes it to be
     * called for the next block in the tag store.
     *
     * \param visitor Visitor to call on each block.
     */
    void forEachBlk(CacheBlkVisitor &visitor) override {
        for (unsigned i = 0; i < numSets * assoc; ++i) {
            if (!visitor(blks[i]))
                return;
        }
    }
    /**
     * Calculate the sector index from the address. by qi
     * @param addr The address to get the set from.
     * @return The sector index of the address.
     */
    int extractSectorID(Addr addr)
    {
        return ((addr >> (setShift)) & secMask);
    }
    /**
     * extract the dictionary from a block.
     * @param block content and size of dictionary entry.( 8byte at most)
     * @return dictionary entries vector.
     */
    static std::vector<int> extractDict(const uint8_t* blkData, std::unordered_map<uint64_t , int> &oldMap, int blkSize, int entSize, int offsetSize = 0){
		//unordered_map<uint64_t, int> ret;
		std::vector<int> ret;
		int numEnt = blkSize/entSize;
		//if(size == 65) cout<<"nearby"<<endl;
		//else cout <<"random"<<endl;
		for(int i = 0; i<numEnt; i++){
			uint64_t cur = 0;
			for(int j = 0; j<entSize; j++) 
				cur = ((cur<<8) | blkData[ i*entSize + j]);
			cur = cur >> offsetSize; // offset of each entry
			if( oldMap.find(cur) == oldMap.end()){
				//cout<<(uint64_t)ablock[0]<< std::hex<<"  data "<<cur<< "  dic_size " << oldMap.size() << "  offset " << offsetSize <<endl;
				ret.push_back(oldMap.size());
				oldMap[cur] = oldMap.size();
				//ret.push_back(oldMap.size());
			}else{
				//cout<<(uint64_t)ablock[0]<< std::hex<<"  data "<<cur<< "  dic_size " << oldMap.size() << "  offset " << offsetSize <<" found"<<endl;
				ret.push_back(oldMap[cur]);
			}
		}
		return ret;		
	}
	int countUniqueKeys(const uint8_t* blkData, int blkSize, int entSize, int offsetSize = 0){
		std::unordered_map<uint64_t , int> tMap;
		int numEnt = blkSize/entSize;
		//if(size == 65) cout<<"nearby"<<endl;
		//else cout <<"random"<<endl;
		for(int i = 0; i<numEnt; i++){
			uint64_t cur = 0;
			for(int j = 0; j<entSize; j++) 
				cur = ((cur<<8) | blkData[ i*entSize + j]);
			cur = cur >> offsetSize; 
			if( tMap.find(cur) == tMap.end()){
				//cout<<(uint64_t)ablock[0]<< std::hex<<"  data "<<cur<< "  dic_size " << oldMap.size() << "  offset " << offsetSize <<endl;
				tMap[cur] = 1;
			}else
				tMap[cur] += 1;
		}
		return tMap.size();
	}
	void countSideKeys(CacheBlk * a){
		/*if(a->masterDictionary.size() == 0 or a->dictionary.size() == 0) return;

		for(auto it = a->dictionary.begin(); it != a->dictionary.end(); it++ ){
			if( a->masterDictionary.find(it->first) == a->masterDictionary.end()){
				
				sideCnt[it->first] += 1;
			}
		}*/
		return;
	}
	int similarityMeasure(const uint8_t* blkData1, const uint8_t* blkData2, int size = 64){
		int res = 0;
		for(int i = 0; i<size; i++){
			if (blkData1[i] != blkData2[i])
				res ++; 
		}
		return res;		
	}
    int zeroMeasure(const uint8_t* blkData1, int size = 64){
        int res = 0;
        for(int i = 0; i<size; i++){
            if (blkData1[i] != 0)
                res ++; 
        }
        return res;     
    }
	int keysDiff(std::unordered_map<uint64_t , int> masterMap, std::unordered_map<uint64_t, int> sideMap){
		int ret = 0;
        if(masterMap.size() == 0 ) return sideMap.size();
		for(auto it = sideMap.begin(); it != sideMap.end(); it++ ){
            if(it->first == 0 || it->first == 0xffffffff || it->first == 0x2000000 \
                || it->first == 0x20000000 || it->first == 0x1000000 || it->first == 0x4000000 || it->first == 0x8000000 || it->first == 0x80000000 ) continue;
			if( masterMap.find(it->first) == masterMap.end())
				ret ++;
		}
		return ret;
	}
	int keysDiff(const uint8_t* blkData, std::unordered_map<uint64_t, int> sideMap){
		int ret = 0;
		if(blkData == nullptr) return 100;
		std::unordered_map<uint64_t, int> masterMap;
		extractDict(blkData, masterMap, 64, 4, 0);
        if(masterMap.size() == 0 ) return sideMap.size();
		for(auto it = sideMap.begin(); it != sideMap.end(); it++ ){
            if(it->first == 0 || it->first == 0xffffffff || it->first == 0x2000000 \
                || it->first == 0x20000000 || it->first == 0x1000000 || it->first == 0x4000000 || it->first == 0x8000000 || it->first == 0x80000000 ) continue;
			if( masterMap.find(it->first) == masterMap.end())
				ret ++;
		}
		return ret;
	}
	int checkThree(CacheBlk * a, CacheBlk * b, PacketPtr pkt, int entSize) {
		std::unordered_map<uint64_t, int> temp;
		extractDict(a->data, temp, 65, entSize, 0);
		extractDict(b->data, temp, 65, entSize, 0);
		extractDict(pkt->getConstPtr<uint8_t>(), temp, 65, entSize, 0);
		if(keysDiff(a->data, temp) <= 8) {
			//blk->dictionary = temp;
			return 4;
		}
		else return -1;
	}
	int checkCompanion(CacheBlk * b, PacketPtr pkt, int entSize, int companionLimit) {
		std::unordered_map<uint64_t, int> temp(b->dictionary);
		extractDict(pkt->getConstPtr<uint8_t>(), temp, 65, entSize, 0);
		if(keysDiff(b->masterDictionary, temp) <= companionLimit) {
			//blk->dictionary = temp;
			return 4;
		}
		if(keysDiff(pkt->getConstPtr<uint8_t>(), temp) <= companionLimit) {
			b->masterDictionary.clear();
			extractDict(pkt->getConstPtr<uint8_t>(), b->masterDictionary, 65, entSize, 0);
			//blk->dictionary = temp;
			return 5;
		}
		int mindex = -1;
		int minSize = 99;
		for(int i = 0 ; i< 4; i++){
			if(b->subs[i] != nullptr && (b->subs[i]->tag & b->tag) < secSize){
				int t = keysDiff(b->subs[i]->data, temp);
				if( t < minSize){
					minSize = t;
					mindex = i;
					
				}
			}
		}
		if(minSize <= companionLimit){
			b->masterkeys = minSize;
			b->masterDictionary.clear();
			extractDict(b->subs[mindex]->data, b->masterDictionary, 65, entSize, 0);
			//blk->dictionary = temp;
			return mindex;
		}
		return -1;
	}
    int checkcompressbility(CacheBlk * b, PacketPtr pkt, int entSize) {
        int ifCompressS1 = 0;
        int ifCompressS2 = 0;
        int ifCompressS3 = 0;
        int ifCompressS4 = 0;
        //int ifCompressS5 = 0;

        std::unordered_map<uint64_t, int> temp(b->dictionary); // protect the dictionary from block and make a copy
        extractDict(pkt->getConstPtr<uint8_t>(), temp, 65, entSize, 0);
        if(temp.size() <= 8 ) {                
            //b->dictionary = temp;            
            ifCompressS1 = 1;
        }
        std::unordered_map<uint64_t, int> temp2(b->dictionary2);
        extractDict(pkt->getConstPtr<uint8_t>(), temp2, 65, entSize, 4); // higher priority for scheme2
        
		/*int diff = keysDiff(b->masterDictionary, temp);
		if(diff <= 8 )
			ifCompressS2 = 2;
			// higher priority for scheme2
		*/
        
        if(temp2.size() <= 4 ) { // 1 for DISH
            //b->dictionary2 = temp2;
            ifCompressS1 = 1;
        }
        //if(temp.size() <= b->dictionary.size() + 4) // record some entries in extended keys
		//	ifCompressS2 = 2; 
        if(temp.size() <= 11) // record some entries in extended keys
			ifCompressS3 = 4;
        if(temp2.size() <= 8) // record some entries in extended keys
			ifCompressS3 = 4;
		//if( b->cells<3 && (temp.size() <= b->masterkeys + 8 || temp.size() <= 16 )) //// record some entries with additional block
		//	ifCompressS4 = 2;
		//if(similarityMeasure(b->data, pkt->getConstPtr<uint8_t>(), 64) < dictLimit)
		//	ifCompressS5 = 16;
			
        return ifCompressS1 + ifCompressS2 + ifCompressS3 + ifCompressS4;
        
     }
     void delink(CacheBlk *blk) override{
		 return;
	 }
	 void printContent(CacheBlk *blk){
		 uint32_t *cur = (uint32_t *)blk->data;
		 std::cout<<std::hex<<blk->tag <<" s"<<blk->curScheme <<" d"<<blk->dictionary.size() <<" c"<<blk->cells <<" side" <<blk->side ;
		 for(int i = 0; i<16;i++){
			 std::cout<<" :"<<std::hex<< *cur;
			 cur++;
		 }	
		 std::cout<<std::endl;	 
	 }
};

#endif // __MEM_CACHE_TAGS_BASESETASSOC_HH__
