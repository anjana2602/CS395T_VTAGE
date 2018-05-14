/*
 * Copyright (c) 2011-2012, 2014 ARM Limited
 * Copyright (c) 2010 The University of Edinburgh
 * Copyright (c) 2012 Mark D. Hill and David A. Wood
 * All rights reserved
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
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
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
 * Authors: Pawan Joshi
 */

#include "cpu/pred/lastVP.hh"

#include <algorithm>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/VP.hh"

lastVP::lastVP(const lastVPParams *params)
    : ValuePredictor(params),
      predictorSize(params->predictorSize),
      tagBits(params->tagBits)
{
    if (!isPowerOf2(predictorSize))
        fatal("Invalid value predictor size - must be power of two!\n");
                  
    indexMask = predictorSize - 1; 

    /*create the prediction table*/

    value_prediction_table.resize(predictorSize);

    for(int i=0; i<predictorSize; i++)
    {
        value_prediction_table[i].valid=false;
    }

    tagMask = (1<< tagBits) -1;
    
    tagShiftAmt = instShiftAmt + floorLog2(predictorSize);

    DPRINTF(VP, "Created the last value preditor object\n");
}

void
lastVP::reset()
{
    for(int i=0; i<predictorSize; i++)
    {
        value_prediction_table[i].valid=false;
    }
}

struct lastVP::prediction
lastVP::predict(Addr pc)
{
    //DPRINTF(VP, "Predict called\n");
    struct prediction P;
    P.valid = false;
    P.value = 0;

    unsigned idx = getVPT_Index(pc);
    Addr inst_tag = getTag(pc);    

    if(value_prediction_table[idx].valid && inst_tag == value_prediction_table[idx].tag)
    {
        DPRINTF(VP,"Found value\n");
        P.valid = true;
        P.value = value_prediction_table[idx].value;
    }
    
   
    return P;
   
}

unsigned 
lastVP::getVPT_Index(Addr pc)
{
    return (pc >> instShiftAmt) & indexMask;
}

Addr
lastVP::getTag(Addr pc)
{
    return (pc >> tagShiftAmt) & tagMask;
}

void 
lastVP::update(Addr pc,double value)
{
	DPRINTF(VP, "Update called\n");
    unsigned idx = getVPT_Index(pc);

    value_prediction_table[idx].valid = true;
    value_prediction_table[idx].value = value;
    value_prediction_table[idx].tag = getTag(pc);

    DPRINTF(VP,"done updating predictor for PC = %u\n",pc);
}

lastVP*
lastVPParams::create()
{
        return new lastVP(this);
}

