#include "include.h"

POINT		apt[NUM];
POINT		apt_fft[NUM];
int		cxClient = 1;
int		cyClient = 1;

COLORREF	waterfall[LENGTH * LENGTH];

float frequency1 = 0.25;
float frequency2 = 0.75;
float thresh1 = 0.25;
float thresh2 = 0.25;

#define SIZE 20
char data_str[SIZE] = "Bits: ";
char freq_str[SIZE] = "";
float blackman[NUM] = {0};
static float	fft_output[LENGTH];

char mstr[8][80] = {0};
POINT mpoint[8] = {0};

HWND ghwnd;
extern int running;
int blackman_flag = 0;
int log_scale = 0;



COLORREF ColorScale[21] = 
{
	0x001D1B3A,
	0x0020007A,
	0x004D4988,
	0x003B0087,
	0x00680795,
	0x00A1008C,
	0x00B70680,
	0x00C91565,
	0x00D62638,
	0x00E34011,
	0x00DF4F05,
	0x00EE6400,
	0x00E07C00,
	0x00F99400,
	0x00FCAB00,
	0x00FCC701,
	0x00FCDA18,
	0x00FCEA5F,
	0x00FCF4B0,
	0x00FCFC17,
	0x00FCFCF8
};


int fft_psd(const short int *data, int size, float *fft_output, float *max, float *min, float *avg);
void draw_view(HDC hdc, HPEN green_pen, HPEN blue_pen, HPEN red_pen, HPEN yellow_pen, HPEN white_pen, RECT *text_rect, RECT *freq_rect, int input_mode, int draw_bins);
void draw_waterfall(HDC hdc, HDC hdcMem, int fft_width, int fft_height, float fft_scale);
float blackman_harris(int n, int N);
void draw_complex(HDC hdc, HPEN white_pen, HPEN red_pen, HPEN green_pen);

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
unsigned int __stdcall stream_thread(void *arg);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("WinFFT");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;
	WSADATA	WSAData;
     
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppName;
          
	if (!RegisterClass (&wndclass))
	{
		MessageBox(NULL, TEXT ("Program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	} 

	WSAStartup(MAKEWORD(2, 2), &WSAData);


     	_setmode(_fileno(stdin), _O_BINARY);
	hwnd = CreateWindow (szAppName, TEXT ("WinFFT"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
     
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	WSACleanup();
	return msg.wParam;
}


LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND	button_normal;
	static HWND	button_distortion;
	static HWND	button_echo;
	static HPEN	red_pen;
	static HPEN	green_pen;
	static HPEN	blue_pen;
	static HPEN	yellow_pen;
	static HPEN	white_pen;
	static HGDIOBJ	old_pen;
	static HANDLE	hThread;
	static RECT	rect = {0, 0, 1, 1};
	static RECT	text_rect;
	static RECT	freq_rect;
	static HDC	hdcMem;
	static int	old_style = 0;
	static int	new_style = WS_CHILD | WS_VISIBLE;
	static int	xres, yres;
	static int	input_mode = MIC; 
	static int	draw_flag = FREQ | TIME;
	HDC		hdc;
	PAINTSTRUCT	ps;
	int		i;


     
	switch (message)
	{
	case WM_CREATE:
	{
		HMONITOR hmon;
		MONITORINFO mi = {sizeof(MONITORINFO)};
		int i;

		white_pen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
		yellow_pen = CreatePen(PS_SOLID, 1, RGB(255,255,0));
		red_pen = CreatePen(PS_SOLID, 1, RGB(255,0,0));
		green_pen = CreatePen(PS_SOLID, 1, RGB(0,255,0));
		blue_pen = CreatePen(PS_SOLID, 1, RGB(0,0,255));

		hThread = (HANDLE)_beginthreadex(0, 0, stream_thread, (void *)input_mode, 0, 0);
		ghwnd = hwnd;

		hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hmon, &mi);

		xres = abs(mi.rcMonitor.right - mi.rcMonitor.left);
		yres = abs(mi.rcMonitor.bottom - mi.rcMonitor.top);

		for(i = 0; i < NUM; i++)
		{
			blackman[i] = blackman_harris(i, NUM);
		}

		return 0;
	}
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		rect.right = cxClient;
		rect.bottom = cyClient;

		text_rect.left = cxClient * 0.25 + 1.5 * MARGIN;
		text_rect.right = cxClient * 0.35 + 400;
		text_rect.top = cyClient * 0.25;
		text_rect.bottom = cyClient * 0.25 + 200;

		freq_rect.left = cxClient * 0.9;
		freq_rect.right = cxClient * 0.99;
		freq_rect.top = cyClient * 0.95;
		freq_rect.bottom = cyClient * 0.99;

		return 0;
	case WM_MBUTTONDOWN:
		sprintf(data_str, "Bits: ");
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			old_style = SetWindowLong(hwnd, GWL_STYLE, new_style);
			new_style = old_style;
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, xres, yres, 0);
		}
		else if (wParam == VK_F1)
		{
			draw_flag ^= BIN;
		}


		return 0;
	case WM_CHAR:
		if (wParam == '0')
		{
			running = 0;
			Sleep(500); // give the thread time to die
			input_mode = PIPE;
			hThread = (HANDLE)_beginthreadex(0, 0, stream_thread, (void *)input_mode, 0, 0);
		}
		else if (wParam == '1')
		{
			running = 0;
			Sleep(500); // give the thread time to die
			input_mode = MIC;
			hThread = (HANDLE)_beginthreadex(0, 0, stream_thread, (void *)input_mode, 0, 0);
		}
		else if (wParam == '2')
		{
			running = 0;
			Sleep(500); // give the thread time to die
			input_mode = NET;
			hThread = (HANDLE)_beginthreadex(0, 0, stream_thread, (void *)input_mode, 0, 0);
		}
		else if (wParam == 'b' || wParam == 'B')
		{
			blackman_flag = !blackman_flag;
		}
		else if (wParam == 't' || wParam == 'T')
		{
			draw_flag ^= TIME;
		}
		else if (wParam == 'f' || wParam == 'F')
		{
			draw_flag ^= FREQ;
		}
		else if (wParam == 'l' || wParam == 'L')
		{
			log_scale = !log_scale;
		}

		return 0;
	case WM_MOUSEMOVE:
	{
		int	x = LOWORD(lParam);
		int	y = HIWORD(lParam);
		float	xf = (float) x / cxClient;
		float	bin_size = (float)(SAMPLE_RATE / NUM) * FFT_SCALE;
		float frequency = 0.0f;

		if (xf < 0)
			xf = 0.0f;


		if (wParam & MK_LBUTTON)
		{
			thresh1 = (float)y / cyClient;
			frequency1 = xf;			
			InvalidateRect(ghwnd, &rect, FALSE);
		}

		if (wParam & MK_RBUTTON)
		{
			thresh2 = (float)y / cyClient;
			frequency2 = xf;
			InvalidateRect(ghwnd, &rect, FALSE);
		}

		frequency = xf * NUM * FFT_SCALE * bin_size;
		if ( frequency / 1000.0 < 1.0)
			sprintf(freq_str, "%f Hz", frequency);
		else
			sprintf(freq_str, "%f Khz", frequency / 1000.0);

		return 0;
	}
	case WM_PAINT:
		if (running)
		{
			hdc = BeginPaint (hwnd, &ps);
			draw_waterfall(hdc, hdcMem, NUM, FFT_HEIGHT, FFT_SCALE);
			if (input_mode != NET)
			{
				draw_view(hdc, green_pen, blue_pen, red_pen, yellow_pen, white_pen, &text_rect, &freq_rect, input_mode, draw_flag);
			}
			else
			{
				if (draw_flag & TIME)
					draw_complex(hdc, white_pen, red_pen, green_pen);
			}
			EndPaint(hwnd, &ps);
		}
		return 0;
	     
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}


void draw_waterfall(HDC hdc, HDC hdcMem, int fft_width, int fft_height, float fft_scale)
{
	HBITMAP hBitmap, hOldBitmap;

	hBitmap = CreateCompatibleBitmap(hdc, fft_width, fft_height);
	SetBitmapBits(hBitmap, fft_width * fft_height, waterfall);
	hdcMem = CreateCompatibleDC(hdc);
	hOldBitmap = SelectObject(hdcMem, hBitmap);

	// This scaling is a little strange because Stretch maintains aspect ratios
	StretchBlt(hdc, 0, 0, (1.0 / fft_scale) * cxClient, (2.0 / fft_scale) * cyClient, hdcMem, 0, 0, fft_width, fft_height, SRCCOPY);
	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}

void draw_complex(HDC hdc, HPEN white_pen, HPEN red_pen, HPEN green_pen)
{
	HPEN old_pen;
	int i;

	RECT	coord = {cxClient - cxClient * 0.4, cyClient - cyClient * 0.4, cxClient,  cyClient};

	SelectObject(hdc, GetStockObject(LTGRAY_BRUSH)); 
	Rectangle(hdc, coord.left, coord.top, coord.right, coord.bottom);


	old_pen = SelectObject(hdc, white_pen);
	// DrawAxis
	MoveToEx (hdc, coord.left, coord.top + (coord.bottom - coord.top) / 2, NULL);
	LineTo   (hdc, coord.right, coord.top + (coord.bottom - coord.top) / 2);
	MoveToEx (hdc, coord.left + (coord.right - coord.left) / 2, coord.top, NULL);
	LineTo   (hdc, coord.left + (coord.right - coord.left) / 2, coord.bottom);



	SelectObject(hdc, red_pen);

	for( i = 0; i < NUM; i++)
	{
		char xc = apt[i].x;
		char yc = apt[i].y;

		int x = (xc / 255.0) * (coord.right - coord.left) + (coord.right - coord.left) * 2;
		int y = (yc / -255.0) * (coord.bottom - coord.top) + (coord.bottom - coord.top) * 2;

		Ellipse(hdc, x, y, x + 5, y + 5);
	}

	SelectObject(hdc, old_pen);
}

void draw_view(HDC hdc, HPEN green_pen, HPEN blue_pen, HPEN red_pen, HPEN yellow_pen, HPEN white_pen, RECT *text_rect, RECT *freq_rect, int input_mode, int draw_flag)
{
	HPEN old_pen;
	old_pen = SelectObject(hdc, green_pen);

	if (draw_flag & TIME)
	{
		// Center line
		MoveToEx (hdc, 0,        cyClient / 2, NULL);
		LineTo   (hdc, cxClient, cyClient / 2);
	}


	SelectObject(hdc, red_pen);
	if (draw_flag & BIN)
	{
		// draw bin1
		MoveToEx(hdc, cxClient * frequency1, 0, NULL);
		LineTo(hdc,   cxClient * frequency1, cyClient);
		MoveToEx(hdc, cxClient * frequency1 + MARGIN, 0, NULL);
		LineTo(hdc,   cxClient * frequency1 + MARGIN, cyClient);
	
		// draw bin2
		MoveToEx(hdc, cxClient * frequency2, 0, NULL);
		LineTo(hdc,   cxClient * frequency2, cyClient);
		MoveToEx(hdc, cxClient * frequency2 + MARGIN, 0, NULL);
		LineTo(hdc,   cxClient * frequency2 + MARGIN, cyClient);
	
		SelectObject(hdc, yellow_pen);
		// draw thresh1
		MoveToEx(hdc, cxClient * frequency1, cyClient * thresh1, NULL);
		LineTo(hdc,   cxClient * frequency1 + MARGIN, cyClient * thresh1);
	
		// draw thresh2
		MoveToEx(hdc, cxClient * frequency2, cyClient * thresh2, NULL);
		LineTo(hdc,   cxClient * frequency2 + MARGIN, cyClient * thresh2);
	
		DrawText(hdc, data_str, strlen(data_str), text_rect, 0);
	}

	if (draw_flag & TIME)
	{
		SelectObject(hdc, green_pen);
		Polyline(hdc, apt, NUM);
	}

	if (draw_flag & FREQ)
	{
		SelectObject(hdc, blue_pen);
		Polyline(hdc, apt_fft, NUM);
	}

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255,255,255));
	DrawText(hdc, freq_str, strlen(freq_str), freq_rect, 0);

	SelectObject(hdc, old_pen);
}

