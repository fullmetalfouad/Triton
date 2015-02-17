#include <iostream>
#include <sstream>
#include <stdexcept>

#include "AddIRBuilder.h"
#include "SMT2Lib.h"
#include "SymbolicElement.h"



AddIRBuilder::AddIRBuilder(uint64_t address, const std::string &disassembly):
  BaseIRBuilder(address, disassembly) {
}


void AddIRBuilder::regImm(const ContextHandler &ctxH, AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr;
  uint64_t          reg     = std::get<1>(_operands[0]);
  uint64_t          imm     = std::get<1>(_operands[1]);

  uint64_t          symReg  = ap.getRegSymbolicID(ctxH.translateRegID(reg));
  uint32_t          regSize = ctxH.getRegisterSize(reg);

  /* Create the SMT semantic */
  if (symReg != UNSET)
    expr << "(bvadd #" << std::dec << symReg << " " << smt2lib::bv(imm, regSize) << ")";
  else 
    expr << "(bvadd " << smt2lib::bv(ctxH.getRegisterValue(reg), regSize) << " " << smt2lib::bv(imm, regSize) << ")";

  /* Create the symbolic element */
  se = ap.createRegSE(expr, ctxH.translateRegID(reg));

  /* Apply the taint */
  ap.aluSpreadTaintRegImm(se, ctxH.translateRegID(reg));

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  //inst.addElement(this->af(se, ap));
  //inst.addElement(this->cf(se, ap));
  //inst.addElement(this->of(se, ap));
  //inst.addElement(this->pf(se, ap));
  //inst.addElement(this->sf(se, ap));
  inst.addElement(this->zf(se, ap));
}


void AddIRBuilder::regReg(const ContextHandler &ctxH, AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint64_t          reg1    = std::get<1>(_operands[0]);
  uint64_t          reg2    = std::get<1>(_operands[1]);

  uint64_t          symReg1 = ap.getRegSymbolicID(ctxH.translateRegID(reg1));
  uint64_t          symReg2 = ap.getRegSymbolicID(ctxH.translateRegID(reg2));
  uint32_t          regSize = ctxH.getRegisterSize(reg1);
  

  /* Create the SMT semantic */
  // OP_1
  if (symReg1 != UNSET)
    op1 << "#" << std::dec << symReg1;
  else
    op1 << smt2lib::bv(ctxH.getRegisterValue(reg1), regSize);
    
  // OP_2
  if (symReg2 != UNSET)
    op2 << "#" << std::dec << symReg2;
  else
    op2 << smt2lib::bv(ctxH.getRegisterValue(reg2), regSize);

  // Final expr
  expr << "(bvadd " << op1.str() << " " << op2.str() << ")";

  /* Create the symbolic element */
  se = ap.createRegSE(expr, ctxH.translateRegID(reg1));

  /* Apply the taint */
  ap.aluSpreadTaintRegReg(se, ctxH.translateRegID(reg1), ctxH.translateRegID(reg2));

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  //inst.addElement(this->af(se, ap));
  //inst.addElement(this->cf(se, ap));
  //inst.addElement(this->of(se, ap));
  //inst.addElement(this->pf(se, ap));
  //inst.addElement(this->sf(se, ap));
  inst.addElement(this->zf(se, ap));
}


