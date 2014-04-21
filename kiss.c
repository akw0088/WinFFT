#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kiss_fft.h"

#define SIZE 4096



// only malloc once
kiss_fft_cpx *mat2;
kiss_fft_cfg fft;

extern float blackman[2048];
extern int blackman_flag;

kiss_fft_cpx *copycpx(const short int *data, int nframe)
{
	int i;
        kiss_fft_scalar zero;
	static int once = 0;

	if (once == 0)
	{
		mat2 = (kiss_fft_cpx*)KISS_FFT_MALLOC(sizeof(kiss_fft_cpx) * nframe * 2);
		once = 1;
	}

        memset(&zero, 0, sizeof(zero) );
	for(i = 0; i < nframe; i++)
	{
		mat2[i].r = 1.0 - (data[i] / 32768.0); //convert PCM to [1,-1] float
		mat2[i].i = zero;

		if (blackman_flag)
			mat2[i].r *= blackman[i];
	}
	return mat2;
}

kiss_fft_cpx *copycpx_net(const char *data, int nframe)
{
	int i, j = 0;
        kiss_fft_scalar zero;
	static int once = 0;

	if (once == 0)
	{
		mat2 = (kiss_fft_cpx*)KISS_FFT_MALLOC(sizeof(kiss_fft_cpx) * nframe * 2);
		once = 1;
	}

        memset(&zero, 0, sizeof(zero) );
	for(i = 0; i < nframe; i++)
	{
		mat2[i].r = data[j++];
		mat2[i].i = zero;
		j++;
		//mat2[i].i = data[j++];

		if (blackman_flag)
		{
			mat2[i].r *= blackman[i];
			//mat2[i].i *= blackman[i];
		}
	}
	return mat2;
}

int fft_psd(const short int *data, int size, float *fft_output, float *max, float *min, float *avg)
{
	kiss_fft_cpx out_cpx[SIZE * 2];
	kiss_fft_cpx *cpx_buf;
	static int once = 0;

	int isinverse = 1;
	int i;


	if (once == 0)
	{
		fft = kiss_fft_alloc(size * 2 ,0 ,0,0);
		once = 1;
	}

	cpx_buf = copycpx(data, size);
	kiss_fft(fft, (kiss_fft_scalar*)cpx_buf, out_cpx);
	*avg = 0;
	for(i = 0; i < size; i++)
	{
		// power spectral = output * conjugate of output
		fft_output[i] = (out_cpx[i].r * out_cpx[i].r + out_cpx[i].i * out_cpx[i].i) / size;
		if (fft_output[i] > *max)
			*max = fft_output[i];

		if (fft_output[i] < *min)
			*min = fft_output[i];

		*avg += fft_output[i];

	}
	*avg /= size;


	kiss_fft_cleanup();
//	free(fft);
	return 0;
}

int fft_psd_net(const char *data, int size, float *fft_output, float *max, float *min, float *avg)
{
	kiss_fft_cpx out_cpx[SIZE * 2];
	kiss_fft_cpx *cpx_buf;
	static int once = 0;

	int isinverse = 1;
	int i;


	if (once == 0)
	{
		fft = kiss_fft_alloc(size * 2 ,0 ,0,0);
		once = 1;
	}

	cpx_buf = copycpx_net(data, size);
	kiss_fft(fft, (kiss_fft_scalar*)cpx_buf, out_cpx);
	*avg = 0;
	for(i = 0; i < size; i++)
	{
		// power spectral = output * conjugate of output
		fft_output[i] = (out_cpx[i].r * out_cpx[i].r + out_cpx[i].i * out_cpx[i].i) / size;
		if (fft_output[i] > *max)
			*max = fft_output[i];

		if (fft_output[i] < *min)
			*min = fft_output[i];

		*avg += fft_output[i];

	}
	*avg /= size;


	kiss_fft_cleanup();
//	free(fft);
	return 0;
}


#define BUFFER_COUNT 30
#include "low_pass.h"


/*
	This code was pulled from a program I wrote for a c64x dsp and is kinda ugly, but works

	Does 30 samples at a time using a low pass filter that also happens to have a size of 30
	just zero pad input if you have less than 30 samples
*/
void fir_filter(float *pBuf)
{
	static float Sample[BUFFER_COUNT];
	float *pSample = Sample;
	int i, j;
	float temp;


	for(i = 0; i < BUFFER_COUNT; i++)
	{
		*pSample++ = *pBuf++;
	}

	pSample = Sample;

	//Filter sample
	for(i = 0; i < BUFFER_COUNT; i++)
	{
		temp = 0.0f;
		for(j = 0; j < 30; j++)
		{
			 if ( i + j < BUFFER_COUNT)
			 {
				 temp += B_lowpass[j] * (pSample[i + j]);
			 }
			 *pSample = temp;
			 *pSample++;
		}
	}
  
	pSample = Sample;

	for(i = 0;i < BUFFER_COUNT;i++)
	{
		*pBuf++ = *pSample++;
	}

}

void pcm_to_float(short int *data, float *fdata)
{
	int i = 0;

	for(i = 0; i < 30; i++)
	{
		fdata[i] = (1.0 - data[i] / 32767.0);
	}
}

void float_to_pcm(short int *data, float *fdata)
{
	int i = 0;

	for(i = 0; i < 30; i++)
	{
		data[i] = 32767.0 * (fdata[i] + 0.5);
	}
}

