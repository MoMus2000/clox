#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chunk.h"
#include "scanner.h"
#include "compiler.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

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
} Precedence;

void static advance();
void static errorAtCurrent(const char*);
void consume(TokenType, const char*);
static void error(const char*);
void errorAt(Token*, const char*);
static void expression();
void emitByte(uint8_t byte);
void endCompiler();
void emitReturn();
void emitConstant(Value value);
static void binary();
static void unary();
static void number();
static void literal();
static void grouping();
typedef void (*ParseFn)();
typedef struct{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
}ParseRule;
static ParseRule* getRule(TokenType t);


typedef struct {
  Token previous;
  Token current;
  bool  hadError;
  bool  panicMode;
} Parser;

static Chunk* compilingChunk;

Parser parser;


static void parsePrecedence(Precedence);


ParseRule rules[] = {
  [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_NONE},
  [TOKEN_MINUS] = {unary, binary, PREC_TERM},
  [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
  [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
  [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
  [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
  [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
  [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
  [TOKEN_NIL] = {literal, NULL, PREC_NONE},
  [TOKEN_BANG] = {unary, NULL, PREC_NONE},
  [TOKEN_EOF] = {NULL, NULL, PREC_NONE}
};

static ParseRule* getRule(TokenType t){
  return &rules[t];
}

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
  parsePrecedence(PREC_ASSIGNMENT);
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
#ifdef DEBUG_PRINT_CODE
  if(!parser.hadError){
     disassembleChunk(currentChunk(), "code");
  }
#endif
}

void emitReturn(){
  writeChunk(currentChunk(), OP_RETURN, parser.previous.line);
}

static void literal() {
  TokenType type = parser.previous.type;
  switch(type){
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    default: { return; }
  }
}

static void number(){
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
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

  parsePrecedence(PREC_UNARY); // parse unary or anything greater

  switch(operatorType){
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    case TOKEN_BANG: emitByte(OP_NEGATE); break;
    default: return;
  }
}

static void grouping(){
  // ( is already parsed
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void binary(){
  TokenType operator = parser.previous.type;
  ParseRule* rule = getRule(operator);
  parsePrecedence((Precedence)(rule->precedence+1)); // beyond the current prec

  switch(operator){
    case TOKEN_PLUS: {
        emitByte(OP_ADD); break;
     }
    case TOKEN_MINUS: {
        emitByte(OP_SUBTRACT); break;
     }
    case TOKEN_STAR: {
        emitByte(OP_MULTIPLY); break;
     }
    case TOKEN_SLASH: {
        emitByte(OP_DIVIDE); break;
     }
    default: return;
  }
}

static void error(const char* message) {
  errorAt(&parser.previous, message);
}

static void parsePrecedence(Precedence precedence){
  advance();
  ParseFn prefixFn = getRule(parser.previous.type)->prefix;
  if(prefixFn == NULL){
    error("Expected an Expression");
    return;
  }

  prefixFn();

  while(precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixFn = getRule(parser.previous.type)->infix;
    infixFn();
  }

}

