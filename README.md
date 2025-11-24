# clox
1 millionth take on writing a bytecode VM


## Instructions


### To Build
```bash
make
```

### To Run
```bash
make run
```

## Pratt Parsing

Different types of expressions:

- Number Literals: 123
- Parenthesis for Grouping: (123)
- Unary Negation: -123
- Arithmetic Operations: + - / *

### Parsing Number Literals

In short we **map** each **TokenType** for an expression.

We define a **function** for each **expression**.

We then build an **array / table of function pointers**.

The **index of the array** is the **TokenType**.

The **function at each index** is the code to **compile an expression**
of that type.

### Example Functions to compile expressions


`Unary`
```c
static void unary(){
  TokenType operatorType = parser.previous.type;

  parsePrecidence(PREC_UNARY);

  switch(operatorType){
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return;
  }
}
```

`Grouping`
```c
static void grouping(){
  // ( is already parsed
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}
```

### Precendence

```c
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

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};
```

To define precidence you make a enum as such, the order of
precidence is defined as increasing downwards. **PREC_NONE**
having the lowest precidence while **PREC_CALL** having the
highest precidence.

#### Why do we need precidence ?

```python3
-a.b + c;
```

Consider this expression. If precidence were not defined, the language would calculate a.b +c before performing the negation, which is the wrong order of operation. When parsing unary, we need to parse expressions
at a certain precedence or higher.

So if we were to define **parsePresidence(PREC_ASSIGN)** then the entire
expression would be parsed since **PREC_ASSIGN** is less than 
**PREC_UNARY**. However if **parsePresidence(PREC_UNARY)** were
defined then we would stop at `-a.b` since `+` has less precidence
than unary.

### Connection

![Mapping](./img/connections.png)

### Whats happening ??

- *expression(PREC_ASSIGNMENT)* kicks off the parser.
- Depending on what sort of token gets encountered relevant func from the table gets triggered.
- getRule() is being used to get the precedence.
- parsePrecedence() is doing the heavy lifting and determining what to parse.
- If the token starts an expression (like a number, (, or -), its prefix function runs.
- If it follows an expression (like +, \*, or ==), its infix function runs.

### Example - Parse 1 + 2 * 3

Step by Step

```python3
parsePrecedence(precedence):
    advance()
    prefixRule = getRule(previous.type).prefix
    prefixRule()

    while precedence <= getRule(current.type).precedence:
        advance()
        infixRule = getRule(previous.type).infix
        infixRule()
```

`Token Steam`

```bash
[NUMBER(1)] [+] [NUMBER(2)] [*] [NUMBER(3)] [EOF]
```

`Steps`

You first enter via expression(*PREC_ASSIGNMENT*).

You advance the token such that 1 is parser.previous.

Since 1 is a number you getback number as the prefixRule().

You emit bytecode for 1.

Now the precidence given was *PREC_ASSIGNMENT*.

So *PREC_ASSIGNMENT* is less than *PREC_TERM* (+).

Hence the while loops holds true.

You advance such that + is the previous and the current is
now 2.

The infix rule is now binary based on the PLUS token.

`The implementation of binary is as follows`
```c
static void binary() {
  TokenType operatorType = parser.previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (operatorType) {
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    default: return; // Unreachable.
  }
}
```

You get the operator which is +.

You get the rule based on + which is binary.

You parse again but with precidence (+ 1) of '+'.

Why ? You are saying parse everything higher than '+'.
You’re telling the parser: “Keep reading anything that should happen before +, but stop if you see another +.”
That makes sure the + operators are handled one after another from left to right, just like in normal math.

That way you handle the right hand side, emit the byte code

Then end up at the operatorType for that to be processed.

You return back to the orignal parsePrecidence encounter an EOF,
which has less precidence than ASSIGNMENT and exit out.

Resulting bytecode is as follows

`1 2 3 * +`
`OP_CONSTANT index OP_CONSTANT index OP_CONSTANT index OP_MULTIPLY OP_ADD`

## Global Variables

### Example: var a = 5;

`OP_CONSTANT` pushes a value from the constant pool onto the stack.
`OP_DEFINE_GLOBAL` reads a variable name (string) from the constant pool and stores the top stack value in the global table under that name, then pops the value off the stack.

```
Bytecode:  
OP_CONSTANT (index for 5)  
OP_DEFINE_GLOBAL (index for "a")

Execution:

Stack before OP_CONSTANT:  
[ ]  (empty)

After OP_CONSTANT:  
[ 5 ]  ← value 5 pushed onto stack

Globals table before OP_DEFINE_GLOBAL:  
{ }

During OP_DEFINE_GLOBAL:  
- Reads "a" from constant pool  
- Sets globals["a"] = 5 (top of stack)  
- Pops 5 from stack

Stack after OP_DEFINE_GLOBAL:  
[ ]  (empty)

Globals table after OP_DEFINE_GLOBAL:  
{ "a" : 5 }
```

