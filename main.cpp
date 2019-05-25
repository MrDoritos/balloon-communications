#include "serialClient.h"
#include "packets.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
//#include <iofstream>
#include <ctime>
#include <chrono>
//host sHost;
#ifdef __linux__
#define delay(timeXs) usleep(timeXs)
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <libexplain/tcsetattr.h>
#elif _WIN32
#define delay(timeXs) Sleep(timeXs)
#include <windows.h>
#endif

#if defined GCC && defined _WIN32
#include "mingw.condition_variable.h"
#include "mingw.mutex.h"
#include "mingw.thread.h"
#else
#include <condition_variable>
#include <thread>
#include <mutex>
#endif

void error(const char* message, int errnum) {
	std::cout << message << std::endl;
	exit(errnum);
}

class host :
	public serialClient {
	public:
		host(int fd, int formats) :
		serialClient(fd), disk(formats) {
		
		}
		
		long millis;
		int loop() {
			
		}
		template<typename T>
		void changeFd(T fd) {
			this->fd = fd;
		}
	private:
		packetToDisk disk;
};

std::thread loop;

#ifdef _WIN32
HANDLE serialHandle;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
	switch (fdwCtrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
        //sHost.doLoop = false;
        //t1.join();
        //tMain.join();
		CloseHandle(serialHandle);
		return TRUE;
	}
}
#endif


int main(int argc, char** argv) {
	time_t timet;
	if (argc < 2)
		error("No arguments", 1);
	char* device = argv[1];
	host sHost(0,FILE_CSV);
#ifdef __linux__
	termios attribs;
	speed_t speed;
	int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
		error("Got an invalid file descriptor for the serialport\r\nTry as root?", 2);
	if (tcgetattr(fd, &attribs) < 0)
		error("Invalid serial port created", 3);
	attribs.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	int a;
	if ((a = tcsetattr(fd, TCSANOW, &attribs)) < 0) {
		//int err = errno;
			//printf("%s\n", explain_tcsetattr(err, fd, (termios*)&attribs));
		error("Invalid attributes for serialport neccessary for operation", a);
	}
	sHost.changeFd(fd);
#elif _WIN32
	//HANDLE serialHandle;
	//loop = true;
	size_t size = strlen(device) + 1;
	wchar_t* t = (wchar_t*)alloca((sizeof(wchar_t) * size));
	t[strlen(device)] = L'\0';
#ifdef GCC
	mbstowcs(t, device, strlen(device));
	serialHandle = CreateFile(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#else
	size_t a;
	mbstowcs_s(&a, t, size, device, size - 1);
	serialHandle = CreateFile(t, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#endif

	// Do some basic settings
	DCB serialParams = { 0 };
	serialParams.DCBlength = sizeof(serialParams);

	GetCommState(serialHandle, &serialParams);
	serialParams.BaudRate = CBR_9600;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = FALSE;
	SetCommState(serialHandle, &serialParams);

	// Set timeouts
	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 2000000;
	timeout.ReadTotalTimeoutConstant = 2000000;
	timeout.ReadTotalTimeoutMultiplier = 1;
	timeout.WriteTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutMultiplier = 10;

	SetCommTimeouts(serialHandle, &timeout);

	//sHost(serialHandle, FILE_CSV);
    	sHost.changeFd(serialHandle);
#endif

puts("Serial port open");

loop = std::thread(&host::loop, (host*)&sHost);

loop.join();

puts("Loop Stopped");
return 0;

}
