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

