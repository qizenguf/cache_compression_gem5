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
 * Definitions of a base set associative tag store.
 */

#include "mem/cache/tags/base_set_assoc.hh"

#include <string>

#include "base/intmath.hh"
#include "sim/core.hh"
typedef std::function<bool(std::pair<uint64_t, int>, std::pair<uint64_t, int>)> Comparator2;
typedef std::function<bool(std::pair<std::vector<uint64_t>, int>, std::pair<std::vector<uint64_t>, int>)> Comparator;
using namespace std;

BaseSetAssoc::BaseSetAssoc(const Params *p)
    :BaseTags(p), assoc(p->assoc *(p->sector_size+1)), allocAssoc(p->assoc), secSize(p->sector_size), entrySize(p->entry_size), compactLimit(p->compactLimit), dictLimit(p->dictLimit),
     numSets(p->size / (p->block_size * p->assoc)),
     sequentialAccess(p->sequential_access), sideCnt()
{
    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
    if (numSets <= 0 || !isPowerOf2(numSets)) {
        fatal("# of sets must be non-zero and a power of 2");
    }
    if (assoc <= 0) {
        fatal("associativity must be greater than zero");
    }

    setShift = floorLog2(blkSize);
    setMask = numSets - 1;
    secShift = floorLog2(secSize); // add by Qi
    secMask = secSize - 1;
    tagShift = setShift + floorLog2(numSets);
    /** @todo Make warmup percentage a parameter. */
    warmupBound = numSets * assoc;
	if(p->secLimit > 0 )
		secLimit = true;
    sets = new SetType[numSets];
    blks = new BlkType[numSets * assoc];
    // allocate data storage in one big chunk
    numBlocks = numSets * assoc;
    dataBlks = new uint8_t[numBlocks * blkSize] ;
	
    unsigned blkIndex = 0;       // index into blks array
    for (unsigned i = 0; i < numSets; ++i) {
        sets[i].assoc = assoc;
		sets[i].secSize = secSize; //add by Qi
        sets[i].blks = new BlkType*[assoc];

        // link in the data blocks
        for (unsigned j = 0; j < assoc; ++j) {
            // locate next cache block
            BlkType *blk = &blks[blkIndex];
            blk->data = &dataBlks[blkSize*blkIndex]; // withholes
            blk->parent = blk;
            if(j >= allocAssoc) {
				blk->isChild = true;
				blk->parent = nullptr;
			}
            //blk->freeDataBlks = &dataBlks[blkSize*(1+blkIndex*secSize)]; //Qi
            //blk->endOfDataBlks = &dataBlks[blkSize*(1+blkIndex)*secSize - 1]; 
            ++blkIndex;

            // invalidate new cache block
            blk->invalidate();

            //EGH Fix Me : do we need to initialize blk?

            // Setting the tag to j is just to prevent long chains in the hash
            // table; won't matter because the block is invalid
            blk->tag = j;
            blk->whenReady = 0;
            blk->isTouched = false;
            sets[i].blks[j]=blk;
            blk->set = i;
            blk->way = j;
        }
    }
}

BaseSetAssoc::~BaseSetAssoc()
{
    delete [] dataBlks;
    delete [] blks;
    delete [] sets;
}

CacheBlk*
BaseSetAssoc::findBlock(Addr addr, bool is_secure) const
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    BlkType *blk = sets[set].findBlk(tag, is_secure);
    return blk;
}

CacheBlk*
BaseSetAssoc::findBlockBySetAndWay(int set, int way) const
{
    return sets[set].blks[way];
}

std::string
BaseSetAssoc::print() const {
    std::string cache_state;
    for (unsigned i = 0; i < numSets; ++i) {
        // link in the data blocks
        for (unsigned j = 0; j < assoc; ++j) {
            BlkType *blk = sets[i].blks[j];
            if (blk->isValid())
                cache_state += csprintf("\tset: %d block: %d %s\n", i, j,
                        blk->print());
        }
    }
    if (cache_state.empty())
        cache_state = "no valid tags\n";
    return cache_state;
}

void
BaseSetAssoc::cleanupRefs()
{
    for (unsigned i = 0; i < numSets*assoc; ++i) {
        if (blks[i].isValid()) {
            totalRefs += blks[i].refCount;
            ++sampledRefs;
        }
    }
}

