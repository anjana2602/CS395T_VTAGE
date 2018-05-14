# Copyright (c) 2012 Mark D. Hill and David A. Wood
# Copyright (c) 2015 The University of Wisconsin
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Pawan Joshi

from m5.SimObject import SimObject
from m5.params import *



class ValuePredictor(SimObject):
    type = 'ValuePredictor'
    cxx_class = "ValuePredictor"
    cxx_header = "cpu/pred/valuepred.hh"
    abstract = True

    instShiftAmt = Param.Unsigned(2,"amount to the shift the PC by")


class lastVP(ValuePredictor):
    type = 'lastVP'
    cxx_class = "lastVP"
    cxx_header = "cpu/pred/lastVP.hh"

    predictorSize = Param.Unsigned(1024,"Size of the last value prediction table")
    tagBits = Param.Unsigned(16,"number of tag bits")

class VTAGE(ValuePredictor):
    type = 'VTAGE'
    cxx_class = "VTAGE"
    cxx_header = "cpu/pred/VTAGE.hh"