void AddIRBuilder::regMem(const ContextHandler &ctxH, AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint32_t          readSize = std::get<2>(_operands[1]);
  uint64_t          mem      = std::get<1>(_operands[1]);
  uint64_t          reg      = std::get<1>(_operands[0]);

  uint64_t          symReg   = ap.getRegSymbolicID(ctxH.translateRegID(reg));
  uint64_t          symMem   = ap.getMemorySymbolicID(mem);

  /* Create the SMT semantic */
  // OP_1
  if (symReg != UNSET)
    op1 << "#" << std::dec << symReg;
  else
    op1 << smt2lib::bv(ctxH.getRegisterValue(reg), readSize);
    
  // OP_2
  if (symMem != UNSET)
    op2 << "#" << std::dec << symMem;
  else
    op2 << smt2lib::bv(ctxH.getMemoryValue(mem, readSize), readSize);

  // Final expr
  expr << "(bvadd " << op1.str() << " " << op2.str() << ")";

  /* Create the symbolic element */
  se = ap.createRegSE(expr, ctxH.translateRegID(reg));

  /* Apply the taint */
  ap.aluSpreadTaintRegMem(se, ctxH.translateRegID(reg), mem, readSize);
  
  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  //inst.addElement(this->af(se, ap));
  //inst.addElement(this->cf(se, ap));
  //inst.addElement(this->of(se, ap));
  //inst.addElement(this->pf(se, ap));
  //inst.addElement(this->sf(se, ap));
  inst.addElement(this->zf(se, ap));
}


void AddIRBuilder::memImm(const ContextHandler &ctxH, AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr;
  uint32_t          writeSize = std::get<2>(_operands[0]);
  uint64_t          mem       = std::get<1>(_operands[0]);
  uint64_t          imm       = std::get<1>(_operands[1]);

  uint64_t          symMem    = ap.getMemorySymbolicID(mem);

  /* Create the SMT semantic */
  if (symMem != UNSET)
    expr << "(bvadd #" << std::dec << symMem << " " << smt2lib::bv(imm, writeSize) << ")";
  else 
    expr << "(bvadd " << smt2lib::bv(ctxH.getMemoryValue(mem, writeSize), writeSize) << " " << smt2lib::bv(imm, writeSize) << ")";

  /* Create the symbolic element */
  se = ap.createMemSE(expr, mem);

  /* Apply the taint */
  ap.aluSpreadTaintMemImm(se, mem, writeSize);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  //inst.addElement(this->af(se, ap));
  //inst.addElement(this->cf(se, ap));
  //inst.addElement(this->of(se, ap));
  //inst.addElement(this->pf(se, ap));
  //inst.addElement(this->sf(se, ap));
  inst.addElement(this->zf(se, ap));
}


void AddIRBuilder::memReg(const ContextHandler &ctxH, AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint32_t          writeSize = std::get<2>(_operands[0]);
  uint64_t          mem       = std::get<1>(_operands[0]);
  uint64_t          reg       = std::get<1>(_operands[1]);

  uint64_t          symReg    = ap.getRegSymbolicID(ctxH.translateRegID(reg));
  uint64_t          symMem    = ap.getMemorySymbolicID(mem);

  /* Create the SMT semantic */
  // OP_1
  if (symMem != UNSET)
    op1 << "#" << std::dec << symMem;
  else
    op1 << smt2lib::bv(ctxH.getMemoryValue(mem, writeSize), writeSize);

  // OP_1
  if (symReg != UNSET)
    op2 << "#" << std::dec << symReg;
  else
    op2 << smt2lib::bv(ctxH.getRegisterValue(reg), writeSize);

  // Final expr
  expr << "(bvadd " << op1.str() << " " << op2.str() << ")";

  /* Create the symbolic element */
  se = ap.createMemSE(expr, mem);

  /* Apply the taint */
  ap.aluSpreadTaintMemReg(se, mem, ctxH.translateRegID(reg), writeSize);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

  /* Add the symbolic flags element to the current inst */
  //inst.addElement(this->af(se, ap));
  //inst.addElement(this->cf(se, ap));
  //inst.addElement(this->of(se, ap));
  //inst.addElement(this->pf(se, ap));
  //inst.addElement(this->sf(se, ap));
  inst.addElement(this->zf(se, ap));
}


Inst *AddIRBuilder::process(const ContextHandler &ctxH, AnalysisProcessor &ap) const {
  checkSetup();

  Inst *inst = new Inst(_address, _disas);

  try {
    this->templateMethod(ctxH, ap, *inst, _operands, "ADD");
  }
  catch (std::exception &e) {
    delete inst;
    throw e;
  }

  return inst;
}

