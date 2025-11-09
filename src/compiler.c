#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chunk.h"
#include "scanner.h"
#include "compiler.h"

void static advance();
void static errorAtCurrent(const char*);
void consume(TokenType, const char*);
void error(Token, const char*);
void errorAt(Token*, const char*);
static void expression();
void emitByte(uint8_t byte);
void endCompiler();
void emitReturn();
void emitConstant(Value value);

typedef struct {
  Token previous;
  Token current;
  bool  hadError;
  bool  panicMode;
} Parser;

static Chunk* compilingChunk;

Parser parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR, // or
  PREC_AND, // and
  PREC_EQUALITY, // ==
  PREC_COMPARISION, // < > <= >=
  PREC_TERM, // + -
  PREC_FACTOR, // * /
  PREC_UNARY, // ! -
  PREC_CALL, // ()
  PREC_PRIMARY,
} Precidence;

static void parsePrecidence(Precidence);


Chunk* currentChunk(){
  return compilingChunk;
}

bool compile(const char* source, Chunk* chunk){
  initScanner(source);
  compilingChunk = chunk;
  parser.hadError  = false;
  parser.panicMode = false;
  advance();
  expression();
  consume(TOKEN_EOF, "Expected EOF Expression");
  endCompiler();
  return !parser.hadError;
}

void static advance(){
  parser.previous = parser.current;
  for(;;){
    Token token = scanToken();
    parser.current = token;
    if(parser.current.type != TOKEN_ERROR) break;
    errorAtCurrent(parser.current.start);
  }
}

void static errorAtCurrent(const char* message){
  errorAt(&parser.current, message);
}

void consume(TokenType type, const char* message){
  if(parser.current.type == type){
    advance();
    return;
  }
  errorAtCurrent(message);
}

static void expression(){
  parsePrecidence(PREC_ASSIGNMENT);
}

void errorAt(Token* token, const char* message){
  if(parser.panicMode) return;
  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);
  if(token->type == TOKEN_EOF){
    fprintf(stderr, " at end");
  }
  else if(token->type == TOKEN_ERROR){
    //
  }
  else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }
  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

void emitByte(uint8_t byte){
  writeChunk(currentChunk(), byte, parser.previous.line);
}

void emitBytes(uint8_t byte1, uint8_t byte2){
  emitByte(byte1);
  emitByte(byte2);
}

void endCompiler(){
  emitReturn();
}

void emitReturn(){
  writeChunk(currentChunk(), OP_RETURN, parser.previous.line);
}

static void number(){
  double value = strtod(parser.previous.start, NULL);
  emitConstant(value);
}

void emitConstant(Value value){
  int index = addConstant(
      currentChunk(),
      value
  );
  emitBytes(OP_CONSTANT, (uint8_t)index);
}

static void unary(){
  TokenType operatorType = parser.previous.type;

  parsePrecidence(PREC_UNARY);

  switch(operatorType){
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return;
  }
}

static void grouping(){
  // ( is already parsed
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void parsePrecidence(){

}

