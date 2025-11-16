#ifndef clox_value_h
#define clox_value_h

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
  VAL_BOOL,
  VAL_NUMBER,
  VAL_NIL,
  VAL_OBJ
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool   boolean;
    double number;
    Obj*   obj;
  } as; // to keep the same name
} Value;

#define BOOL_VAL(value) ((Value){VAL_BOOL, .as={.boolean=value}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, .as={.number=value}})
#define NIL_VAL ((Value){VAL_NIL, .as={.number=0}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, .as={.obj=(Obj*)object}})

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

typedef struct{
  int capacity;
  int count;
  Value* values;
} ValueArray;

void initValueArray(ValueArray* valueArray);
void writeValueArray(ValueArray* valueArray, Value value);
void freeValueArray(ValueArray* valueArray);

#endif

