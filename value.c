#include <stdio.h>
#include "memory.h"
#include "value.h"


void initValueArray(ValueArray* valueArray){
  valueArray->count = 0;
  valueArray->capacity = 0;
  valueArray->values = NULL;
}

void writeValueArray(ValueArray* valueArray, Value value){
  if(valueArray->capacity < valueArray->count + 1){
    int oldCapacity = valueArray->capacity;
    valueArray->capacity = GROW_CAPACITY(oldCapacity);
    valueArray->values = GROW_ARRAY(Value, valueArray->values, 
        oldCapacity, valueArray->capacity);
  }

  valueArray->values[valueArray->count] = value;
  valueArray->count ++;
}

void freeValueArray(ValueArray* valueArray){
 FREE_ARRAY(Value, valueArray->values, valueArray->capacity);
 initValueArray(valueArray);
}

