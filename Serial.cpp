#include "stdafx.h"
#include "Serial.h"

#include <algorithm>

namespace
{
	const int UsbSpeed = 57600;
}

SerialPort::SerialPort() : _file(nullptr)
{
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
		return false;

	std::wstring path = L"\\\\.\\";
	std::wstring portName = _portName.empty() ? FindPortName() : _portName;
	path += portName;
		
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
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

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts(file, &timeouts))
	{
		AfxMessageBox(L"SetCommTimeouts failed");
		return false;
	}

	_file = file;
	_portName = portName;

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

int SerialPort::Read()
{
	DWORD bytesRead = 0;
	ReadFile(_file, _buffer, BufferSize, &bytesRead, nullptr);
	return bytesRead;
}

bool SerialPort::Write(const byte* data, DWORD bytes) 
{
	DWORD bytesRead = 0;
	WriteFile(_file, data, bytes, &bytesRead, nullptr);
	return bytesRead == bytes;
}