void check_levels(POINT *fft_pixel)
{
	int i;
	int data = -1;

	for(i = 0; i < NUM; i++)
	{
		if (fft_pixel[i].x > cxClient * frequency1 && fft_pixel[i].x < cxClient * frequency1 + MARGIN)
		{
			if (fft_pixel[i].y < cyClient * thresh1 && strlen(data_str) < SIZE - 1)
			{
				data = 0;
			}
		}
	}



	for(i = 0; i < NUM; i++)
	{
		if (fft_pixel[i].x > cxClient * frequency2 && fft_pixel[i].x < cxClient * frequency2 + MARGIN)
		{
			if (fft_pixel[i].y < cyClient * thresh2 && strlen(data_str) < SIZE - 1)
			{
				data = 1;
			}
		}
	}


	if (data != -1 && strlen(data_str) < SIZE - 1)
	{
		if (data)
			strcat(data_str, "1");
		else
			strcat(data_str, "0");

		if (strstr(data_str, "Bits: 1111111111") != NULL)
		{
			sprintf(data_str, "Bits: ");
		}

	}

}


void ProcessNetBuffer(char *data, int length)
{
	RECT	rect = {0, 0, cxClient, cyClient};
	float max = -10000;
	float min =  10000;
	float avg = 0;


	int i, j, y;

	for (i = 0; i < length / 2;)
	{
		apt[i].x = data[i++];
		apt[i].y = data[i++];
	}


	fft_psd_net(data, length / 2, fft_output, &max, &min, &avg);


	//copy waterfall down one pixel (inefficient, but works well enough)
	for(y = FFT_HEIGHT-1; y >= 0; y--)
	{
		memcpy(&waterfall[(y+1) * NUM], &waterfall[y * NUM], NUM * sizeof(COLORREF));
	}

	for (i = 0; i < length * FFT_SCALE + FFT_OFFSET; i++)
	{
		int index = 0;

		index = (fft_output[i + FFT_OFFSET] / (4 * avg)) * 18;

		if (index < 0)
			index = 0;

		if (index >= 21)
			index = 20;

		waterfall[i] = ColorScale[index];
	}
	InvalidateRect(ghwnd, &rect, FALSE);
}


