// Miguel Ramos, 2012.
// vim: set et sw=4 sts=4 ts=8:

// This kernel is meant for a 1-D domain with one group.
ALU: BARRIER;

    MOV:
        DST_GPR(1) DST_CHAN.CHAN_X
        SRC0_SEL.ALU_SRC_TIME_LO WRITE_MASK LAST;

    MOV:
        DST_GPR(1) DST_CHAN.CHAN_Y
        SRC0_SEL.ALU_SRC_TIME_LO WRITE_MASK LAST;

    MOV:
        DST_GPR(1) DST_CHAN.CHAN_Z
        SRC0_SEL.ALU_SRC_TIME_LO WRITE_MASK LAST;

    MOV:
        DST_GPR(1) DST_CHAN.CHAN_W
        SRC0_SEL.ALU_SRC_TIME_LO WRITE_MASK LAST;

    SUB_INT:
        DST_GPR(1) DST_CHAN.CHAN_Y
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_Y
        SRC1_SEL.GPR(1) SRC1_CHAN.CHAN_X WRITE_MASK;
    SUB_INT:
        DST_GPR(1) DST_CHAN.CHAN_Z
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_Z
        SRC1_SEL.GPR(1) SRC1_CHAN.CHAN_X WRITE_MASK;
    SUB_INT:
        DST_GPR(1) DST_CHAN.CHAN_W
        SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_W
        SRC1_SEL.GPR(1) SRC1_CHAN.CHAN_X WRITE_MASK LAST;

MEM_RAT_CACHELESS:
    RAT_ID(1) RAT_INST.EXPORT_RAT_INST_STORE_RAW TYPE(1) RW_GPR(1) INDEX_GPR(0) ELEM_SIZE(3)
    COMP_MASK(15) BARRIER END_OF_PROGRAM;

end;
