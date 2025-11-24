#ifndef clox_vm_h

#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  Value stack[STACK_MAX];
  Value* stackTop; // points to where the next item will go
  Obj* objects;
  Table strings;
  Table globals;
}VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void push(Value value);
Value pop();

void initVM();
void freeVM();

InterpretResult interpret(const char* source);

void concatenate();

#endif
