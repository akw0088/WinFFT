#include "include.h"

char data1[LENGTH];
char data2[LENGTH];
char data3[LENGTH];

int running = 1;


int init_rtltcp(char *ip, int port)
{
	dongle_info_t info;
	int sock;
	struct sockaddr_in      servaddr;
	int ret;
	int bytes_read;

	sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	// 3 way handshake
	printf("Attempting to connect to %s:%d\n", ip, port);
	ret = connect(sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));

	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();

		switch(ret)
		{
		case WSAETIMEDOUT:
			printf("Fatal Error: The network is unreachable from this host at this time.\n(Bad IP address)\n");
			break;
		case WSAECONNREFUSED:
			printf("Fatal Error: %s refused connection.\n(Server program is not running)\n", ip);
			break;
		case WSAEHOSTUNREACH:
			printf("Fatal Error: router sent ICMP packet (destination unreachable)\n");
			break;
		default:
			printf("Fatal Error: %d\n", ret);
			break;
		}

		return -1;
	}

	bytes_read = recv(sock, (char *)&info, sizeof(dongle_info_t), 0);
	if (bytes_read != sizeof(dongle_info_t))
	{
		printf("Unable to get dongle info\n");
		return -1;
	}

	printf("Got magic: %c%c%c%c\n", info.magic[0], info.magic[1], info.magic[2], info.magic[3]);

	return sock;
}

void play_wave(WAVEFORMATEX *format, char *data, int length)
{
	HWAVEOUT	hWaveOut;
	WAVEHDR		wavehdr = {0};

	wavehdr.lpData = data;
	wavehdr.dwBufferLength = length;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, format, 0, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));
	Sleep( 1000 * length / (format->nSamplesPerSec * format->nChannels * (format->wBitsPerSample / 8)) );
}


int ReadBuffer(int type, char *data, int length, FILE *fp, int sock, HWAVEIN hWaveIn, WAVEHDR *wavehdr, WAVEHDR *wavehdr_next)
{
	int bytes_read = 0;
	int ret;


	if (type == PIPE)
	{
		bytes_read = fread(data, 1, length, fp);
		printf("read %d bytes\n", bytes_read);
	}
	else if (type == NET)
	{
		memset(data1, 0, length);
		bytes_read = recv(sock, &data[0], length, 0);
	}
	else if (type == MIC)
	{
		if ( waveInAddBuffer(hWaveIn, wavehdr_next, length) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}

		while ( (wavehdr->dwFlags & WHDR_DONE) == 0)
			Sleep(0);
		bytes_read = LENGTH;
	}
	return bytes_read;
}

