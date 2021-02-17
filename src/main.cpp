#include <Arduino.h>
#include <hal/gpio_pin.h>
#include <coroutine>
#include <staticRingBuffer.h>
#include <chrono>

using namespace etl::hal;
using namespace etl;

struct Promise
{};

struct Task
{
	using promise_type = Promise;
};

template<class T>
struct TimedPromise;

template<class T>
struct TimedTask
{
	using promise_type = TimedPromise<T>;
};

template<class T>
struct TimedPromise
{	
	using Clock = std::chrono::steady_clock;
	using promise_type = Promise;

	Clock::time_point due;

	// Promise interface
	auto initial_suspend() { return std::suspend_always{}; }
	auto final_suspend() { return std::suspend_never{}; }
	void return_value(T) {}
	TimedTask<T> get_return_object() { return {}; }
};

class TaskScheduler
{
public:
	static TaskScheduler& Get() {
		static TaskScheduler singleton;
		return singleton;
	}

	static constexpr size_t kMaxRealTimeTasks = 16;
	static constexpr size_t kMaxPriorityTasks = 8;
	static constexpr size_t kMaxBackgroundTasks = 8;
    // Task queues
	FixedRingBuffer<Task, kMaxRealTimeTasks> 	realTimeQueue;
	FixedRingBuffer<Task, kMaxPriorityTasks> 	priorityQueue;
	FixedRingBuffer<Task, kMaxBackgroundTasks> 	backgroundQueue;

	void Continue()
	{
		if(!realTimeQueue.empty())
		{
			realTimeQueue.pop_front();
			return;
		}
	}
};

// G-Code parsing loop
// {
//		for(;;)
//			line = co_await serial.getLine();
//			opCode = parseLine(line);
//			co_yield opCode;
// }

// opcode execution loop
// Task<> executeGCode();
// {
	//	for(;;)
	//		op = co_await GCodeParse();
	//		// Invoke op from a call table
	//		operation[op]();
// }

// motorControl();
// {
// 		co_await SetupMotors();
//		res = co_await RunDiagnostics();
//		if(res) for(;;) co_await StepMotors();
// }

void setup() {
	// Setup scheduler
	// Setup serial port
	Serial.begin(115200);
	// Start motor control
	// motorControl();
	// Start message processing
	// executeGCode();
}

void loop() {
  // Consume work from the task queue
  // Global work flow looks like:
  // - Interrupt from hardware stops
  // - Interrupt from motor control
  // - Consume data from the serial port. Interpret the G-Code and write ack to it
  TaskScheduler::Get().Continue();
}