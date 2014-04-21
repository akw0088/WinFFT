#ifndef INCLUDE_H
#define INCLUDE_H

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>

#include <math.h>
#include <process.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>

#define PIPE 1
#define NET 2
#define MIC 3


#define TIME	0x1
#define FREQ	0x2
#define BIN	0x4


#define MYPI  (3.141592)
#define TWOPI  (2 * 3.141592)


#define LENGTH		4096	//Buffer size in bytes
#define NUM    		2048	//Buffer size in short ints / bin's of FFT
#define	FFT_HEIGHT	1024

#define FFT_SCALE	0.5	// Scale length (ie: 0.5 means only look at first half)
#define FFT_OFFSET	0	// offset length

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define SAMPLE_RATE 32000


#define MARGIN 20


typedef struct { /* structure size must be multiple of 2 bytes */
	char magic[4];
	int tuner_type;
	int tuner_gain_count;
} dongle_info_t;


//#define CONSOLE

#endif