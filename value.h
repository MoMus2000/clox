#ifndef clox_value_h
#define clox_value_h

typedef double Value;

typedef struct{
  int capacity;
  int count;
  Value* values;
} ValueArray;

void initValueArray(ValueArray* valueArray);
void writeValueArray(ValueArray* valueArray, Value value);
void freeValueArray(ValueArray* valueArray);

#endif

