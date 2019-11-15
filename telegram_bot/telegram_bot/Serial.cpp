#include "Serial.hpp"

Serial::Serial(std::string comName) :
	m_hCom(CreateFile(comName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) {

	if(m_hCom == INVALID_HANDLE_VALUE)
		throw std::exception("cannot open com port!");
}

Serial::Serial(Serial&& serial) :
	m_hCom(serial.m_hCom) {}

Serial::~Serial() { CloseHandle(m_hCom); }

void Serial::begin(int baudRate) {
	SetupComm(m_hCom, 128, 128);

	DCB dcb;
	GetCommState(m_hCom, &dcb);

	dcb.BaudRate = baudRate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(m_hCom, &dcb);
}

void Serial::print(std::string a) { WriteFile(m_hCom, a.c_str(), a.size(), NULL, NULL); }
void Serial::print(char a) { WriteFile(m_hCom, &a, sizeof(char), NULL, NULL); }

int Serial::available() {
	COMSTAT comStat;
	ClearCommError(m_hCom, NULL, &comStat);

	return comStat.cbInQue;
}

char Serial::read() {
	char a;
	ReadFile(m_hCom, &a, sizeof(char), NULL, NULL);
	return a;
}