// Length in bytes, not short ints
void ProcessBuffer(short int *data, int length)
{
	RECT	rect = {0, 0, cxClient, cyClient};
	int i;
	int x, y;
	float max = -10000;
	float min =  10000;
	float avg = 0;

	length = length >> 1;

	if (length != NUM)
		return;

	for (i = 0; i < length; i++)
	{
		apt[i].x = i * cxClient / length;
		apt[i].y = (int) (0.5 * cyClient * (1.0 - data[i] / 32767.0));
	}

	fft_psd(data, length, fft_output, &max, &min, &avg);


	//copy waterfall down one pixel (inefficient, but works well enough)
	for(y = FFT_HEIGHT-1; y >= 0; y--)
	{
		memcpy(&waterfall[(y+1) * NUM], &waterfall[y * NUM], NUM * sizeof(COLORREF));
	}

	for (i = 0; i < length * FFT_SCALE + FFT_OFFSET; i++)
	{
		int index = 0;

		apt_fft[i].x = (i + FFT_OFFSET) * cxClient / (length * FFT_SCALE);

		if (log_scale)
			apt_fft[i].y = 0.5 * cyClient - (int)(10 * log(fft_output[i + FFT_OFFSET]));
		else
			apt_fft[i].y = cyClient - cyClient * fft_output[i + FFT_OFFSET];

		index = (fft_output[i + FFT_OFFSET] / (avg - min)) * 21;

		if (index < 0)
			index = 0;

		if (index >= 21)
			index = 20;


		waterfall[i] = ColorScale[index];
	}

	for (; i < length; i++)
	{
		apt_fft[i].x = cxClient;
		apt_fft[i].y = cyClient;
	}

	check_levels(apt_fft);

/*
	if (0)
	{
		float fdata[30];

		for(i = 0; i < length % 30; i+=30)
		{
			if (i > MIN(length, NUM))
				break;

			pcm_to_float(&data[i], fdata);
			fir_filter(&fdata[i]);
			float_to_pcm(&filt_data[i], fdata);
		}
	}
*/
	InvalidateRect(ghwnd, &rect, FALSE);
}



float blackman_harris(int n, int N)
{
	float seg1, seg2, seg3, w_n;
	float a0 = 0.35875;
	float a1 = 0.48829;
	float a2 = 0.14128;
	float a3 = 0.01168;

	seg1 = a1 * (float) cos((float)(2 * MYPI * n)/(float) (N - 1));
	seg2 = a2 * (float) cos((float)(4 * MYPI * n)/(float) (N - 1));
	seg3 = a3 * (float) cos((float)(6 * MYPI * n)/(float) (N - 1));

	w_n = a0 - seg1 + seg2 - seg3;

	return w_n;
}