int stream_wave(int input_type)
{
	HWAVEOUT	hWaveOut;
	WAVEHDR		wavehdr1_out = {0};
	WAVEHDR		wavehdr2_out = {0};
	WAVEHDR		wavehdr3_out = {0};

	HWAVEIN	hWaveIn;
	WAVEHDR	wavehdr1_in = {0};
	WAVEHDR	wavehdr2_in = {0};
	WAVEHDR	wavehdr3_in = {0};


	WAVEFORMATEX	wformat;
	int started = 0;
	int bytes_read = 0;
	int sock = -1;

	wformat.wFormatTag = WAVE_FORMAT_PCM;
	wformat.nChannels = 1;
	wformat.nSamplesPerSec = SAMPLE_RATE;
	wformat.nAvgBytesPerSec = SAMPLE_RATE * 2;
	wformat.nBlockAlign = 2;
	wformat.wBitsPerSample = 16;
	wformat.cbSize = 0;

	wavehdr1_in.lpData = data1;
	wavehdr1_in.dwBufferLength = LENGTH;

	wavehdr2_in.lpData = data2;
	wavehdr2_in.dwBufferLength = LENGTH;

	wavehdr3_in.lpData = data3;
	wavehdr3_in.dwBufferLength = LENGTH;

	wavehdr1_out.lpData = data1;
	wavehdr1_out.dwBufferLength = LENGTH;

	wavehdr2_out.lpData = data2;
	wavehdr2_out.dwBufferLength = LENGTH;


	wavehdr3_out.lpData = data3;
	wavehdr3_out.dwBufferLength = LENGTH;


	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL);

	if ( waveOutPrepareHeader(hWaveOut, &wavehdr1_out, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveOutPrepareHeader failed\n");
		return -1;
	}

	if ( waveOutPrepareHeader(hWaveOut, &wavehdr2_out, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveOutPrepareHeader failed\n");
		return -1;
	}

	if ( waveOutPrepareHeader(hWaveOut, &wavehdr3_out, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("waveOutPrepareHeader failed\n");
		return -1;
	}

	if (input_type == NET)
	{
		sock = init_rtltcp("127.0.0.1", 1234);
	}
	else if (input_type == MIC)
	{
		if ( waveInOpen(&hWaveIn, WAVE_MAPPER, &wformat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		{
			printf("waveInOpen failed\n");
			return -1;
		}
	
		if ( waveInPrepareHeader(hWaveIn, &wavehdr1_in, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveInPrepareHeader failed\n");
			return -1;
		}
	
		if ( waveInPrepareHeader(hWaveIn, &wavehdr2_in, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveInPrepareHeader failed\n");
			return -1;
		}
	
		if ( waveInPrepareHeader(hWaveIn, &wavehdr3_in, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("waveInPrepareHeader failed\n");
			return -1;
		}
	
		if ( waveInAddBuffer(hWaveIn, &wavehdr1_in, LENGTH) != MMSYSERR_NOERROR)
		{
			printf("waveInAddBuffer failed\n");
			return -1;
		}
	
		waveInStart(hWaveIn);

		while ( (wavehdr1_in.dwFlags & WHDR_DONE) == 0)
			Sleep(0);
	}

	running = 1;
	while (running)
	{
		bytes_read = ReadBuffer(input_type, data1, LENGTH, stdin, sock, hWaveIn, &wavehdr1_in, &wavehdr2_in);
		if (input_type != NET)
		{
			ProcessBuffer((short int *)data1, bytes_read);
			waveOutWrite(hWaveOut, &wavehdr1_out, sizeof(WAVEHDR));
			
			while (started && (wavehdr2_out.dwFlags & WHDR_DONE) == 0)
			{
				Sleep(1);
			}	
		}
		else
		{
			ProcessNetBuffer((char *)data1, bytes_read);
		}


		bytes_read = ReadBuffer(input_type, data2, LENGTH, stdin, sock, hWaveIn, &wavehdr2_in, &wavehdr3_in);
		if (input_type != NET)
		{
			ProcessBuffer((short int *)data2, bytes_read);
			waveOutWrite(hWaveOut, &wavehdr2_out, sizeof(WAVEHDR));
	
			while (started && (wavehdr3_out.dwFlags & WHDR_DONE) == 0)
			{
				Sleep(1);
			}
		}
		else
		{
			ProcessNetBuffer((char *)data2, bytes_read);
		}

		bytes_read = ReadBuffer(input_type, data3, LENGTH, stdin, sock, hWaveIn, &wavehdr3_in, &wavehdr1_in);
		if (input_type != NET)
		{
			ProcessBuffer((short int *)data3, bytes_read);
			waveOutWrite(hWaveOut, &wavehdr3_out, sizeof(WAVEHDR));
			while ( started && (wavehdr1_out.dwFlags & WHDR_DONE) == 0)
			{
				Sleep(1);
			}
		}
		else
		{
			ProcessNetBuffer((char *)data3, bytes_read);
		}

		started = 1;
	}

	if (input_type == MIC)
	{
		waveInReset(hWaveIn);
		waveInClose(hWaveIn);
	}
	else if (input_type == NET)
		closesocket(sock);

	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);
	return 0;
}




unsigned int __stdcall stream_thread(void *arg)
{
	int type = (int)arg;

	stream_wave(type);
	return 0;
}


#ifdef CONSOLE
int main(void)
{
	_setmode(_fileno(stdin), _O_BINARY);

	stream_wave(PIPE);
	return 0;
}
#endif