#pragma once
//Packet types
#define ROTATE_HOME 0x0011
#define ROTATED 0x0012
#define ROTATED_HOME 0x0012
#define ROLL_HOME 0x0013
#define ROLL_EDGE 0x0014
#define ROLLED_HOME 0x0015
#define ROLLED_EDGE 0x0016
#define HALT 0x0017
#define PWR_FAIL 0x0018
#define MTR_DISC 0x0019
#define ROLL 0x001A
#define ROTATE 0x001B
#define RESET 0x001C
#define ROLL_PULLDOWN 0x001D
#define ROTATE_PULLDOWN 0x001F
#define DEBUG_MSG 0x0020
#define DEBUG_2 0x0021

//Balloon Communications Definitions
#define SYNC_TIME 0x0011
#define TEMP_SENSOR 0x0100
#define UV_SENSOR 0x0200
#define TEMP_1 0x0011
#define TEMP_2 0x0012
#define TEMP_3 0x0013
#define UV_1 0x0014

//Error types
#define IsError(xxx) ((xxx & ERRMASK) > 0)
#define ERRMASK 0xF000
#define DISCONNECTED 0x1000
#define _EOF 0x2000
#define GENERAL_ERROR 0x3000

#define NO_ERROR 0x0000

#if defined _WIN32 && defined GCC
//#include <SDKDDKVer.h>
#endif

#if defined(__AVR_ATmega328P__)
#include "Arduino.h"
#elif __linux__
#include <unistd.h>
#include <sys/ioctl.h>
#elif _WIN32
#include <windows.h>
#endif

#if defined _WIN32 || defined __linux__
#include <iostream>
#include <climits>
#if defined GCC && defined _WIN32
#include "mingw.condition_variable.h"
#include "mingw.mutex.h"
#include "mingw.thread.h"
#else
#include <condition_variable>
#include <thread>
#include <mutex>
#endif
#endif

//#define ifarduino if defined(__AVR_ATmega328P__)

