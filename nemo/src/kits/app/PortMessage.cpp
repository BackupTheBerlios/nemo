//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		PortMessage.cpp
//	Author:			DarkWyrm <bpmagic@columbus.rr.com>
//	Description:	Package class for port messaging-based data
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <string.h>

// System Includes -------------------------------------------------------------

// Private Includes ------------------------------------------------------------
#include "PortMessage.h"
/*
//------------------------------------------------------------------------------
PortMessage::PortMessage(int32 code, const void *buffer, ssize_t buffersize,
	bool copy)
{
	_code = code;
	_protocol = AS_SERVER_PORTLINK;
	_buffersize = buffersize;

	if(buffer) {
    	if(copy)
    	{
    		_buffer = new uint8[_buffersize];
    		memcpy(_buffer, buffer, _buffersize);
    		_delete_buf = true;
    	}
    	else
    	{
    		_buffer = (uint8*)buffer;
    		_delete_buf = false;
    	}
	}
	else {
		_buffer = new uint8[_buffersize];
		_delete_buf = true;
	}
	
	_index = _buffer + 8;
}
//------------------------------------------------------------------------------
PortMessage::PortMessage(ssize_t size)
{
	if(size < 8)
		size = 8;
		
	_code = 0;
	_protocol = AS_SERVER_PORTLINK;
	_buffer = new uint8[size];
	_buffersize = size;
	_delete_buf = true;
	_index = NULL;
}
//------------------------------------------------------------------------------
PortMessage::~PortMessage(void)
{
	if(_delete_buf)
		delete[] _buffer;
}
//------------------------------------------------------------------------------
status_t PortMessage::ReadFromPort(port_id port, bigtime_t timeout)
{
	if(timeout == B_INFINITE_TIMEOUT)
	{
		_buffersize = read_port(port, &_protocol, _buffer, _buffersize);
	}
	else
	{
		_buffersize = read_port_etc(port, &_protocol, _buffer,
			_buffersize, B_TIMEOUT, timeout);
	}

	// error?
	if((status_t)_buffersize < B_OK)
		return (status_t)_buffersize;

	_code = ((int32*)_buffer)[0];
	_index = _buffer + 8;

	return B_OK;
}
//------------------------------------------------------------------------------
status_t PortMessage::WriteToPort(port_id port)
{
	// Check port validity
	port_info pi;
	if(get_port_info(port, &pi) != B_OK)
		return B_BAD_VALUE;

	((int32*)_buffer)[0] = _code;

	status_t error =
		write_port(port, AS_SERVER_PORTLINK, _buffer, _buffersize);

	// error?
	if(error < B_OK)
		return error;

	// reset buffer pointer
	_buffersize = 0;
	_index = NULL;
	
	return B_OK;
}
//------------------------------------------------------------------------------
void PortMessage::SetCode(int32 code)
{
	_code = code;
}
//------------------------------------------------------------------------------
status_t PortMessage::Read(void *data, ssize_t size)
{
	if(!data || size < 1)
		return B_BAD_VALUE;

	if(!_buffer || _buffersize < size ||
		_index + size > _buffer + _buffersize)
		return B_NO_MEMORY;

	memcpy(data, _index, size);
	_index += size;

	return B_OK;
}
//------------------------------------------------------------------------------
void PortMessage::Rewind(void)
{
	_index = _buffer + 8;
}
//------------------------------------------------------------------------------
status_t PortMessage::ReadString(char **string)
{
	int16 len = 0;

	if(Read<int16>(&len) != B_OK)
		return B_ERROR;

	if (len)
	{
		*string = new char[len];
		if(Read(*string, len) != B_OK)
		{
			delete[] *string;
			*string = NULL;
		}
	}

	return B_OK;
}
//------------------------------------------------------------------------------
*/
