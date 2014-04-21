/* low_pass.c                          */
/* FIR filter coefficients              */
/* exported by MATLAB using FIR_dump2c  */
/* Michael G. Morrow - 2000, 2003       */


#include "low_pass.h"

float B_lowpass[N+1] = {
-0.003998796282,	/* h[0] */
-0.017420428151,	/* h[1] */
-0.021169400426,	/* h[2] */
-0.010681830754,	/* h[3] */
0.009698785802,	/* h[4] */
0.027862117263,	/* h[5] */
0.030436279355,	/* h[6] */
0.011443784823,	/* h[7] */
-0.021947384019,	/* h[8] */
-0.050599444477,	/* h[9] */
-0.051927348572,	/* h[10] */
-0.011918123560,	/* h[11] */
0.065671538515,	/* h[12] */
0.158325505398,	/* h[13] */
0.233319532982,	/* h[14] */
0.262079156347,	/* h[15] */
0.233319532982,	/* h[16] */
0.158325505398,	/* h[17] */
0.065671538515,	/* h[18] */
-0.011918123560,	/* h[19] */
-0.051927348572,	/* h[20] */
-0.050599444477,	/* h[21] */
-0.021947384019,	/* h[22] */
0.011443784823,	/* h[23] */
0.030436279355,	/* h[24] */
0.027862117263,	/* h[25] */
0.009698785802,	/* h[26] */
-0.010681830754,	/* h[27] */
-0.021169400426,	/* h[28] */
-0.017420428151,	/* h[29] */
-0.003998796282,	/* h[30] */
};
