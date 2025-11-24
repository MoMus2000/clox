#ifndef clox_object_h

#define clox_object_h

#include <stdbool.h>
#include "common.h"
#include "value.h"

#define OBJ_TYPE(obj) (AS_OBJ(obj)->type)

typedef enum{
  OBJ_STRING
} ObjType;

struct Obj {
  ObjType type;
  struct Obj* next;
};

/*
Because ObjString is an Obj, it also needs the state all Objs share. 
It accomplishes that by having its first field be an Obj. 
C specifies that struct fields are arranged in memory in the order that they are declared. 
Also, when you nest structs, the inner struct’s fields are expanded right in place. 
So the memory for Obj and for ObjString looks like this:

Object:
--Obj--
[000000]

ObjectString:
--Obj--  -length-   --chars--
[000000][00000000][00000000000]

Note how the first bytes of ObjString exactly line up with Obj. 
This is not a coincidence—C mandates it. This is designed to enable a clever pattern: 
You can take a pointer to a struct and safely convert it to a pointer 
to its first field and back.

Given an ObjString*, you can safely cast it to Obj* and then access the type field 
from it. Every ObjString “is” an Obj in the OOP sense of “is”. When we later add other 
object types, each struct will have an Obj as its first field. Any code that wants to 
work with all objects can treat them as base Obj* and ignore any other fields that 
may happen to follow.
*/

struct ObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
};

#define IS_STRING(value) (isObjType(value, OBJ_STRING))
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);

static inline bool isObjType(Value value, ObjType type){
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif

