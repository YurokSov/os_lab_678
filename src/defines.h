#ifndef INCLUDE_DEFINES_H
#define INCLUDE_DEFINES_H

typedef unsigned long long QWORD;
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef signed long long SQWORD;
typedef signed int SDWORD;
typedef signed short SWORD;
typedef signed char SBYTE;

#define STRBUF_LEN 256
#define NEW_STRING malloc(STRBUF_LEN * sizeof(char))

#define BAD_ALLOC 1

#endif