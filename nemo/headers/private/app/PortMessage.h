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
//	File Name:		PortMessage.h
//	Author:			DarkWyrm <bpmagic@columbus.rr.com>
//	Description:	Package class for port messaging-based data
//  
//------------------------------------------------------------------------------
#ifndef _PORTMESSAGE_H
#define _PORTMESSAGE_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <OS.h>

// Private Includes ------------------------------------------------------------
#include "ServerProtocol.h"

// Macro Defs ------------------------------------------------------------------
#define DFLT_PORT_MSG_SIZE	4096

//------------------------------------------------------------------------------
class PortMessage {

public:
//------------------------------------------------------------------------------
	PortMessage(int32 code, const void *buffer,	ssize_t buffersize, bool copy)
    {
		fCode = code;
		fProtocol = AS_SERVER_PORTLINK;
		fBufferSize = buffersize;
		fSizeRead = fSizeWritten = 0;

    	if(buffer) {
        	if(copy)
        	{
        		fBuffer = new uint8[fBufferSize];
        		memcpy(fBuffer, buffer, fBufferSize);
        		fDeleteBuffer = true;
        	}
        	else
        	{
        		fBuffer = (uint8*)buffer;
        		fDeleteBuffer = false;
        	}
    	}
    	else {
    		fBuffer = new uint8[fBufferSize];
    		fDeleteBuffer = true;
    	}

    	fReadPosition = fWritePosition = fBuffer + 8;
    }							
//------------------------------------------------------------------------------							
	PortMessage(ssize_t size = DFLT_PORT_MSG_SIZE)
    {
		if(size < 8)
			size = 8;

		fCode = 0;
		fProtocol = AS_SERVER_PORTLINK;
		fBuffer = new uint8[size];
		fBufferSize = size;
		fSizeRead = fSizeWritten = 0;
		fDeleteBuffer = true;
		fReadPosition = fWritePosition = fBuffer + 8;
    }						
//------------------------------------------------------------------------------						
	~PortMessage(void)
    {
    	if(fDeleteBuffer)
    		delete[] fBuffer;
    }
//------------------------------------------------------------------------------						
	status_t ReadFromPort(port_id port,	bigtime_t timeout = B_INFINITE_TIMEOUT)
    {
		if(timeout == B_INFINITE_TIMEOUT)
		{
			fSizeWritten = read_port(port, &fProtocol, fBuffer, fBufferSize);
		}
		else
		{
			fSizeWritten = read_port_etc(port, &fProtocol, fBuffer,
				fBufferSize, B_TIMEOUT, timeout);
		}

		// error?
		if((status_t)fSizeWritten < B_OK)
			return (status_t)fSizeWritten;

		fCode = ((int32*)fBuffer)[0];
		fSizeRead = 0;
		fSizeWritten -= 8;
		fReadPosition = fBuffer + 8;
		fWritePosition = fBuffer + 8 + fSizeWritten;

    	return B_OK;
    }							
//------------------------------------------------------------------------------							
	status_t WriteToPort(port_id port)
	{
		// Check port validity
		port_info pi;
		if(get_port_info(port, &pi) != B_OK)
			return B_BAD_VALUE;

		((int32*)fBuffer)[0] = fCode;
		((int32*)fBuffer)[1] = fSizeWritten;

		status_t error =
			write_port(port, AS_SERVER_PORTLINK, fBuffer, fSizeWritten + 8);
		
		return error;
    }		
//------------------------------------------------------------------------------	
    void SetCode(int32 code)
    {
    	fCode = code;
    }     
//------------------------------------------------------------------------------     
   	void SetProtocol(int32 protocol)
    {
		fProtocol = protocol;
	}
//------------------------------------------------------------------------------
    int32 Code(void)
    {
		return fCode;
	}
//------------------------------------------------------------------------------     
    void* Buffer(void)
    {
		return fBuffer;
	}
//------------------------------------------------------------------------------     
    ssize_t BufferSize(void)
	{
		return fBufferSize;
	}
//------------------------------------------------------------------------------     
    int32 Protocol(void) const
    {
		return fProtocol;
	}
//------------------------------------------------------------------------------     
    void Reset(void)
    {
		fSizeRead = 0;
		fSizeWritten = 0;
		fReadPosition = fBuffer + 8;
		fWritePosition = fBuffer + 8;
    }
//------------------------------------------------------------------------------      
    status_t Read(void *data, ssize_t size)
    {
		if(!data || size < 1)
			return B_BAD_VALUE;

		if(!fBuffer ||
			fBufferSize < size ||
			fReadPosition + size > fBuffer + fBufferSize)
    		return B_NO_MEMORY;

    	memcpy(data, fReadPosition, size);
    	fReadPosition += size;

    	return B_OK;
    }
//------------------------------------------------------------------------------     
   	template <class Type>
    status_t Read(Type *data)
	{
		int32 size = sizeof(Type);

		if(!data)
			return B_BAD_VALUE;

		if(!fBuffer ||
			fBufferSize < size ||
   			fReadPosition + size > fBuffer + fBufferSize)
			return B_NO_MEMORY;

   		*data = *((Type*)fReadPosition);
   		fReadPosition += size;

   		return B_OK;
	}
//------------------------------------------------------------------------------     
    status_t ReadString(char **string)
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
	status_t Attach(const void *data, size_t size)
    {
		if (size <= 0)
			return B_ERROR;

		if(!fBuffer ||
			fBufferSize < size ||
   			fWritePosition + size > fBuffer + fBufferSize)
			return B_NO_MEMORY;

   		memcpy(fWritePosition, data, size);
   		fWritePosition += size;
   		fSizeWritten += size;
     
   		return B_OK;
    }
//------------------------------------------------------------------------------
	template <class Type>
	status_t Attach(Type data)
	{
		int32 size	= sizeof(Type);

    	if(!fBuffer ||
    		fBufferSize < size ||
   			fWritePosition + size > fBuffer + fBufferSize)
    		return B_NO_MEMORY;
			
		memcpy(fWritePosition, &data, size);
		fWritePosition += size;
		fSizeWritten += size;
		
		return B_OK;
   	}    
//------------------------------------------------------------------------------
    status_t AttachString(const char *string)
    {
    	int16 len = (int16)strlen(string) + 1;

    	Attach<int16>(len);
    	return Attach(string, len);
    }
//------------------------------------------------------------------------------
private:
	int32		fCode;
   	uint8		*fBuffer;
   	ssize_t		fBufferSize;
    ssize_t		fSizeRead;
    ssize_t		fSizeWritten;
    uint8		*fReadPosition;
    uint8		*fWritePosition;
   	int32 		fProtocol;
	bool		fDeleteBuffer;
};
//------------------------------------------------------------------------------
#endif
