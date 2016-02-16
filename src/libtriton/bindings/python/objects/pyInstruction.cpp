//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the LGPLv3 License.
*/

#ifdef TRITON_PYTHON_BINDINGS

#include <api.hpp>
#include <pythonObjects.hpp>
#include <pythonUtils.hpp>
#include <pythonXFunctions.hpp>



/*! \page py_Instruction_page Instruction
    \brief [**python api**] All information about the Instruction python object.

\tableofcontents

\section py_Instruction_description Description
<hr>

This object is used to represent an Instruction.

~~~~~~~~~~~~~{.py}
import  sys
from    triton import *


trace = [
    (0x400000, "\x48\x8b\x05\xb8\x13\x00\x00"), # mov        rax, QWORD PTR [rip+0x13b8]
    (0x400007, "\x48\x8d\x34\xc3"),             # lea        rsi, [rbx+rax*8]
    (0x40000b, "\x67\x48\x8D\x74\xC3\x0A"),     # lea        rsi, [ebx+eax*8+0xa]
    (0x400011, "\x66\x0F\xD7\xD1"),             # pmovmskb   edx, xmm1
    (0x400015, "\x89\xd0"),                     # mov        eax, edx
    (0x400017, "\x80\xf4\x99"),                 # xor        ah, 0x99
]


if __name__ == '__main__':

    #Set the arch
    setArchitecture(ARCH.X86_64)

    for (addr, opcodes) in trace:

        # Build an instruction
        inst = Instruction()

        # Setup opcodes
        inst.setOpcodes(opcodes)

        # Setup Address
        inst.setAddress(addr)

        # optional - Update register state
        inst.updateContext(Register(REG.RAX, 0x4444444455555555));
        inst.updateContext(Register(REG.RBX, 0x1111111122222222));

        # optional - Add memory access <addr, size, content>
        inst.updateContext(Memory(0x66666666, 4, 0x31323334));

        # Process everything
        processing(inst)

        print inst
        for op in inst.getOperands():
            print '\t', op
            if op.getType() == OPERAND.MEM:
                print '\t\t base  : ', op.getBaseRegister()
                print '\t\t index : ', op.getIndexRegister()
                print '\t\t disp  : ', op.getDisplacement()
                print '\t\t scale : ', op.getScale()
        print

    sys.exit(0)
~~~~~~~~~~~~~

\section Instruction_py_api Python API - Methods of the Instruction class
<hr>

- **getAddress(void)**<br>
Returns the address of the instruction as integer.

- **getDisassembly(void)**<br>
Returns the disassembly of the instruction as string.

- **getFirstOperand(void)**<br>
Returns the first operand of the instruction.

- **getNextAddress(void)**<br>
Returns the next address of the instruction as integer.

- **getOpcodes(void)**<br>
Returns the opcodes of the instruction as bytes.

- **getOpcodesSize(void)**<br>
Returns the opcodes size of the instruction as integer.

- **getOperands(void)**<br>
Returns the operands of the instruction as list of \ref py_Immediate_page, \ref py_Memory_page or \ref py_Register_page.

- **getSecondOperand(void)**<br>
Returns the second operand of the instruction.

- **getThirdOperand(void)**<br>
Returns the third operand of the instruction.

- **getSymbolicExpressions(void)**<br>
Returns the symbolic expression of the instruction as list of \ref py_SymbolicExpression_page.

- **getThreadId(void)**<br>
Returns the thread id of the instruction as integer.

- **getType(void)**<br>
Returns the type of the instruction as \ref py_OPCODE_page.

- **isBranch(void)**<br>
Returns true if the instruction modifies is a branch (i.e x86: JUMP, JCC).

- **isConditionTaken(void)**<br>
Returns true if the condition is taken (i.e x86: JCC, CMOVCC, SETCC, ...).

- **isControlFlow(void)**<br>
Returns true if the instruction modifies the control flow (i.e x86: JUMP, JCC, CALL, RET).

- **isTainted(void)**<br>
Returns true if at least one of its \ref py_SymbolicExpression_page is tainted.

- **setAddress(integer addr)**<br>
Sets the address of the instruction.

- **setOpcodes(bytes opcodes, integer size)**<br>
Sets the opcodes of the instruction.

- **setThreadId(integer tid)**<br>
Sets the thread id of the instruction.

- **updateContext(\ref py_Memory_page memCtx)**<br>
Updates the context of the instruction by adding a concrete value for a **LOAD** memory access. Please note that you don't have to define a **STORE**
concrete value, this value will be computed symbolically - **Only LOAD** accesses are necessary.

- **updateContext(\ref py_Register_page regCtx)**<br>
Updates the context of the instruction by adding a concrete value for a specific register. Be careful you cannot update the context on a flag.

*/



