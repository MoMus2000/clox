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

### Pratt Parsing

Different types of expressions:

- Number Literals: 123
- Parenthesis for Grouping: (123)
- Unary Negation: -123
- Arithmetic Operations: + - / *

#### Parsing Number Literals

In short we *map* each *TokenType* for an expression.

We define a *function* for each *expression*.

We then build an *array / table of function pointers*.

The *index of the array* is the *TokenType*.

The *function at each index* is the code to *compile an expression*
of that type.

