/*
 * Copyright (c) 2013,2016 ARM Limited
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
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
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
 *          Ron Dreslinski
 */

/**
 * @file
 * Definitions of BaseTags.
 */

#include "mem/cache/tags/base.hh"

#include "cpu/smt.hh" //maxThreadsPerCPU
#include "mem/cache/base.hh"
#include "sim/sim_exit.hh"

using namespace std;

BaseTags::BaseTags(const Params *p)
    : ClockedObject(p), blkSize(p->block_size), blkMask(blkSize - 1),
      size(p->size),secSize(p->sector_size), compactLimit(p->compactLimit), dictLimit(p->dictLimit), entrySize(p->entry_size),
      lookupLatency(p->tag_latency),
      accessLatency(p->sequential_access ?
                    p->tag_latency + p->data_latency :
                    std::max(p->tag_latency, p->data_latency)),
      cache(nullptr), warmupBound(0),
      warmedUp(false), numBlocks(0), patterns()
{
}

void
BaseTags::setCache(BaseCache *_cache)
{
    assert(!cache);
    cache = _cache;
}

void
BaseTags::regStats()
{
    ClockedObject::regStats();

    using namespace Stats;

    replacements
        .init(maxThreadsPerCPU)
        .name(name() + ".replacements")
        .desc("number of replacements")
        .flags(total)
        ;

    tagsInUse
        .name(name() + ".tagsinuse")
        .desc("Cycle average of tags in use")
        ;
	avgCompression
		.name(name() +".avg_compression_sum");
	selfCompressiblity
		.init(17)
		.name(name()+".self_compress")
		.flags(total);
		
	selfCompressiblity2
		.init(7)
		.name(name()+".self_compress2")
		.flags(total);
    totalRefs
        .name(name() + ".total_refs")
        .desc("Total number of references to valid blocks.")
        ;
	totalBlocks
		.name(name()+".totalblocks")
		.desc("num Of Blocks");
	endBlocks
		.name(name()+".endblocks")
		.desc("num Of Blocks at end");
	endBlocks2
		.name(name()+".endblocks2")
		.desc("num Of Blocks at end");
    numOfInsertion
		.name(name()+".numOfInsertion")
		.desc("num Of Insertion");
	maxCompacted
		.name(name()+".maxCompacted");
	selfNoCompress
		.name(name()+".selfNoCompress");
	similarity
		.init(32)
		.name(name()+".similarityPatterns");
    sectorCount
        .init(17)
        .name(name()+".keysInSector");
    SchemeCount
        .init(21)
        .name(name()+".schemeused");
    noBuddy
        .init(2)
        .name(name()+".noBuddy");
    sameSector.
		name(name()+".sameSector");
	numofReplacement.
		name(name()+"numofReplacement");
	sectorRatio.
		name(name()+"avgSectorSize");
	sectorRatio = 	sameSector/numofReplacement;
    compactSizeCount
		.init(compactLimit == 0? secSize+1: compactLimit+1)
		.name(name()+".compactCount")
		.flags(total);
	compactedCount
		.init(compactLimit+1)
		.name(name()+".compactedCount")
		.flags(total);
	compressionRatio
		.name(name() + ".compressionRatio")
        .desc("compression ratio.")
        ;
    compressResult
		.name(name() + ".compression_avg")
        .desc("compress happened.")
        ;
	compressionRatio = endBlocks * blkSize / size;
	compressResult= avgCompression/numOfInsertion;
	schemeUsed
		.init(5)
		.name(name()+".scheme")
		.desc("num Of sschemes");

    sampledRefs
        .name(name() + ".sampled_refs")
        .desc("Sample count of references to valid blocks.")
        ;

    avgRefs
        .name(name() + ".avg_refs")
        .desc("Average number of references to valid blocks.")
        ;

    avgRefs = totalRefs/sampledRefs;

    warmupCycle
        .name(name() + ".warmup_cycle")
        .desc("Cycle when the warmup percentage was hit.")
        ;

    occupancies
        .init(cache->system->maxMasters())
        .name(name() + ".occ_blocks")
        .desc("Average occupied blocks per requestor")
        .flags(nozero | nonan)
        ;
    for (int i = 0; i < cache->system->maxMasters(); i++) {
        occupancies.subname(i, cache->system->getMasterName(i));
    }

    avgOccs
        .name(name() + ".occ_percent")
        .desc("Average percentage of cache occupancy")
        .flags(nozero | total)
        ;
    for (int i = 0; i < cache->system->maxMasters(); i++) {
        avgOccs.subname(i, cache->system->getMasterName(i));
    }

    avgOccs = occupancies / Stats::constant(numBlocks);

    occupanciesTaskId
        .init(ContextSwitchTaskId::NumTaskId)
        .name(name() + ".occ_task_id_blocks")
        .desc("Occupied blocks per task id")
        .flags(nozero | nonan)
        ;

    ageTaskId
        .init(ContextSwitchTaskId::NumTaskId, 5)
        .name(name() + ".age_task_id_blocks")
        .desc("Occupied blocks per task id")
        .flags(nozero | nonan)
        ;

    percentOccsTaskId
        .name(name() + ".occ_task_id_percent")
        .desc("Percentage of cache occupancy per task id")
        .flags(nozero)
        ;

    percentOccsTaskId = occupanciesTaskId / Stats::constant(numBlocks);

    tagAccesses
        .name(name() + ".tag_accesses")
        .desc("Number of tag accesses")
        ;

    dataAccesses
        .name(name() + ".data_accesses")
        .desc("Number of data accesses")
        ;

    registerDumpCallback(new BaseTagsDumpCallback(this));
    registerExitCallback(new BaseTagsCallback(this));
}
