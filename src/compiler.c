#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chunk.h"
#include "scanner.h"
#include "compiler.h"

#ifdef DEBUG_IMPLEMENTATION
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

static void binary(bool canAssign);
static void unary(bool canAssign);
static void number(bool canAssign);
static void literal(bool canAssign);
static void handle_string(bool canAssign);
static void grouping(bool canAssign);
static void variable(bool canAssign);

typedef void (*ParseFn)(bool canAssign);

typedef struct{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
}ParseRule;
static ParseRule* getRule(TokenType t);

void emitBytes(uint8_t byte1, uint8_t byte2){
  emitByte(byte1);
  emitByte(byte2);
}

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
  [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
  [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
  [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISION},
  [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISION},
  [TOKEN_LESS] = {NULL, binary, PREC_COMPARISION},
  [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISION},
  [TOKEN_STRING] = {handle_string, binary, PREC_COMPARISION},
  [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
  [TOKEN_EOF] = {NULL, NULL, PREC_NONE}
};

static ParseRule* getRule(TokenType t){
  return &rules[t];
}

Chunk* currentChunk(){
  return compilingChunk;
}

static bool check(TokenType type) {
  return parser.current.type == type;
}

static bool match(TokenType type){
  if(!check(type)) return false;
  advance();
  return true;
}


static void printStatement(){
  expression();
  consume(TOKEN_SEMICOLON, "Expect ; after value.");
  emitByte(OP_PRINT);
}

static void expressionStatement(){
  expression();
  consume(TOKEN_SEMICOLON, "Expect ; after value.");
  emitByte(OP_POP);
}

static void statement(){
  if(match(TOKEN_PRINT)){
    printStatement();
  }
  else {
    expressionStatement();
  }
}

static void synchronize() {
  parser.panicMode = false;
  while(parser.current.type != TOKEN_EOF){
    if(parser.previous.type == TOKEN_SEMICOLON) return;
    switch(parser.current.type){
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default:
        ; // Do nothing
    }
    advance();
  }
}

static uint32_t identifierConstant(Token* name){
  return addConstant(
      currentChunk(), 
      OBJ_VAL(
        copyString(
          name->start, name->length  
        )
      )
  );
}

static void namedVariable(Token name, bool canAssign){
  uint8_t arg = identifierConstant(&name);
  if(canAssign && match(TOKEN_EQUAL)){
    expression();
    emitBytes(OP_SET_GLOBAL, arg);
  } else{
    emitBytes(OP_GET_GLOBAL, arg);
  }
}

static void variable(bool canAssign){
  namedVariable(parser.previous, canAssign);
}

static uint32_t parseVariable(const char* message) {
  consume(TOKEN_IDENTIFIER, message); 
  return identifierConstant(&parser.previous);
}

static void defineVariable(uint32_t global){
  emitBytes(OP_DEFINE_GLOBAL, global);
}

static void varDeclaration() {
  uint32_t global = parseVariable("Expect Variable Name.");
  if(match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ; after var decl.");
  defineVariable(global);
}

static void declaration() {
  if(match(TOKEN_VAR)){
    varDeclaration();
  } else {
    statement();
  }

  if(parser.panicMode){
    synchronize();
  }
}

bool compile(const char* source, Chunk* chunk){
  initScanner(source);
  compilingChunk = chunk;
  parser.hadError  = false;
  parser.panicMode = false;
  advance();
  while(!match(TOKEN_EOF)){
    declaration();
  }
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

static void handle_string(bool canAssign){
  copyString(parser.previous.start + 1, parser.previous.length -2);
  emitConstant(
      OBJ_VAL(
        // trim start and end quotation marks
        copyString(parser.previous.start + 1, parser.previous.length -2)
      )
  );
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


void endCompiler(){
  emitReturn();
#ifdef DEBUG_IMPLEMENTATION
  if(!parser.hadError){
     disassembleChunk(currentChunk(), "code");
  }
#endif
}

void emitReturn(){
  writeChunk(currentChunk(), OP_RETURN, parser.previous.line);
}

static void literal(bool canAssign) {
  TokenType type = parser.previous.type;
  switch(type){
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    default: { return; }
  }
}

static void number(bool canAssign){
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

static void unary(bool canAssign){
  TokenType operatorType = parser.previous.type;

  parsePrecedence(PREC_UNARY); // parse unary or anything greater

  switch(operatorType){
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    case TOKEN_BANG: emitByte(OP_NOT); break;
    default: return;
  }
}

static void grouping(bool canAssign){
  // ( is already parsed
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void binary(bool canAssign){
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
    case TOKEN_EQUAL_EQUAL: {
        emitByte(OP_EQUAL); break;
    }
    case TOKEN_BANG_EQUAL: {
        emitBytes(OP_EQUAL, OP_NOT); break;
    }
    case TOKEN_GREATER: {
        emitByte(OP_GREATER); break;
    }
    case TOKEN_GREATER_EQUAL: {
        emitBytes(OP_LESS, OP_NOT); break;
    }
    case TOKEN_LESS_EQUAL: {
        emitBytes(OP_GREATER, OP_NOT); break;
    }
    case TOKEN_LESS: {
        emitByte(OP_LESS); break;
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

  bool canAssign = precedence <= PREC_ASSIGNMENT;

  prefixFn(canAssign);

  while(precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixFn = getRule(parser.previous.type)->infix;
    infixFn(canAssign);
  }

  if(canAssign && match(TOKEN_EQUAL)){
    error("Invalid Assignment Target");
  }

}

