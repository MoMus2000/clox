#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"

VM vm; // We only need one VM so its easier to pass it around

static InterpretResult run();
static void runTimeError(const char*);

void push(Value value){
  *vm.stackTop = value;
  vm.stackTop++; // move the stack pointer
}

static void runTimeError(const char* message){
  fprintf(stderr, "%s", message);
}

Value peek(int distance){
  return vm.stackTop[-1 - distance];
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

InterpretResult interpret(const char* source){
  Chunk chunk;
  initChunk(&chunk);

  if(!compile(source, &chunk)){
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  
  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}

static InterpretResult run(){
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(valueType, op) \
  do { \
    if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runTimeError("Operands must be numbers"); \
        return INTERPRET_RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
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
        if(IS_NUMBER(peek(0))){
          push(NUMBER_VAL(AS_NUMBER(pop())*-1));
        } 
        else if(IS_BOOL(peek(0))){
          push(BOOL_VAL(!AS_BOOL(pop())));
        }
        else {
          runTimeError("Unable to negate");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_ADD: BINARY_OP(NUMBER_VAL, +); break;
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_FALSE: push(BOOL_VAL(false)); break;
      case OP_TRUE: push(BOOL_VAL(true)); break;
      case OP_NIL: push(NIL_VAL); break;
    }
  }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

