/*
 * Copyright (c) 2011-2012, 2014 ARM Limited
 * Copyright (c) 2010 The University of Edinburgh
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

#ifndef __CPU_PRED_VALUEPRED_HH__
#define __CPU_PRED_VALUEPRED_HH__


#include "params/ValuePredictor.hh"
#include "sim/sim_object.hh"
#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/static_inst.hh"




class ValuePredictor : public SimObject
{
  public:
      typedef ValuePredictorParams Params;
    /**
     * @param params The params object, that has the size of the BP and BTB.
     */
    ValuePredictor(const Params *p);

    /*shift amount to shift the PC by*/
    unsigned instShiftAmt;

    struct prediction
    {
        prediction()
            :valid(false),value(0)
        {}

        bool valid;
        double value;
    };

    //pure virtual function for - predict. Overridden by lastVP, VTAGE, etc. 
    virtual struct prediction predict(Addr pc)
    {
	prediction p;
	return p;
    }
    //pure virtual function for - update. Overridden by lastVP, VTAGE, etc. 
    virtual void update(Addr pc,double value)
    {
	return ;
    }

    virtual bool getPrediction(uint64_t seq_no, uint64_t pc, uint8_t piece, uint8_t* bHist, int pHist, double& predicted_value)
    {
	return false;
    }

    virtual void updateValuePredictor(uint64_t seq_no, uint8_t piece, double actual_value)
    {
	return;
    }

};

#endif // __CPU_PRED_VALUEPRED_HH__
