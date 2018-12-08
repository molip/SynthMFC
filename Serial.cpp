#include "stdafx.h"
#include "Serial.h"

#include "synth/libKernel/Debug.h"

#include <algorithm>
#include <thread>

namespace
{
	const int UsbSpeed = 57600;
}

SerialPort::SerialPort() : _file(nullptr)
{
}

SerialPort::~SerialPort()
{
	_abort = true;
	
	if (_thread.joinable())
		_thread.join();

	Close();
}

std::wstring SerialPort::FindPortName() const
{
	return L"COM7";

	//WCHAR devs[1 << 16];
	//DWORD chars = ::QueryDosDevice(nullptr, devs, 1 << 16);

	//for (const WCHAR* p = devs; *p;)
	//{
	//	std::wstring s = p;
	//	if (s.substr(0, 3) == L"COM")
	//		return s;
	//	p += s.length() + 1;
	//}
	//return std::wstring();
}

bool SerialPort::Open()
{
	if (_file)
		return true;

	std::wstring path = L"\\\\.\\";
	std::wstring portName = _portName.empty() ? FindPortName() : _portName;
	path += portName;
	
	// FILE_FLAG_OVERLAPPED means reading doesn't block writing. 
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
	if (!file || file == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	DCB dcb;
	::memset(&dcb, 0, sizeof dcb);
	dcb.fBinary = true;
	dcb.BaudRate = UsbSpeed;
	dcb.ByteSize = 8;
	if (!SetCommState(file, &dcb))
	{
		AfxMessageBox(L"SetCommState failed");
		return false;
	}

	COMMTIMEOUTS timeouts;
	if (!GetCommTimeouts(file, &timeouts))
	{
		AfxMessageBox(L"GetCommTimeouts failed");
		return false;
	}

	// No timeout - reading blocks until data available.
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts(file, &timeouts))
	{
		AfxMessageBox(L"SetCommTimeouts failed");
		return false;
	}

	_file = file;
	_portName = portName;

	if (_thread.joinable())
		_thread.join();

	_thread = std::thread(&SerialPort::Go, this);

	return true;
}

bool SerialPort::Close()
{
	if (!_file)
		return false;
	
	if (!CloseHandle(_file))
	{
		AfxMessageBox(L"CloseHandle failed");
		return false;
	}

	_file = nullptr;
	return true;
}

void SerialPort::Go()
{
	while (!_abort)
	{

		const int BufferSize = 1;
		char buffer[BufferSize];
		DWORD bytesRead = 0;
		OVERLAPPED overlapped{}; // Needed for FILE_FLAG_OVERLAPPED mode.
		if (!ReadFile(_file, buffer, BufferSize, &bytesRead, &overlapped))
			if (!GetOverlappedResult(_file, &overlapped, &bytesRead, true))
			{	
				int a = 0;
				_file = nullptr;
				return;
			}

		if (bytesRead)
		{
			std::lock_guard<std::mutex> lk(_mutex);
			_input.append(buffer, bytesRead);
		}
	}
}

std::string SerialPort::HarvestInput()
{
	std::string result;
	if (!_input.empty())
	{
		std::lock_guard<std::mutex> lk(_mutex);
		result = std::move(_input);
	}
	return result;
}

bool SerialPort::Write(const byte* data, DWORD bytes) 
{
	DWORD bytesRead = 0;
	OVERLAPPED overlapped{}; // Needed for FILE_FLAG_OVERLAPPED mode.
	WriteFile(_file, data, bytes, &bytesRead, &overlapped);
	return bytesRead == bytes;
}

