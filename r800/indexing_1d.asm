ALU:
  BARRIER;

  MULLO_INT:
    SRC0_SEL.GPR(1) SRC0_CHAN.CHAN_X ///< group_ID.x
    SRC1_SEL.ALU_SRC_LITERAL SRC1_CHAN.CHAN_X 
    WRITE_MASK DST_GPR(2) DST_CHAN.CHAN_X;
    LAST;
    0x00000100;
    0x00000000;

  ADD_INT:
    SRC0_SEL.GPR(0) SRC0_CHAN.CHAN_X ///< local_ID.x
    SRC1_SEL.GPR(2) SRC1_CHAN.CHAN_X 
    WRITE_MASK DST_GPR(3) DST_CHAN.CHAN_X;
    LAST;
  
MEM_RAT_CACHELESS:
  RAT_ID(11) RAT_INST.EXPORT_RAT_INST_STORE_RAW TYPE(1) RW_GPR(3) INDEX_GPR(3) ELEM_SIZE(0);
  COMP_MASK(1) BARRIER ARRAY_SIZE(0);

NOP: 
  BARRIER END_OF_PROGRAM;

end;