class serialClient {
public:
#if defined(__AVR_ATmega328P__)
serialClient() {}
#elif __linux__
serialClient(int fd)
{
this->fd = fd;
}
#elif _WIN32
serialClient(HANDLE fd)
{
this->fd = fd;
}
#endif
#ifdef __linux__
int fd;
#elif _WIN32
HANDLE fd;
#endif
#if defined __linux__ || defined _WIN32
virtual void onRotateHome() {}
virtual void onRotated() {}
virtual void onRollEdge() {}
virtual void onRollHome() {}
virtual void onRolled() {}
virtual void onRolledEdge() {}
virtual void onHalt() {}
virtual void onPowerFailure() {}
virtual void onMotorDisconnect() {}
virtual void onRoll() {}
virtual void onRotate() {}
virtual void onReset() {}
virtual void onRollPulldown() {}
virtual void onRotatePulldown() {}
virtual void onUpdate(float elapsedTime) {}
bool doLoop = true;
//void eventLoop(serialClient* client) {
static void eventLoop(serialClient* client){
    while (client->doLoop) {        
    short rec = client->blockingRecieve();
	std::cout << "RECIEVE" << std::endl;
    //if (!IsError) 
        client->handle(rec);
    //std::cout << "RECIEVE: ";
    //std::printf("0x%04x\r\n", rec);
    }
}
void waitFor(short sSend, short sRecieve) {
    if (sRecieve - ROTATE_HOME < 0) {
        return;
    }
    event* n = getEvent(sRecieve);
    send(sSend);
    n->waitFor();
}
void waitFor(int eventNum) {
    if (eventNum - ROTATE_HOME < 0) {
        return;
    }
    getEvent(eventNum)->waitFor();
    //event* evt = getEvent(eventNum);    
    //std::unique_lock<std::mutex> lock(evt->m);    
    //evt->updated.wait(lock, [evt]{return evt->happened; });
}
private:
struct event {
    public:
	event() {
		happened = false;
	}
    //bool happened=false;
    bool hasHappened() {return happened;}
	//std::atomic<bool> happened;
	bool happened;
    std::chrono::time_point<std::chrono::system_clock> time_point;
    void waitFor() {
        //std::cout << "ee" << std::endl;
        //while (!happened) {Sleep(50); /*std::cout << "." << std::endl;*/}
        //happened = false;
		std::unique_lock<std::mutex> l(mtx);
		cv.wait(l, [this]{return happened;});
    }
    void happen() {
        happened = true;
		cv.notify_all();
    }
    void unhappen() {
        happened = false;
    }
	private:
	std::condition_variable cv;
	std::mutex mtx;
} events[20];
public:
event* getEvent(int eventNum) {
    return &events[eventNum - ROTATE_HOME];
}
#endif

short send(short data) {
#if defined(__AVR_ATmega328P__)
if (!Serial)
        return DISCONNECTED;
uint8_t* buf;
buf = (uint8_t*)&data;
Serial.write(buf, 2);
#elif __linux__
char* buf;
buf = (char*)&data;
write(fd, buf, 2);
#elif _WIN32
char* buf;
buf = (char*)&data;
LPDWORD b = (LPDWORD)alloca(sizeof(DWORD));
WriteFile(fd, buf, 2, b, NULL);
#endif
return NO_ERROR;
}

int send(void* buffer, int length) {
#if defined(__AVR_ATmega328P__)
if (!Serial)
	return DISCONNECTED;
Serial.write((uint8_t*)buffer, length);
#elif __linux__
write(fd, buffer, length);
#elif _WIN32
LPDWORD b = (LPDWORD)alloca(sizeof(DWORD));
WriteFile(fd, buffer, length, b, NULL);
#endif
return NO_ERROR;
}

#if defined __linux__ || defined _WIN32
~serialClient() {
#ifdef __linux__
	close(fd);
#elif _WIN32
	CloseHandle(serialHandle);
#endif
}
#endif

int available() {
#if defined(__AVR_ATmega328P__)
return Serial.available();
#elif __linux__
int length = 0;
ioctl(fd, FIONREAD, &length);
return length;
#elif _WIN32
COMSTAT cmStat;
LPDWORD lperrors = (LPDWORD)alloca(sizeof(DWORD));
ClearCommError(fd, lperrors, &cmStat);
return cmStat.cbInQue;
#endif
}

#if defined __linux__ || defined _WIN32
void update(short packet) {
    int n = packet - ROTATE_HOME;
    for (int i = 0; i < 20; i++) {
        if (i == n) {
            getEvent(i + ROTATE_HOME)->happen();
        } else {
            getEvent(i + ROTATE_HOME)->unhappen();
        }
    }
}
void handle(short packet) {
    for (int i = 0; i < 20; i++) {
        events[i].unhappen();
    }
    if (packet - ROTATE_HOME >= 0) {
        event* pct = getEvent(packet);
    pct->time_point = std::chrono::system_clock::now();
    pct->happen();
    //update(packet);
    //std::unique_lock<std::mutex> lock(pct->m);
    //pct->happened = true;
    //pct->updated.notify_all();
    }
    switch(packet) {
        case ROTATED_HOME:
        onRotate();
        return;
        case ROLLED_EDGE:
        onRolledEdge();
        return;
        case ROLLED_HOME:
        onRolled();
        return;
        case PWR_FAIL:
        onPowerFailure();
        return;
        case MTR_DISC:
        onMotorDisconnect();
        return;
    }
}
#endif

#if defined __linux__ || defined _WIN32
short blockingRecieve() {
char data[3];
data[2] = '\0'; 
#ifdef _WIN32
LPDWORD d = (LPDWORD)alloca(sizeof(DWORD));
//while (!isPacketReady()) {Sleep(50);}
ReadFile(fd, &data, 2, d, NULL);
if (*d == 0) return 0x0000;
//std::cout << "ere" << std::endl;
#elif __linux__
read(fd, &data, 2);
#endif  
return ((short(data[1]) << 8) | short(data[0]));
}
#endif

int recieve(void* buffer, int length) {
#if defined(__AVR__ATmega328P__)
if (available() < length)
	return _EOF;
return Serial.readBytes(&buffer, length);
#elif __linux__
if (available() < length)
	return _EOF;
return read(fd, &buffer, length);
#elif _WIN32
if (available() < length)
	return _EOF;
LPDWORD d = (LPDWORD)alloca(sizeof(DWORD));
ReadFile(fd, &buffer, length, d, NULL);
return *d;
#endif
}

short recieve() {
#if defined(__AVR__ATmega328P__)
if (!Serial)
	return DISCONNECTED;
if (!isPacketReady())
	return _EOF;
short data[2];
data[0] = Serial.read();
data[1] = Serial.read();
return ((data[1] << 8) | data[0]);
#elif __linux__
if (!isPacketReady())
	return _EOF;
char data[2];
read(fd, &data, 2);
//stream->read(&data[0], 2);
return ((short(data[1]) << 8) | short(data[0]));
#elif _WIN32
if (!isPacketReady())
    return _EOF;
char data[2];
LPDWORD d = (LPDWORD)alloca(sizeof(DWORD));
ReadFile(fd, &data, 2, d, NULL);
return ((short(data[1]) << 8) | short(data[0]));
#endif
}
//virtual bool onRecieve(short data) {
//return true;
//}
//virtual void onClose() {
//}
bool isReady();
bool isPacketReady() {
#if defined(__AVR__ATmega328P__)
return (Serial.available() > 1);
#elif __linux__
bool gn;
//stream->seekg(0, stream->end);
size_t length = 0;
ioctl(fd, FIONREAD, &length);
if (length > 1)
	return true;
else
	return false;
#elif _WIN32
return (available() > 1);
//gn = (length > 1);
//if (length < 0) return gn;
//stream->seekg(0, stream->beg);
//return gn;
#endif
}
void flush() {
#if defined(__AVR__ATmega328P__)
while (Serial.available())
        Serial.read();
#else
//stream->clear();
//stream->ignore(INT_MAX);
#endif
}

#if defined(__AVR__ATmega328P__)
bool begin(int baud) {
Serial.begin(baud);
if (!Serial)
        return DISCONNECTED;
else
        return 0x0000;
}

#else
bool begin(int baud, char* port);
#endif
};

