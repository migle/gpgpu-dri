// Miguel Ramos, 2012.
// vim: set et sw=4 sts=4 ts=8:

// This CS is meant for a 1-D domain.
// It expects the following resources to be set up:
//  constant buffer 0:
//      items per group x, 1, 1, 0
//      number of groups X, 1, 1, 0
//  RAT resource 0 (output buffer) with one dwords per work-item.

// R0.x <- local_ID + group_ID * group_size
ALU: KCACHE_BANK0(0) KCACHE_MODE0.CF_KCACHE_LOCK_1 BARRIER;
    // R0.x <- R0.x + R1.x * Kcache_bank0(0)
    MULADD_UINT24: DST_GPR(0) DST_CHAN.CHAN_X
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_X
        SRC1_SEL.Kcache_bank0(0) SRC1_CHAN.CHAN_X
        SRC2_SEL.GPR(0) SRC2_CHAN.CHAN_X LAST;

    // R1.w <- TIME
    MOV: DST_GPR(1) DST_CHAN.CHAN_W
        SRC0_SEL.ALU_SRC_TIME_LO WRITE_MASK LAST;

// if (R0.x % 4 == 1 && R1.x % 2 == 0 || R1.x % 4 == 1)
ALU_PUSH_BEFORE: BARRIER;
    // R127.x <- R0.x & 3, R127.y <- R1.x & 1, R127.z <- R1.x & 3
    AND_INT: DST_GPR(127) DST_CHAN.CHAN_X
        SRC0_SEL.GPR(0) SRC0_CHAN.CHAN_X
        SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_X WRITE_MASK;
    AND_INT: DST_GPR(127) DST_CHAN.CHAN_Y
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_X BANK_SWIZZLE.ALU_VEC_120
        SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_Y WRITE_MASK;
    AND_INT: DST_GPR(127) DST_CHAN.CHAN_Z
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_X BANK_SWIZZLE.ALU_VEC_120
        SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_X WRITE_MASK LAST;
    0x00000003 0x00000001;

    // R127.x == 1 ?
    PREDE_INT: DST_CHAN.CHAN_X UPDATE_PRED UPDATE_EXEC_MASK
        SRC0_SEL.GPR(127) SRC0_CHAN.CHAN_X
        SRC1_SEL.ALU_SRC_1_INT SRC1_CHAN.CHAN_X LAST;
    // && R127.y == 0 ?
    PREDE_INT: DST_CHAN.CHAN_Y UPDATE_PRED UPDATE_EXEC_MASK 
        SRC0_SEL.GPR(127) SRC0_CHAN.CHAN_Y
        SRC1_SEL.ALU_SRC_0 SRC1_CHAN.CHAN_Y LAST PRED_SEL.PRED_SEL_ONE;
    // || R127.z == 1 ?
    PREDE_INT: DST_CHAN.CHAN_Z UPDATE_PRED UPDATE_EXEC_MASK 
        SRC0_SEL.GPR(127) SRC0_CHAN.CHAN_Z
        SRC1_SEL.ALU_SRC_1_INT SRC1_CHAN.CHAN_Z LAST PRED_SEL.PRED_SEL_ZERO;

JUMP: ADDR(@1);
// {
    // R2.x <- 0xdead0000
    ALU:
        MOV: DST_GPR(2) DST_CHAN.CHAN_X
            SRC0_SEL.ALU_SRC_LITERAL SRC0_CHAN.CHAN_X WRITE_MASK LAST;
        0xdead0000 0x00000000;
// }
@1
ELSE: ADDR(@2) POP_COUNT(1);
// {
    // R2.x <- 0xbeef0000
    ALU_POP_AFTER:
        MOV: DST_GPR(2) DST_CHAN.CHAN_X
            SRC0_SEL.ALU_SRC_LITERAL SRC0_CHAN.CHAN_X WRITE_MASK LAST;
        0xbeef0000 0x00000000;
// }

@2
ALU: BARRIER;
    // PV.x <- TIME - R1.w
    SUB_INT: DST_CHAN.CHAN_X
        SRC0_SEL.ALU_SRC_TIME_LO
        SRC1_SEL.GPR(1) SRC1_CHAN.CHAN_W LAST;
    // R2.x <- R2.x | PV.x
    OR_INT: DST_GPR(2) DST_CHAN.CHAN_X
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_X
        SRC1_SEL.ALU_SRC_PV SRC1_CHAN.CHAN_X WRITE_MASK LAST;

// rat0[R0.x] <- R2.x
MEM_RAT_CACHELESS:
    RAT_ID(0) RAT_INST.EXPORT_RAT_INST_STORE_RAW TYPE(1) RW_GPR(2) INDEX_GPR(0) ELEM_SIZE(0)
    COMP_MASK(1) BARRIER END_OF_PROGRAM;

end;