namespace triton {
  namespace bindings {
    namespace python {

      //! Instruction destructor.
      void Instruction_dealloc(PyObject* self) {
        delete PyInstruction_AsInstruction(self);
        Py_DECREF(self);
      }


      static PyObject* Instruction_getAddress(PyObject* self, PyObject* noarg) {
        return PyLong_FromUint(PyInstruction_AsInstruction(self)->getAddress());
      }


      static PyObject* Instruction_getDisassembly(PyObject* self, PyObject* noarg) {
        return PyString_FromFormat("%s", PyInstruction_AsInstruction(self)->getDisassembly().c_str());
      }


      static PyObject* Instruction_getFirstOperand(PyObject* self, PyObject* noarg) {
        triton::arch::Instruction*      inst;
        triton::uint32                  opSize;
        PyObject*                       obj = nullptr;

        inst     = PyInstruction_AsInstruction(self);
        opSize   = inst->operands.size();

        if (opSize < 1) {
          return PyErr_Format(PyExc_TypeError, "Instruction::getFirstOperand(): The instruction hasn't operands.");
        }


        if (inst->operands[0].getType() == triton::arch::OP_IMM) {
          auto imm = inst->operands[0].getImmediate();
          obj = PyImmediateOperand(imm);
        }
        else if (inst->operands[0].getType() == triton::arch::OP_MEM) {
          auto mem = inst->operands[0].getMemory();
          obj = PyMemoryOperand(mem);
        }
        else if (inst->operands[0].getType() == triton::arch::OP_REG) {
          auto reg = inst->operands[0].getRegister();
          obj = PyRegisterOperand(reg);
        }

        return obj;
      }


      static PyObject* Instruction_getNextAddress(PyObject* self, PyObject* noarg) {
        return PyLong_FromUint(PyInstruction_AsInstruction(self)->getNextAddress());
      }


      static PyObject* Instruction_getOpcodes(PyObject* self, PyObject* noarg) {
        const triton::uint8* opcodes = PyInstruction_AsInstruction(self)->getOpcodes();
        triton::uint32 size          = PyInstruction_AsInstruction(self)->getOpcodesSize();
        return PyBytes_FromStringAndSize(reinterpret_cast<const char*>(opcodes), size);
      }


      static PyObject* Instruction_getOpcodesSize(PyObject* self, PyObject* noarg) {
        return Py_BuildValue("k", PyInstruction_AsInstruction(self)->getOpcodesSize());
      }


      static PyObject* Instruction_getOperands(PyObject* self, PyObject* noarg) {
        triton::arch::ImmediateOperand  imm;
        triton::arch::MemoryOperand     mem;
        triton::arch::RegisterOperand   reg;
        triton::arch::Instruction*      inst;
        triton::uint32                  opSize;
        PyObject*                       operands;

        inst     = PyInstruction_AsInstruction(self);
        opSize   = inst->operands.size();
        operands = xPyList_New(opSize);

        for (triton::uint32 index = 0; index < opSize; index++) {
          PyObject* obj = nullptr;

          if (inst->operands[index].getType() == triton::arch::OP_IMM) {
            imm = inst->operands[index].getImmediate();
            obj = PyImmediateOperand(imm);
          }
          else if (inst->operands[index].getType() == triton::arch::OP_MEM) {
            mem = inst->operands[index].getMemory();
            obj = PyMemoryOperand(mem);
          }
          else if (inst->operands[index].getType() == triton::arch::OP_REG) {
            reg = inst->operands[index].getRegister();
            obj = PyRegisterOperand(reg);
          }
          else
            continue;

          PyList_SetItem(operands, index, obj);
        }

        return operands;
      }


