#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "memory.h"
#include "vm.h"

VM vm; // We only need one VM so its easier to pass it around

static InterpretResult run();
static void runTimeError(const char*);
bool isFalsey(Value value);
bool valuesEqual(Value a, Value b);

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
  vm.objects = NULL;
  initTable(&vm.strings);
  initTable(&vm.globals);
}

void freeVM(){
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
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
#define READ_STRING() AS_STRING(READ_CONSTANT())

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
        return INTERPRET_OK;
      }
      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        break;
      }
      case OP_POP: {
        pop();
        break;
      }
      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        tableSet(&vm.globals, name, peek(0));
        pop();
        break;
      }
      case OP_NEGATE: {
        if(IS_NUMBER(peek(0))){
          push(NUMBER_VAL(AS_NUMBER(pop())*-1));
        } 
        else {
          runTimeError("Unable to negate");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_ADD:{
          if(IS_STRING(peek(0)) && IS_STRING(peek(1))){
            concatenate();
          } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
            BINARY_OP(NUMBER_VAL, +);
          }
          else {
            runTimeError(
              "Operands must be two numbers or strings\n"
            );
            return INTERPRET_RUNTIME_ERROR;
          }
          break;
      }
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
      case OP_NOT: {
        Value value = pop();
        push(BOOL_VAL(isFalsey(value)));
        break;
       }
      case OP_EQUAL: {
        Value a = pop();
        Value b = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
        break;
       }
      case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
      case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
    }
  }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

void concatenate(){
  ObjString* bString = AS_STRING(pop());
  ObjString* aString = AS_STRING(pop());

  int length = aString->length + bString->length;


  char* chars = ALLOCATE(char, length+1);
  memcpy(chars, aString->chars, aString->length);
  memcpy(chars + aString->length, bString->chars, bString->length);

  chars[length] = '\0';

  ObjString* result = takeString(chars, length);

  push(OBJ_VAL(result));
}

bool isFalsey(Value value){
  return IS_BOOL(value) && !AS_BOOL(value) || IS_NIL(value);
}

bool valuesEqual(Value a, Value b){
  if(a.type != b.type) return false;
  switch(a.type){
    case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_NIL: return true;
    case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
    default: return false;
  }
}

