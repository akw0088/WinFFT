WinFFT
	Used for a EE project, will display PSD, waterfall, and time domain signal from microphone, PCM piped input (intended for use with rtl_fm), or TCP input (intended for use with rtl_tcp)

	Hotkeys
		Escape	fullscreen
		F1		Show frequency bins
		0		PCM Input
		1		Microphone input
		2		tcp input
		b		enable/disable blackman harris 4 windowing
		l		enable/disable log scale for frequency display
		t		show/hide time display (green)
		f		show/hide frequency display (blue)

Microphone
	To use with the microphone, just start the application as this is the default mode


PCM input from rtl_fm

	rtl_fm -f 102.1M -M wbfm | main.exe

(Then press 0 to stream)



High frequency TCP input (displays waterfall similar to SDR#)

	First start rtl_tcp (program will only connect to localhost with default port 1234)

	rtl_tcp -a 127.0.0.1 -f 101M -s 100

	Then start winfft and press '2' to connect



SOX is similar to aplay in linux and can be used to record FM / play other PCM input into WinFFT. [The PCM input is assumed to be 16 bit PCM at a sample rate of 32k]


Example, recording FM to a file
	rtl_fm -f 102.1M -M wbfm | "C:\Program Files\sox-14-4-1\sox.exe" -t raw -r 32k -b 16 -c 1 -e signed-integer - -d

	-b 16 (bits)
	-r 32k sample rate-c channel one
	-e signed integer (type)
	- pipe from file
	-d play to default output




Compiling using visual studio command prompt
	cl main.c process.c kiss.c kiss_fft.c low_pass.c /link kernel32.lib user32.lib gdi32.lib winmm.lib wsock32.lib