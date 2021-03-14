// Mock arduino interface
#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

// Arduino mega pin definitions
#define NUM_DIGITAL_PINS            70
#define NUM_ANALOG_INPUTS           16
#define analogInputToDigitalPin(p)  ((p < 16) ? (p) + 54 : -1)
#define digitalPinHasPWM(p)         (((p) >= 2 && (p) <= 13) || ((p) >= 44 && (p)<= 46))

#define PIN_SPI_SS    (53)
#define PIN_SPI_MOSI  (51)
#define PIN_SPI_MISO  (50)
#define PIN_SPI_SCK   (52)

static const uint8_t SS = PIN_SPI_SS;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK = PIN_SPI_SCK;

#define PIN_WIRE_SDA        (20)
#define PIN_WIRE_SCL        (21)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

#define LED_BUILTIN 13

#define PIN_A0   (54)
#define PIN_A1   (55)
#define PIN_A2   (56)
#define PIN_A3   (57)
#define PIN_A4   (58)
#define PIN_A5   (59)
#define PIN_A6   (60)
#define PIN_A7   (61)
#define PIN_A8   (62)
#define PIN_A9   (63)
#define PIN_A10  (64)
#define PIN_A11  (65)
#define PIN_A12  (66)
#define PIN_A13  (67)
#define PIN_A14  (68)
#define PIN_A15  (69)

static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;
static const uint8_t A3 = PIN_A3;
static const uint8_t A4 = PIN_A4;
static const uint8_t A5 = PIN_A5;
static const uint8_t A6 = PIN_A6;
static const uint8_t A7 = PIN_A7;
static const uint8_t A8 = PIN_A8;
static const uint8_t A9 = PIN_A9;
static const uint8_t A10 = PIN_A10;
static const uint8_t A11 = PIN_A11;
static const uint8_t A12 = PIN_A12;
static const uint8_t A13 = PIN_A13;
static const uint8_t A14 = PIN_A14;
static const uint8_t A15 = PIN_A15;

// Common Arduino Macros
#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))

struct SerialComm
{
	void begin(unsigned baudrate) {}
	template<class T>
	void print(T x) { std::cout << x; }
	template<class T>
	void println(T x) { std::cout << x << "\n"; }

	void InitFromFile(const std::string& filePath)
	{
		m_file.open(filePath);
	}

	int available()
	{
		return (m_file.is_open() && !m_file.eof())? 1 : 0;
	}

	size_t readBytes(char* buffer, size_t length)
	{
		m_file.read(buffer, length);
		std::cout << std::string(buffer, length);
		return length;
	}

	static SerialComm com0;

private:
	std::ifstream m_file;
};

inline void delay(unsigned long ms)
{
	auto sleepTime = std::chrono::milliseconds(ms);
	std::this_thread::sleep_for(sleepTime);
}

inline void delayMicroseconds(unsigned int us)
{
	auto sleepTime = std::chrono::microseconds(us);
	auto t0 = std::chrono::steady_clock::now();
	while(std::chrono::steady_clock::now() - t0 < sleepTime)
	{ }
}

inline void pinMode(uint8_t pin, uint8_t mode)
{}

inline int analogRead(uint8_t pin) { return 0; }

#define Serial SerialComm::com0
