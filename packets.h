#pragma once
#include "serialClient.h"
#include <time.h>
#include <fstream>
#define FILE_CSV 0b0010
#define FILE_TSV 0b0001
#define FILE_BINARY 0b0100

class timeSync :
	public serialPacket {
	public:
	timeSync(serialClient& client) :
		serialPacket(client, SYNC_TIME) {
		
	}
	void serialize(std::ofstream file, int type) {
	
	}
	long time;
	#if defined __linux__ || defined _WIN32
	int receivePacket() override {
		int i = receive();
		if (!dataAvailable)
			return GENERAL_ERROR;
		if (dataLeft != 8)
			return GENERAL_ERROR;
		receiveData((char*)&time, 8);
	}
	#endif
	#if defined(__AVR_ATmega328P__)
	int send() override {
		send(millis(), 8);
	}
	#endif
};

class tempSensor :
	public serialPacket {
	public:
	tempSensor(serialClient& client, int id) :
		serialPacket(client, TEMP_SENSOR | id) {
		
	}
	double tempurature;
	int send() override {
		
	}
	protected:
	
	private:
	
};

class uvSensor :
	public serialPacket {
	public:
	uvSensor(serialClient& client, int id) :
		serialPacket(client, UV_SENSOR | id) {
		
	}
	double value;
	int send() override {
	
	}
};

//#define SaveAs(idd, xyzw) (((xyzw & idd) == xyzw))
#if defined __linux__ || defined _WIN32
class packetToDisk {
	public:
		packetToDisk(int forms) :
			formats(forms) {
			time_t ntime;
			time(&ntime);
			std::string fName = std::to_string(ntime);
			if ((formats & FILE_TSV) == FILE_TSV) {//if (SaveAs(formats, FILE_TSV)) {
				tsv = std::ofstream(fName + ".tsv");
			}
			if ((formats & FILE_CSV) == FILE_CSV) {//if (SaveAs(formats, FILE_CSV)) {
				csv = std::ofstream(fName + ".csv");
			}
			if ((formats & FILE_BINARY) == FILE_BINARY) {//if (SaveAs(formats, FILE_BINARY)) {
				binary = std::ofstream(fName + ".bin");
			}
		}
#if defined __linux__
		void write(serialPacket& packet) {
			
			if ((formats & FILE_TSV) == FILE_TSV) {
			
			}
			if ((formats & FILE_CSV) == FILE_CSV) {
			
			}
			if ((formats & FILE_BINARY) == FILE_BINARY) {
			
			}
		}
#endif
	private:
		const int formats;
		std::ofstream tsv, csv, binary;
};
#endif
//#undef SaveAs