struct serialPacket {
	public:
		#if defined (__AVR_ATmega328P__)
		virtual int send(long millis) {
			int d = 0;
			return send(d, millis);
		}
		#endif
		#if defined __linux__ || defined _WIN32
		virtual int receivePacket() {
			return receive();
		}
		#endif
		#if defined __linux__ || defined _WIN32
		virtual void serialize(std::ofstream stream, int type) = 0;
		#endif
	public:
		serialPacket(serialClient& client, short type) {
			this->type = type;
			this->mtime = 0;
			this->dataLength = 0;
			this->client = &client;
		}
		
		//#if defined (__AVR_ATmega328P__)
		template<typename T>
		int send(T data, long mtime) {
			dataLength = sizeof(T);
			this->mtime = mtime;
			client->send((char*)&type, 2);
			client->send((char*)&mtime, 8);
			client->send((char*)&dataLength, 4);
			client->send((char*)&data, dataLength);
		}
		
		//#endif
		
		#if defined __linux__ || defined _WIN32
		int receive() {
			short type;
			client->recieve((char*)&type, 2);
			long mtime;
			client->recieve((char*)&mtime, 8);
			int dataLength;
			client->recieve((char*)&dataLength, 4);
			this->dataLeft = dataLength;
			if (this->dataLeft)
				this->dataAvailable = true;
			this->type = type;
			this->mtime = mtime;
			this->dataLength = dataLength;
		}
		#endif
		
		#if defined __linux__ || defined _WIN32
		int receiveData(char* buffer, int length) {
			if (!dataAvailable)
				return 0;
		
			char b;
			int count = 0;
			for (; count < length && count < dataLeft; count++) {
				dataLeft--;
				client->recieve(&b, 1);
				buffer[count] = b;
			}
			return count;
		}
		#endif
		
		protected:
		bool dataAvailable;
		int dataLeft;
		short type;
		int dataLength;
		long mtime;
		
		private:
		serialClient* client;
		
};
