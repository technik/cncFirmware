#include <Arduino.h>
#include <hal/gpio_pin.h>
#include <coroutine>
#include <staticRingBuffer.h>
#include <chrono>

using namespace etl::hal;
using namespace etl;

namespace nes::ted {}

struct Promise;

struct Task
{
	using promise_type = Promise;
	constexpr bool await_ready() const noexcept { return false; }
	void await_suspend(std::coroutine_handle<> h);
    void await_resume();
};

struct Promise
{
	auto initial_suspend() noexcept { return std::suspend_never{}; }
	auto final_suspend() noexcept { return std::suspend_never{}; }
	Task get_return_object() const noexcept;
	void return_void();
	void unhandled_exception() const noexcept {}

	template<class Awaitable>
	Awaitable await_transform(Awaitable expr)
	{
		//expr.then(m_handle); // Append my own execution at the end of the awaitable expression
		return expr;
	}
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

template<class Clock_>
class TimeoutPromise;

template<class Clock_>
class AwaitableTimeout
{
public:
	using time_point = typename Clock_::time_point;
	using promise_type = TimeoutPromise<Clock_>;

	AwaitableTimeout(time_point t) noexcept : m_t(t) {}

	// Note: Instead of always shceduling the task, we could check here whether we passed the time already.
	// However, periodic tasks are usually scheduled well in advance, so it's not very useful in practice.
	constexpr bool await_ready() const noexcept { return false; }

	// Awaiter interface
	void await_suspend(std::coroutine_handle<> h) {
		m_coHandle = h;
		// Add h to the scheduler so it resume us;
    }
    void await_resume() {}
	void then(std::coroutine_handle<> postOp) { m_post = postOp; }
private:
	time_point m_t;
	std::coroutine_handle<> m_coHandle;
	std::coroutine_handle<> m_post;
};

template<class Clock_>
class TimeoutPromise
{
public:
	using Clock = Clock_;
	using time_point = typename Clock::time_point;

	TimeoutPromise(time_point t)
		: m_awaiter(t)
	{}

	// Promise interface
	auto initial_suspend()
	{
		return std::suspend_always{};
	}
	auto final_suspend() { return std::suspend_never{}; }
	void return_value() {}
	AwaitableTimeout<Clock_> get_return_object() { return m_awaiter; }

private:
	AwaitableTimeout<Clock_> m_awaiter;
};

// suspend execution until the given time point is reached
template<class Clock_>
AwaitableTimeout<Clock_> asyncTimeout(std::chrono::time_point<Clock_> t);

Task driveStepper()
{
	constexpr auto dt = std::chrono::microseconds(1000);
	auto tNext = std::chrono::steady_clock::now();
	for(;;)
	{
		tNext += dt;
		co_await asyncTimeout(tNext);
	}
}
// Async code to drive a stepper
// dt = getMotorDt();
// co_await asyncTimeout(lastTime + dt);
// stepPulse();

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
// 		SetupMotors();
//		res = RunDiagnostics();
//		if(res)
		// stepperX.run();
		// stepperY.run();
		// ...
// }

void setup() {
	// Setup scheduler
	// Setup serial port
	Serial.begin(115200);
	// Start motor control
	// runSteppers();
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

#ifdef _WIN32

int main()
{
	setup();
	for(;;)
		loop();
	return 0;
}

#endif // WIN32