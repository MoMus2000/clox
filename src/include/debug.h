#ifndef clox_debug_h

#define clox_debug_h

#include "object.h"
#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);
void printValue(Value value);

#endif
