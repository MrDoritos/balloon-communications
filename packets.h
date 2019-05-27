#pragma once
#include "serialClient.h"
#if defined __linux__ || defined _WIN32
#include <time.h>
#include <fstream>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILE_CSV 0b0010
#define FILE_TSV 0b0001
#define FILE_BINARY 0b0100

class timeSync :
	public serialPacket {
	public:
	timeSync(serialClient& client) :
		serialPacket(client, SYNC_TIME) {
		
	}
	long translate(long time) {
		return time - this->syncMillis;
	}
	#if defined __linux__ || defined _WIN32
	void serialize(std::ofstream file, int type) {
		/* Don't want to write time syncs to file */
	}
	#endif
	long syncMillis;
	#if defined __linux__ || defined _WIN32
	time_t syncTime;
	int receivePacket() override {
		int i = receive();
		if (!dataAvailable)
			return GENERAL_ERROR;
		if (dataLeft != 8)
			return GENERAL_ERROR;
		receiveData((char*)&syncMillis, 8);
		time(&syncTime);
	}
	#endif
	#if defined(__AVR_ATmega328P__)
	int send(long mill) override {
		((serialPacket*)this)->send(millis(), mill);
	}
	#endif
};

class tempSensor :
	public serialPacket {
	public:
	tempSensor(serialClient& client, int id, timeSync& ts) :
		serialPacket(client, TEMP_SENSOR | id) {
		this->num = id;
		this->ts = &ts;
	}
	#if defined __linux__ || defined _WIN32
	void serialize(std::ofstream file, int type) {
		if ((type & FILE_TSV) == FILE_TSV) {
			std::string f;
			char buf[256];
			memset(&buf[0], '\0', 256);
			sprintf(&buf[0], "TEMP %i,%l,%d\n", this->num, this->ts->translate(mtime), this->tempurature);
			file << f;
		}
	}
	#endif
	double tempurature;
	#if defined(__AVR_ATmega328P__)
	int send(long millis) override {
		((serialPacket*)this)->send(tempurature, millis);
	}
	#endif
	#if defined __linux__ || defined _WIN32
	int receivePacket() override {
		int i = receive();
		if (!dataAvailable)
			return GENERAL_ERROR;
		if (dataLeft != 8)
			return GENERAL_ERROR;
		receiveData((char*)&tempurature, 8);
	}
	#endif
	protected:
	
	private:
	int num;
	timeSync* ts;
};

class uvSensor :
	public serialPacket {
	public:
	uvSensor(serialClient& client, int id, timeSync& ts) :
		serialPacket(client, UV_SENSOR | id) {
		this->num = id;
		this->ts = &ts;
	}
	int num;
	#if defined __linux__ || defined _WIN32
	void serialize(std::ofstream file, int type) {
		if ((type & FILE_TSV) == FILE_TSV) {
			std::string f;
			char buf[256];
			memset(&buf[0], '\0', 256);
			sprintf(&buf[0], "UV %i\t%l\t%d\n", this->num, this->ts->translate(mtime), this->value);
			file << f;
		}
	}
	#endif
	#if defined __linux__ || defined _WIN32
	int receivePacket() override {
		int i = receive();
		if (!dataAvailable)
			return GENERAL_ERROR;
		if (dataLeft != 8)
			return GENERAL_ERROR;
		receiveData((char*)&value, 8);
	}
	#endif
	double value;
	#if defined(__AVR_ATmega328P__)
	int send(long millis) override {
		((serialPacket*)this)->send(value, millis);
	}
	#endif
	private:
	timeSync* ts;
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
