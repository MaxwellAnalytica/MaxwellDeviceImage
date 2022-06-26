#pragma once

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>

#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#endif

/*
#define _O_RDONLY      0x0000  // open for reading only
#define _O_WRONLY      0x0001  // open for writing only
#define _O_RDWR        0x0002  // open for reading and writing
#define _O_APPEND      0x0008  // writes done at eof

#define _O_CREAT       0x0100  // create and open file
#define _O_TRUNC       0x0200  // open and truncate
#define _O_EXCL        0x0400  // open only if file doesn't already exist

// O_TEXT files have <cr><lf> sequences translated to <lf> on read()'s and <lf>
// sequences translated to <cr><lf> on write()'s

#define _O_TEXT        0x4000  // file mode is text (translated)
#define _O_BINARY      0x8000  // file mode is binary (untranslated)
*/
typedef enum _IOFlag
{
#ifdef _WIN32
	WD_RDONLY = _O_RDONLY,
	WD_WRONLY = _O_WRONLY,
	WD_RDWR = _O_RDWR,
	WD_APPEND = _O_APPEND,
	WD_CREAT = _O_CREAT,
	WD_TRUNC = _O_TRUNC,
	WD_BINARY = _O_BINARY
#else
	WD_RDONLY = O_RDONLY,
	WD_WRONLY = O_WRONLY,
	WD_RDWR = O_RDWR,
	WD_APPEND = O_APPEND,
	WD_CREAT = O_CREAT,
	WD_TRUNC = O_TRUNC,
	WD_BINARY = O_BINARY
#endif
}IOFlag;

#ifdef _WIN32

#ifndef OPEN
#define OPEN(filename, flag, mode) _open(filename, flag, mode) 
#endif // OPEN

#ifndef SEEK
#define SEEK(fd, offset, origin) _lseeki64(fd, offset, origin)
#endif // !SEEK

#ifndef TELL
#define TELL(fd) _telli64(fd)
#endif // !TELL

#ifndef READ
#define READ(fd, buffer, count) _read(fd, buffer, count)
#endif // !READ

#ifndef WRITE
#define WRITE(fd, buffer, count) _write(fd, buffer, count)
#endif // !WRITE

#ifndef CLOSE
#define CLOSE(fd) _close(fd)
#endif // !CLOSE

#else
#ifndef OPEN
#define OPEN(filename, flag, mode) _open(filename, flag) 
#endif // OPEN

#ifndef SEEK
#define SEEK(fd, offset, origin) lseek(fd, offset, origin)
#endif // !SEEK

#ifndef TELL
#define TELL(fd) lseek(fd, 0, SEEK_CUR)
#endif // !TELL

#ifndef READ
#define READ(fd, buffer, count) read(fd, buffer, count)
#endif // !READ

#ifndef WRITE
#define WRITE(fd, buffer, count) write(fd, buffer, count)
#endif // !WRITE

#ifndef CLOSE
#define CLOSE(fd) close(fd)
#endif // !CLOSE

#endif // _WIN32

