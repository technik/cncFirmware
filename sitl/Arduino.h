// Mock arduino interface
#pragma once
#include <iostream>

struct SerialComm
{
	void begin(unsigned baudrate) {}
	void write(unsigned x) { std::cout << x << "\n"; }

	static SerialComm com0;
};

#define Serial SerialComm::com0
