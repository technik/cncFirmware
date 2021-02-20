// Mock arduino interface
#pragma once

struct SerialComm
{
	void begin(unsigned baudrate) {}

	static SerialComm com0;
};

#define Serial SerialComm::com0
