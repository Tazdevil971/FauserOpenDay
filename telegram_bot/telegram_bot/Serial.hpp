#pragma once
#include <Windows.h>
#include <string>

class Serial {
public:
	Serial(std::string comName);
	Serial(Serial&) = delete;
	Serial(Serial&& serial);

	~Serial();

	void begin(int baudRate);

	void print(std::string a);
	void print(char a);

	int available();
	char read();

private:
	HANDLE m_hCom;
};