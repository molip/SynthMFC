#pragma once

#include <algorithm>
#include <deque>
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

	using Buffer = std::vector<byte>;

	bool Open();
	bool Close();
	bool IsOpen() const { return _file != nullptr; }

	std::wstring GetPortName() const { return _portName; }

	bool Write(const Buffer& buffer);
	std::string HarvestInput();

private:
	std::wstring FindPortName() const;
	void ReadThread();
	void WriteThread();
	bool CloseIfAborted();

	HANDLE _file;
	std::wstring _portName;

	std::string _input;
	std::deque<std::unique_ptr<Buffer>> _writeQueue;

	std::thread _readThread, _writeThread;
	std::mutex _readMutex, _writeMutex;
	std::condition_variable _writeCV;
	HANDLE _readEvent, _writeEvent;
	bool _abort{};
};
