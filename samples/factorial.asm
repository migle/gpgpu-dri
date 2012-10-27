// Miguel Ramos, 2012.
// vim: set et sw=4 sts=4 ts=8:

// This kernel is meant for a 1-D domain with one group.

// R1.x <- local_ID.x
ALU: BARRIER;
    MOV: DST_GPR(1) SRC0_SEL.GPR(0)
        WRITE_MASK LAST;
    
// R2.x <- factorial(R1.x)
CALL: COUNT(1) ADDR(@factorial);

MEM_RAT_CACHELESS:
    RAT_ID(0) RAT_INST.EXPORT_RAT_INST_STORE_RAW TYPE(1) RW_GPR(2) INDEX_GPR(0) ELEM_SIZE(0)
    COMP_MASK(1) BARRIER END_OF_PROGRAM;

@factorial
ALU_PUSH_BEFORE: BARRIER;

    PRED_SETGT_INT: SRC0_SEL.GPR(1) SRC1_SEL.ALU_SRC_1_INT
        UPDATE_PRED UPDATE_EXEC_MASK LAST;

    MOV: DST_GPR(2) SRC0_SEL.ALU_SRC_1_INT
        WRITE_MASK PRED_SEL.PRED_SEL_ZERO LAST;

JUMP: POP_COUNT(1) ADDR(@return);

ALU: BARRIER;
    SUB_INT: DST_GPR(1) SRC0_SEL.GPR(1) SRC1_SEL.ALU_SRC_1_INT
        WRITE_MASK LAST;

CALL: COUNT(1) ADDR(@factorial);

ALU_POP_AFTER: BARRIER;

    ADD_INT: DST_GPR(1) SRC0_SEL.GPR(1) SRC1_SEL.ALU_SRC_1_INT
        WRITE_MASK LAST;
    MULLO_INT: DST_GPR(2) SRC0_SEL.GPR(2) SRC1_SEL.ALU_SRC_PV
        WRITE_MASK LAST;

@return
RETURN: BARRIER;

end;
