#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"
#include "scanner.h"

static bool isAtEnd();
Token makeToken(TokenType);
Token errorToken(const char*);
static char advance();
static bool match(char);
static void skipWhitespace();
static char peek();

typedef struct {
  const char* start; // begining
  const char* current; // lexeme being looked at
  int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source){
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

Token scanToken(){
  skipWhitespace();
  scanner.start = scanner.current;
  if(isAtEnd()) return makeToken(TOKEN_EOF);

  char c = advance();

  switch(c) {
    case '(' : return makeToken(TOKEN_LEFT_PAREN);
    case ')' : return makeToken(TOKEN_RIGHT_PAREN);
    case '{' : return makeToken(TOKEN_LEFT_BRACE);
    case '}' : return makeToken(TOKEN_RIGHT_BRACE);
    case ';' : return makeToken(TOKEN_SEMICOLON);
    case ',' : return makeToken(TOKEN_COMMA);
    case '.' : return makeToken(TOKEN_DOT);
    case '-' : return makeToken(TOKEN_MINUS);
    case '+' : return makeToken(TOKEN_PLUS);
    case '/' : return makeToken(TOKEN_SLASH);
    case '*' : return makeToken(TOKEN_STAR);
    case '!' : return makeToken(
      match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
    );
    case '<' : return makeToken(
      match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
    );
    case '>' : return makeToken(
      match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
    );
    case '=' : return makeToken(
      match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
    );
  }

  return errorToken("Unexpected Character.");
}

static bool isAtEnd() {
  return *scanner.current == '\0';
}

Token makeToken(TokenType type){
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

Token errorToken(const char* message){
  Token token;
  token.type  = TOKEN_ERROR;
  token.start = message;
  token.length = strlen(message);
  token.line = scanner.line;
  return token;
}

static char advance(){
  scanner.current ++;
  return scanner.current[-1];
}

static bool match(char expected){
  if(isAtEnd()) return false;
  if(*scanner.current != expected) return false;
  scanner.current ++;
  return true;
}

static char peek(){
  return *scanner.current;
}

static char peekNext(){
  if(isAtEnd()) return '\0';
  return *(scanner.current + 1);
}

static void skipWhitespace(){
  for(;;){
    char c = peek();
    switch(c){
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
      case '\n':
        scanner.line++;
        advance();
        break;
      case '/':
        if(peekNext() == '/'){
          while(peek() != '\0' && !isAtEnd()) advance();
        } else{
          return;
        }
      default:
        return;
    }
  }
}