      static PyObject* Instruction_getSecondOperand(PyObject* self, PyObject* noarg) {
        triton::arch::Instruction*      inst;
        triton::uint32                  opSize;
        PyObject*                       obj = nullptr;

        inst     = PyInstruction_AsInstruction(self);
        opSize   = inst->operands.size();

        if (opSize < 2) {
          return PyErr_Format(PyExc_TypeError, "Instruction::getSecondOperand(): The instruction hasn't second operand.");
        }


        if (inst->operands[1].getType() == triton::arch::OP_IMM) {
          auto imm = inst->operands[1].getImmediate();
          obj = PyImmediateOperand(imm);
        }
        else if (inst->operands[1].getType() == triton::arch::OP_MEM) {
          auto mem = inst->operands[1].getMemory();
          obj = PyMemoryOperand(mem);
        }
        else if (inst->operands[1].getType() == triton::arch::OP_REG) {
          auto reg = inst->operands[1].getRegister();
          obj = PyRegisterOperand(reg);
        }

        return obj;
      }

      static PyObject* Instruction_getSymbolicExpressions(PyObject* self, PyObject* noarg) {
        triton::arch::Instruction*  inst;
        triton::uint32              exprSize;
        PyObject*                   symExprs;

        inst     = PyInstruction_AsInstruction(self);
        exprSize = inst->symbolicExpressions.size();
        symExprs = xPyList_New(exprSize);

        for (triton::uint32 index = 0; index < exprSize; index++) {
          PyObject* obj = nullptr;
          obj = PySymbolicExpression(inst->symbolicExpressions[index]);
          PyList_SetItem(symExprs, index, obj);
        }

        return symExprs;
      }

      static PyObject* Instruction_getThirdOperand(PyObject* self, PyObject* noarg) {
        triton::arch::Instruction*      inst;
        triton::uint32                  opSize;
        PyObject*                       obj = nullptr;

        inst     = PyInstruction_AsInstruction(self);
        opSize   = inst->operands.size();

        if (opSize < 3) {
          return PyErr_Format(PyExc_TypeError, "Instruction::getThirdOperand(): The instruction hasn't third operand.");
        }


        if (inst->operands[2].getType() == triton::arch::OP_IMM) {
          auto imm = inst->operands[2].getImmediate();
          obj = PyImmediateOperand(imm);
        }
        else if (inst->operands[2].getType() == triton::arch::OP_MEM) {
          auto mem = inst->operands[2].getMemory();
          obj = PyMemoryOperand(mem);
        }
        else if (inst->operands[2].getType() == triton::arch::OP_REG) {
          auto reg = inst->operands[2].getRegister();
          obj = PyRegisterOperand(reg);
        }

        return obj;
      }


      static PyObject* Instruction_getThreadId(PyObject* self, PyObject* noarg) {
        return Py_BuildValue("k", PyInstruction_AsInstruction(self)->getThreadId());
      }


      static PyObject* Instruction_getType(PyObject* self, PyObject* noarg) {
        return Py_BuildValue("k", PyInstruction_AsInstruction(self)->getType());
      }


      static PyObject* Instruction_isBranch(PyObject* self, PyObject* noarg) {
        if (PyInstruction_AsInstruction(self)->isBranch() == true)
          Py_RETURN_TRUE;
        Py_RETURN_FALSE;
      }


      static PyObject* Instruction_isConditionTaken(PyObject* self, PyObject* noarg) {
        if (PyInstruction_AsInstruction(self)->isConditionTaken() == true)
          Py_RETURN_TRUE;
        Py_RETURN_FALSE;
      }


