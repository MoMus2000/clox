#include <stdio.h>
#include "debug.h"
#include "vm.h"

VM vm; // We only need one VM so its easier to pass it around

static InterpretResult run();

void initVM(){
  vm.chunk = NULL;
  vm.ip = 0;
}

void freeVM(){
}

InterpretResult interpret(Chunk* chunk){
  vm.chunk = chunk;
  vm.ip = vm.chunk->code; // opcodes for the chunk
  return run();
}

static InterpretResult run(){
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
  for(;;){
#ifdef DEBUG_TRACE_EXECUTION
// Pointer arithmetic to get offset from the start of the opcode
disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    switch(instruction = READ_BYTE()){
      case OP_RETURN: {
        return INTERPRET_OK;
      }
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        printValue(constant);
        printf("\n");
        break;
      }
    }
  }
#undef READ_BYTE
#undef READ_CONSTANT
}

