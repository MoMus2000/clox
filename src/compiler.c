#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chunk.h"
#include "compiler.h"
#include "scanner.h"

void static advance();
void static errorAtCurrent(const char*);
void consume(TokenType, const char*);
void error(Token, const char*);
void errorAt(Token*, const char*);
void expression();

typedef struct {
  Token previous;
  Token current;
  bool  hadError;
  bool  panicMode;
} Parser;

Parser parser;

bool compile(const char* source, Chunk* chunk){
  initScanner(source);
  parser.hadError  = false;
  parser.panicMode = false;
  advance();
  expression();
  consume(TOKEN_EOF, "Expected EOF Expression");
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

void expression(){
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