      static PyObject* Instruction_isControlFlow(PyObject* self, PyObject* noarg) {
        if (PyInstruction_AsInstruction(self)->isControlFlow() == true)
          Py_RETURN_TRUE;
        Py_RETURN_FALSE;
      }


      static PyObject* Instruction_isTainted(PyObject* self, PyObject* noarg) {
        if (PyInstruction_AsInstruction(self)->isTainted() == true)
          Py_RETURN_TRUE;
        Py_RETURN_FALSE;
      }


      static PyObject* Instruction_setAddress(PyObject* self, PyObject* addr) {
        if (!PyLong_Check(addr) && !PyInt_Check(addr))
          return PyErr_Format(PyExc_TypeError, "Instruction::setAddress(): Expected an integer as argument.");
        PyInstruction_AsInstruction(self)->setAddress(PyLong_AsUint(addr));
        Py_INCREF(Py_None);
        return Py_None;
      }


      static PyObject* Instruction_setOpcodes(PyObject* self, PyObject* opc) {

        if (!PyBytes_Check(opc))
          return PyErr_Format(PyExc_TypeError, "Instruction::setOpcodes(): Expected a bytes array as argument.");

        if (PyBytes_Size(opc) >= 32)
          return PyErr_Format(PyExc_TypeError, "Instruction::setOpcodes(): Invalid size (too big).");

        PyInstruction_AsInstruction(self)->setOpcodes(reinterpret_cast<triton::uint8*>(PyBytes_AsString(opc)), PyBytes_Size(opc));
        Py_INCREF(Py_None);
        return Py_None;
      }


      static PyObject* Instruction_setThreadId(PyObject* self, PyObject* tid) {

        if (!PyLong_Check(tid) && !PyInt_Check(tid))
          return PyErr_Format(PyExc_TypeError, "Instruction::setThreadId(): Expected an integer as argument.");

        PyInstruction_AsInstruction(self)->setThreadId(PyLong_AsUint(tid));
        Py_INCREF(Py_None);
        return Py_None;
      }


      static PyObject* Instruction_updateContext(PyObject* self, PyObject* ctx) {
        triton::arch::Instruction*     inst;
        triton::arch::MemoryOperand*   memCtx;
        triton::arch::RegisterOperand* regCtx;

        if (!PyMemoryOperand_Check(ctx) && !PyRegisterOperand_Check(ctx))
          return PyErr_Format(PyExc_TypeError, "Instruction::updateContext(): Expected a Memory or Register as argument.");

        inst = PyInstruction_AsInstruction(self);

        if (PyMemoryOperand_Check(ctx)) {
          memCtx = PyMemoryOperand_AsMemoryOperand(ctx);
          inst->updateContext(*memCtx);
        }

        else if (PyRegisterOperand_Check(ctx)) {
          regCtx = PyRegisterOperand_AsRegisterOperand(ctx);
          if (regCtx->isFlag())
            return PyErr_Format(PyExc_TypeError, "Instruction::updateContext(): You cannot update the context on an isolated flag.");
          inst->updateContext(*regCtx);
        }

        Py_INCREF(Py_None);
        return Py_None;
      }


      static int Instruction_print(PyObject* self) {
        std::cout << PyInstruction_AsInstruction(self);
        return 0;
      }


      static PyObject* Instruction_str(PyObject* self) {
        std::stringstream str;
        str << PyInstruction_AsInstruction(self);
        return PyString_FromFormat("%s", str.str().c_str());
      }


