#pragma once
#include "serialClient.h"
#include <time.h>
#include <fstream>
#define FILE_CSV 0b0010
#define FILE_TSV 0b0001
#define FILE_BINARY 0b0100

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
			if (formats & FILE_TSV == FILE_TSV) {//if (SaveAs(formats, FILE_TSV)) {
				tsv = std::ofstream(fName + ".tsv");
			}
			if (formats & FILE_CSV == FILE_CSV) {//if (SaveAs(formats, FILE_CSV)) {
				csv = std::ofstream(fName + ".csv");
			}
			if (formats & FILE_BINARY == FILE_BINARY) {//if (SaveAs(formats, FILE_BINARY)) {
				binary = std::ofstream(fName + ".bin");
			}
		}
//#if defined __linux__
//		void write(serialPacket& packet) {
//			
//		}
//#endif
	private:
		const int formats;
		std::ofstream tsv, csv, binary;
};
#endif
//#undef SaveAs