void
BaseSetAssoc::computeStats()
{
    for (unsigned i = 0; i < ContextSwitchTaskId::NumTaskId; ++i) {
        occupanciesTaskId[i] = 0;
        for (unsigned j = 0; j < 5; ++j) {
            ageTaskId[i][j] = 0;
        }
    }

    for (unsigned i = 0; i < numSets * assoc; ++i) {
        if (blks[i].isValid()) {
            assert(blks[i].task_id < ContextSwitchTaskId::NumTaskId);
            occupanciesTaskId[blks[i].task_id]++;
            assert(blks[i].tickInserted <= curTick());
            Tick age = curTick() - blks[i].tickInserted;

            int age_index;
            if (age / SimClock::Int::us < 10) { // <10us
                age_index = 0;
            } else if (age / SimClock::Int::us < 100) { // <100us
                age_index = 1;
            } else if (age / SimClock::Int::ms < 1) { // <1ms
                age_index = 2;
            } else if (age / SimClock::Int::ms < 10) { // <10ms
                age_index = 3;
            } else
                age_index = 4; // >10ms

            ageTaskId[blks[i].task_id][age_index]++;
        }
    }
		endBlocks = 0;
		endBlocks2 = 0;
		for(int i = 0; i<numSets; i++){
			for(int j = 0; j<assoc; j++){
				//CacheBlk * cblk = sets[i].blks[j];
				if(!sets[i].blks[j]->isChild && sets[i].blks[j]->isValid()){
					
					endBlocks += sets[i].blks[j]->compactSize;
					//compactedCount[sets[i].blks[j]->compactSize]++;
                    if(sets[i].blks[j]->compactSize > 1){
						/*if(compactLimit == 0 && dictLimit == 0 && cblk->compactSize >= 3 && cblk->dictionary.size() < 25){
							
							printContent(cblk);
							for(int k = 0 ;k<4; k++){
								if(cblk->subs[k] != nullptr && (cblk->subs[k]->tag & cblk->tag) < secSize ){
									printContent(cblk->subs[k]);
								}
							}
						}*/
						if(sets[i].blks[j]->dictionary.size() < 1 or sets[i].blks[j]->dictionary.size() > 64 ) {
                            std::cout<<"too many keys:" << sets[i].blks[j]->dictionary.size()<<std::endl;
                        }else{
                            sectorCount[((int)sets[i].blks[j]->dictionary.size()-1) /4]++;
                        }
                        
						if( sets[i].blks[j]->cells == 4 || sets[i].blks[j]->curScheme == 9){
                            if( sets[i].blks[j]->cells != 4 || sets[i].blks[j]->curScheme != 9) std::cout<<sets[i].blks[j]->cells <<" cell || scheme " << sets[i].blks[j]->curScheme<<std::endl;
                            SchemeCount[sets[i].blks[j]->compactSize + 11]++;
                        }else if(sets[i].blks[j]->side != -1){
                            SchemeCount[sets[i].blks[j]->compactSize + 15]++;
                        }else if(sets[i].blks[j]->curScheme  == 5){
                            SchemeCount[sets[i].blks[j]->compactSize + 7]++; // hanled by companion block
                            //sectorCount[((int)sets[i].blks[j]->dictionary.size()-1)/4]++;
                        }else if(sets[i].blks[j]->curScheme == 1){ //DISH handles it
							SchemeCount[sets[i].blks[j]->compactSize]++;
						}else if(sets[i].blks[j]->curScheme == 2){
							SchemeCount[5]++; // extended keys handle it
						}else if(sets[i].blks[j]->curScheme == 6){ 
							SchemeCount[6]++;
						}else if(sets[i].blks[j]->curScheme == 7){
							SchemeCount[7]++; 
						}
						if(sets[i].blks[j]->curScheme != 1 && sets[i].blks[j]->curScheme != 0){
							countSideKeys(sets[i].blks[j]);
						}
					}
				}
				if(sets[i].blks[j]->isValid())
					endBlocks2++;
			}
		}
        if(avgCompression.value() > 0 ){
    		std::cout<<"total blocks at the end: " <<endBlocks.value() <<" ratio " << avgCompression.value()/numOfInsertion.value() <<" with insertions " <<numOfInsertion.value() << " sum= " << avgCompression.value() <<" all zero " << similarity[0].value()<< " compressible by DISH " << similarity[1].value() << " total "<<similarity.total() - similarity[0].value() - similarity[1].value()<<std::endl;
            for(int  i= 0; i <20; i++){
                std::cout<<i<<" schemes distribution :" <<SchemeCount[i].value()<<std::endl;
            }
            if(dictLimit == 10 )std::cout<< " noBuddy :" <<noBuddy[0].value() <<" || " <<noBuddy[1].value() <<std::endl;
            for(int  j= 0; j <16; j++){
                std::cout<< j*4 <<" keys distribution :" <<sectorCount[j].value()<<std::endl;
            }

        }
        // Defining a lambda function to compare two pairs. It will compare two pairs using second field
		
        if(sideCnt.size() >1){
			
			Comparator2 compFunctor2 =
			[](std::pair<uint64_t, int> elem1 ,std::pair<uint64_t, int> elem2)
			{
				return elem1.second > elem2.second;
			};
			std::set<std::pair<uint64_t, int>, Comparator2> setOfWords2(
				sideCnt.begin(), sideCnt.end(), compFunctor2);
			int i = 0;
			for (std::pair<uint64_t, int> element : setOfWords2){
				if(i > 30) break;				
				std::cout <<std::hex << element.first << " :: " ;
				std::cout<< std::dec<< element.second <<" -- "<< std::endl;
				i++;
			}
			sideCnt.clear();
		}
		/*if(patterns.size() > 1) {
			Comparator compFunctor =
			[](std::pair<std::vector<uint64_t>, int> elem1 ,std::pair<std::vector<uint64_t>, int> elem2)
			{
				return elem1.second > elem2.second;
			};
			// Declaring a set that will store the pairs using above comparision logic
			std::set<std::pair<std::vector<uint64_t>, int>, Comparator> setOfWords(
				patterns.begin(), patterns.end(), compFunctor);
			int i = 0;
			for (std::pair<std::vector<uint64_t>, int> element : setOfWords){
				if(i > 20) break;
				
				for (uint64_t word : element.first)
					std::cout <<std::hex << word << " :: " ;
				std::cout<< std::dec<< element.second <<" -- "<< std::endl;
				i++;
			}
			patterns.clear();
			
		}*/
		
}
