#ifndef clox_debug_h

#define clox_debug_h

#include "chunk.h"

voidDisassembleChunk(Chunk* chunk, const char* name);
voidDisassembleInstruction(Chunk* chunk, int offset);

#endif
