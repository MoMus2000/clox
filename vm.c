#include <stdio.h>
#include "debug.h"
#include "vm.h"

VM vm; // We only need one VM so its easier to pass it around

static InterpretResult run();

void push(Value value){
  *vm.stackTop = value;
  vm.stackTop++; // move the stack pointer
}

Value pop(){
  vm.stackTop --; // Moving is enough, we don't have to remove
  return *vm.stackTop;
}

static void resetStack(){
  vm.stackTop = vm.stack; // point stack pointer back to the start
}

void initVM(){
  vm.chunk = NULL;
  vm.ip = 0;
  resetStack();
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

#define BINARY_OP(op) \
  do { \
    Value b = pop(); \
    Value a = pop(); \
    push(a op b); \
  } while(false)

  for(;;){
#ifdef DEBUG_TRACE_EXECUTION
printf("          ");
//for(Value* slot = vm.stack; slot < vm.stackTop; slot ++){
// less fancy
for (int i = 0; i < (vm.stackTop - vm.stack); i++) {
  printf("[ ");
  printValue(vm.stack[i]);
  printf(" ]");
}
printf("\n");
// Pointer arithmetic to get offset from the start of the opcode
disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    switch(instruction = READ_BYTE()){
      case OP_RETURN: {
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
      }
      case OP_NEGATE: {
        push(pop()*-1);
        break;
      }
      case OP_ADD: BINARY_OP(+); break;
      case OP_SUBTRACT: BINARY_OP(-); break;
      case OP_MULTIPLY: BINARY_OP(*); break;
      case OP_DIVIDE: BINARY_OP(/); break;
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
    }
  }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

