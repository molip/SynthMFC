#pragma once

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

#undef min
#undef max

class SerialPort
{
public:
	SerialPort();
	~SerialPort();

	bool Open();
	bool Close();
	bool IsOpen() const { return _file != nullptr; }

	std::wstring GetPortName() const { return _portName; }

	bool Write(const byte* data, DWORD bytes);
	std::string HarvestInput();

private:
	std::wstring FindPortName() const;
	void Go();

	HANDLE _file;
	std::wstring _portName;

	std::string _input;

	std::thread _thread;
	std::mutex _mutex;
	bool _abort = false;
};
