// Miguel Ramos, 2012.
// vim: set et sw=4 sts=4 ts=8:

// This CS is meant for a 1-D domain.
// It expects the following resources to be set up:
//  constant buffer 0:
//      items per group x, 1, 1, 0
//      number of groups X, 1, 1, 0
//      mat(0,0) mat(0,1) mat(0,2) mat(0,3)
//      mat(1,0) mat(1,1) mat(1,2) mat(1,3)
//      mat(2,0) mat(2,1) mat(2,2) mat(2,3)
//      mat(3,0) mat(3,1) mat(3,2) mat(3,3)
//
//  RAT resource 0 (output buffer) with 4 floats per work-item.
//  VTX resource 0 (intput buffer) with 4 floats per work-item.

ALU: KCACHE_BANK0(0) KCACHE_MODE0.CF_KCACHE_LOCK_1 BARRIER;
    // R0.x <- R0.x + R1.x * Kcache_bank0(0)
    MULADD_UINT24: DST_GPR(0) DST_CHAN.CHAN_X
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_X
        SRC1_SEL.Kcache_bank0(0) SRC1_CHAN.CHAN_X
        SRC2_SEL.GPR(0) SRC2_CHAN.CHAN_X LAST;
    // R1.x <- R0.x << 2
    LSHL_INT: DST_GPR(1) DST_CHAN.CHAN_X WRITE_MASK
        SRC0_SEL.GPR(0) SRC0_CHAN.CHAN_X
        SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_X LAST;
    0x00000002 0x00000000;

TC: BARRIER;
    FETCH: FETCH_TYPE.VTX_FETCH_NO_INDEX_OFFSET BUFFER_ID(1) SRC_GPR(1) SRC_SEL_X.SEL_X MEGA_FETCH_COUNT(15);
        DST_GPR(2) DST_SEL_X.SEL_X DST_SEL_Y.SEL_Y DST_SEL_Z.SEL_Z DST_SEL_W.SEL_1 USE_CONST_FIELDS;
        MEGA_FETCH;

ALU: KCACHE_BANK0(0) KCACHE_MODE0.CF_KCACHE_LOCK_1 BARRIER;

    DOT4: DST_CHAN.CHAN_X DST_GPR(3) WRITE_MASK
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_X
        SRC1_SEL.Kcache_bank0(2) SRC1_CHAN.CHAN_X;
    DOT4: DST_CHAN.CHAN_Y
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Y
        SRC1_SEL.Kcache_bank0(2) SRC1_CHAN.CHAN_Y;
    DOT4: DST_CHAN.CHAN_Z
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Z
        SRC1_SEL.Kcache_bank0(2) SRC1_CHAN.CHAN_Z;
    DOT4: DST_CHAN.CHAN_W
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_W
        SRC1_SEL.Kcache_bank0(2) SRC1_CHAN.CHAN_W LAST;

    DOT4: DST_CHAN.CHAN_X
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_X
        SRC1_SEL.Kcache_bank0(3) SRC1_CHAN.CHAN_X;
    DOT4: DST_CHAN.CHAN_Y DST_GPR(3) WRITE_MASK
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Y
        SRC1_SEL.Kcache_bank0(3) SRC1_CHAN.CHAN_Y;
    DOT4: DST_CHAN.CHAN_Z
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Z
        SRC1_SEL.Kcache_bank0(3) SRC1_CHAN.CHAN_Z;
    DOT4: DST_CHAN.CHAN_W
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_W
        SRC1_SEL.Kcache_bank0(3) SRC1_CHAN.CHAN_W LAST;

    DOT4: DST_CHAN.CHAN_X
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_X
        SRC1_SEL.Kcache_bank0(4) SRC1_CHAN.CHAN_X;
    DOT4: DST_CHAN.CHAN_Y
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Y
        SRC1_SEL.Kcache_bank0(4) SRC1_CHAN.CHAN_Y;
    DOT4: DST_CHAN.CHAN_Z DST_GPR(3) WRITE_MASK
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Z
        SRC1_SEL.Kcache_bank0(4) SRC1_CHAN.CHAN_Z;
    DOT4: DST_CHAN.CHAN_W
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_W
        SRC1_SEL.Kcache_bank0(4) SRC1_CHAN.CHAN_W LAST;

    DOT4: DST_CHAN.CHAN_X
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_X
        SRC1_SEL.Kcache_bank0(5) SRC1_CHAN.CHAN_X;
    DOT4: DST_CHAN.CHAN_Y
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Y
        SRC1_SEL.Kcache_bank0(5) SRC1_CHAN.CHAN_Y;
    DOT4: DST_CHAN.CHAN_Z
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_Z
        SRC1_SEL.Kcache_bank0(5) SRC1_CHAN.CHAN_Z;
    DOT4: DST_CHAN.CHAN_W DST_GPR(3) WRITE_MASK
        SRC0_SEL.GPR(2) SRC0_CHAN.CHAN_W
        SRC1_SEL.Kcache_bank0(5) SRC1_CHAN.CHAN_W LAST;

MEM_RAT_CACHELESS:
    RAT_ID(0) RAT_INST.EXPORT_RAT_INST_STORE_RAW TYPE(1) RW_GPR(3) INDEX_GPR(0) ELEM_SIZE(3)
    COMP_MASK(15) BARRIER END_OF_PROGRAM;

end;
