#ifndef clox_compile_h
#define clox_compile_h

#include <stdbool.h>
#include "chunk.h"
#include "object.h"

bool compile(const char* source, Chunk* chunk);

static void statement();
static void declaration();

#endif
