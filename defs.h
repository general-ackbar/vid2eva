#include <stddef.h>
#include <stdlib.h>
#include <cstring>




#define FALSE false
#define TRUE true

#define ZeroMemory(p, sz) memset((p), 0, (sz))
#define CopyMemory(Destination, Source, Length) memcpy((Destination),(Source),(Length))



#define GENERIC_WRITE                    0x40000000
#define CREATE_ALWAYS                    2
#define FILE_ATTRIBUTE_NORMAL            0x00000080

#define INVALID_HANDLE_VALUE -1


typedef unsigned char BYTE;
typedef unsigned char* LPBYTE;
typedef void* LPVOID;
typedef void* PVOID;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef bool BOOL;
typedef unsigned long ULONG_PTR;
typedef unsigned long* SIZE_T;
typedef void* HANDLE;
typedef unsigned long DWORD;



/*
#include <stddef.h>
#include <stdlib.h>
#include <cstring>



#define FALSE false
#define TRUE true


#define ZeroMemory(p, sz) memset((p), 0, (sz))
#define CopyMemory(Destination, Source, Length) memcpy((Destination),(Source),(Length))

#define CreateFile(path, access, x, y, mode, attr, z) open(path, O_CREAT|O_RDWR)
#define CloseHandle(fd) close(fd)
#define WriteFile(fd, buffer, size, written, x) write(fd, buffer, size)
#define ReadFile(fd, buffer, size, written, x) read(fd, buffer, size)
#define SetFilePointer(fd, location, x, offset) lseek(fd, location, SEEK_SET)

#define GENERIC_WRITE                    0x40000000
#define CREATE_ALWAYS                    2
#define FILE_ATTRIBUTE_NORMAL            0x00000080
#define FILE_BEGIN                       0
#define _MAX_PATH                        1024
#define INVALID_HANDLE_VALUE -1


typedef unsigned char BYTE;
typedef unsigned char* LPBYTE;
typedef void* LPVOID;
typedef void* PVOID;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef bool BOOL;
typedef unsigned long ULONG_PTR;
typedef unsigned long* SIZE_T;
//typedef void* HANDLE;
typedef int HANDLE;
typedef unsigned long DWORD;

*/