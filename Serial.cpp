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
	_readEvent = ::CreateEvent(nullptr, false, false, L"SerialPortReadEvent");
	_writeEvent = ::CreateEvent(nullptr, false, false, L"SerialPortWriteEvent");
}

SerialPort::~SerialPort()
{
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

	_readThread = std::thread(&SerialPort::ReadThread, this);
	_writeThread = std::thread(&SerialPort::WriteThread, this);

	return true;
}

bool SerialPort::Close()
{
	{
		std::unique_lock<std::mutex> lock(_writeMutex);
		_abort = true;
	}
	
	_writeCV.notify_one();

	if (_readThread.joinable())
		_readThread.join();

	if (_writeThread.joinable())
		_writeThread.join();

	if (!_file)
		return false;

	if (!CloseHandle(_file))
	{
		AfxMessageBox(L"CloseHandle failed");
		return false;
	}

	_file = nullptr;
	_abort = false;

	return true;
}

void SerialPort::ReadThread()
{
	while (!_abort)
	{
		const int BufferSize = 1;
		char buffer[BufferSize]{};
		DWORD bytesRead = 0;
		OVERLAPPED overlapped{}; // Needed for FILE_FLAG_OVERLAPPED mode.
		overlapped.hEvent = _readEvent;
		if (!ReadFile(_file, buffer, BufferSize, &bytesRead, &overlapped))
		{
			if (!GetOverlappedResult(_file, &overlapped, &bytesRead, true))
			{
				Kernel::Debug::Trace << "[SerialPort::WriteThread] Failed to read data, closing port" << std::endl;
				_abort = true;
				return;
			}
		}

		KERNEL_ASSERT(bytesRead == 1);

		{
			std::lock_guard<std::mutex> lk(_readMutex);
			_input.append(buffer, bytesRead);
		}
	}
}

void SerialPort::WriteThread()
{
	while (!_abort)
	{
		std::unique_ptr<Buffer> packet;
		
		{
			std::unique_lock<std::mutex> lock(_writeMutex);
			_writeCV.wait(lock, [&]() { return !_writeQueue.empty() || _abort; });

			if (_abort)
				return;

			packet = std::move(_writeQueue.front());
			_writeQueue.pop_front();
		}
		
		int i = 0;
		DWORD bytesWritten = 0;
		OVERLAPPED overlapped{};
		overlapped.hEvent = _writeEvent;
		if (!WriteFile(_file, packet->data(), static_cast<DWORD>( packet->size()), &bytesWritten, &overlapped))
		{
			i = ::GetLastError();
			if (!GetOverlappedResult(_file, &overlapped, &bytesWritten, true))
			{
				Kernel::Debug::Trace << "[SerialPort::WriteThread] Failed to write " << packet->size() << "bytes, closing port" << std::endl;
				_abort = true;
				return;
			}
		}

		KERNEL_ASSERT(bytesWritten == packet->size());
	}
}

bool SerialPort::CloseIfAborted()
{
	if (!_abort)
		return false;

	Close();
	return true;
}

std::string SerialPort::HarvestInput()
{
	if (CloseIfAborted())
		return {};

	std::string result;
	if (!_input.empty())
	{
		std::lock_guard<std::mutex> lk(_readMutex);
		result = std::move(_input);
	}
	return result;
}

bool SerialPort::Write(const Buffer& buffer)
{
	if (!_file)
		return false;

	if (CloseIfAborted())
		return false;

	{
		std::unique_lock<std::mutex> lock(_writeMutex);
		_writeQueue.push_back(std::make_unique<Buffer>(buffer));
	}
	
	_writeCV.notify_one();
	return true;
}

