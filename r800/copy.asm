/*
 * Copyright 2011 StreamNovation Ltd. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY StreamNovation Ltd. ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL StreamNovation Ltd. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of StreamNovation Ltd.
 *
 *
 * Author(s):
 *          Adam Rak <adam.rak@streamnovation.com>
 *    
 *    
 *    
 */

ALU:
  BARRIER;
  
  MULLO_INT:
    SRC0_SEL.GPR(0) SRC0_CHAN.CHAN_X ///< local_ID.x
    SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_X 
    WRITE_MASK DST_GPR(0) DST_CHAN.CHAN_X;
    LAST;
    0x4;
    0x0;
    
  MULLO_INT:
    SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_X ///< group_ID.x
    SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_X 
    WRITE_MASK DST_GPR(1) DST_CHAN.CHAN_X;
    LAST;
    0x400;
    0x0;

  ADD_INT:
    SRC0_SEL.GPR(0) SRC0_CHAN.CHAN_X
    SRC1_SEL.GPR(1) SRC1_CHAN.CHAN_X 
    WRITE_MASK DST_GPR(0) DST_CHAN.CHAN_X;
    LAST;
  
TC:
  BARRIER;

  FETCH:
    FETCH_TYPE.VTX_FETCH_NO_INDEX_OFFSET BUFFER_ID(0) SRC_GPR(0) SRC_SEL_X.SEL_X MEGA_FETCH_COUNT(15);
    DST_GPR(1) DST_SEL_X.SEL_X DST_SEL_Y.SEL_Y DST_SEL_Z.SEL_Z DST_SEL_W.SEL_W USE_CONST_FIELDS;
    MEGA_FETCH;

MEM_RAT_CACHELESS:
  RAT_ID(11) RAT_INST.EXPORT_RAT_INST_STORE_RAW TYPE(1) RW_GPR(1) INDEX_GPR(0) ELEM_SIZE(0);
  COMP_MASK(15) BARRIER ARRAY_SIZE(0);

NOP: 
  BARRIER END_OF_PROGRAM;

end;
