#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, char** argv){
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, OP_RETURN);
  disassembleChunk(&chunk, "My Chunk");
  freeChunk(&chunk);
  return 0;
}
