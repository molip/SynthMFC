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
	int Read();

	const byte* GetBuffer() const { return _buffer; }

private:
	std::wstring FindPortName() const;

	HANDLE _file;
	std::wstring _portName;

	static const int BufferSize = 1024;
	byte _buffer[BufferSize];
};