      //! Instruction methods.
      PyMethodDef Instruction_callbacks[] = {
        {"getAddress",                Instruction_getAddress,               METH_NOARGS,     ""},
        {"getDisassembly",            Instruction_getDisassembly,           METH_NOARGS,     ""},
        {"getFirstOperand",           Instruction_getFirstOperand,          METH_NOARGS,     ""},
        {"getNextAddress",            Instruction_getNextAddress,           METH_NOARGS,     ""},
        {"getOpcodes",                Instruction_getOpcodes,               METH_NOARGS,     ""},
        {"getOpcodesSize",            Instruction_getOpcodesSize,           METH_NOARGS,     ""},
        {"getOperands",               Instruction_getOperands,              METH_NOARGS,     ""},
        {"getSecondOperand",          Instruction_getSecondOperand,         METH_NOARGS,     ""},
        {"getSymbolicExpressions",    Instruction_getSymbolicExpressions,   METH_NOARGS,     ""},
        {"getThirdOperand",           Instruction_getThirdOperand,          METH_NOARGS,     ""},
        {"getThreadId",               Instruction_getThreadId,              METH_NOARGS,     ""},
        {"getType",                   Instruction_getType,                  METH_NOARGS,     ""},
        {"isBranch",                  Instruction_isBranch,                 METH_NOARGS,     ""},
        {"isConditionTaken",          Instruction_isConditionTaken,         METH_NOARGS,     ""},
        {"isControlFlow",             Instruction_isControlFlow,            METH_NOARGS,     ""},
        {"isTainted",                 Instruction_isTainted,                METH_NOARGS,     ""},
        {"setAddress",                Instruction_setAddress,               METH_O,          ""},
        {"setOpcodes",                Instruction_setOpcodes,               METH_O,          ""},
        {"setThreadId",               Instruction_setThreadId,              METH_O,          ""},
        {"updateContext",             Instruction_updateContext,            METH_O,          ""},
        {nullptr,                     nullptr,                              0,               nullptr}
      };


      PyTypeObject Instruction_Type = {
          PyObject_HEAD_INIT(&PyType_Type)
          0,                                          /* ob_size*/
          "Instruction",                              /* tp_name*/
          sizeof(Instruction_Object),                 /* tp_basicsize*/
          0,                                          /* tp_itemsize*/
          (destructor)Instruction_dealloc,            /* tp_dealloc*/
          (printfunc)Instruction_print,               /* tp_print*/
          0,                                          /* tp_getattr*/
          0,                                          /* tp_setattr*/
          0,                                          /* tp_compare*/
          0,                                          /* tp_repr*/
          0,                                          /* tp_as_number*/
          0,                                          /* tp_as_sequence*/
          0,                                          /* tp_as_mapping*/
          0,                                          /* tp_hash */
          0,                                          /* tp_call*/
          (reprfunc)Instruction_str,                  /* tp_str*/
          0,                                          /* tp_getattro*/
          0,                                          /* tp_setattro*/
          0,                                          /* tp_as_buffer*/
          Py_TPFLAGS_DEFAULT,                         /* tp_flags*/
          "Instruction objects",                      /* tp_doc */
          0,                                          /* tp_traverse */
          0,                                          /* tp_clear */
          0,                                          /* tp_richcompare */
          0,                                          /* tp_weaklistoffset */
          0,                                          /* tp_iter */
          0,                                          /* tp_iternext */
          Instruction_callbacks,                      /* tp_methods */
          0,                                          /* tp_members */
          0,                                          /* tp_getset */
          0,                                          /* tp_base */
          0,                                          /* tp_dict */
          0,                                          /* tp_descr_get */
          0,                                          /* tp_descr_set */
          0,                                          /* tp_dictoffset */
          0,                                          /* tp_init */
          0,                                          /* tp_alloc */
          0,                                          /* tp_new */
      };


      PyObject* PyInstruction(void) {
        Instruction_Object *object;

        PyType_Ready(&Instruction_Type);
        object = PyObject_NEW(Instruction_Object, &Instruction_Type);
        if (object != NULL)
          object->inst = new triton::arch::Instruction();

        return (PyObject* )object;
      }


      PyObject* PyInstruction(triton::arch::Instruction &inst) {
        Instruction_Object *object;

        PyType_Ready(&Instruction_Type);
        object = PyObject_NEW(Instruction_Object, &Instruction_Type);
        if (object != NULL)
          object->inst = new triton::arch::Instruction(inst);

        return (PyObject* )object;
      }

    }; /* python namespace */
  }; /* bindings namespace */
}; /* triton namespace */

#endif /* TRITON_PYTHON_BINDINGS */