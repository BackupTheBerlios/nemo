//------------------------------------------------------------------------------
//	MessageUtils.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <string.h>

// System Includes -------------------------------------------------------------
#include <ByteOrder.h>

// Project Includes ------------------------------------------------------------
#include <MessageUtils.h>

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//------------------------------------------------------------------------------
uint32 _checksum_(const uchar* buf, int32 size)
{
	uint32 sum = 0;
	uint32 temp = 0;

	while (size > 3) {
#if defined(__INTEL__)
		sum += B_SWAP_INT32(*(int*)buf);
#else
		sum += *(int*)buf;
#endif

		buf += 4;
		size -= 4;
	}

	while (size > 0) {
		temp = (temp << 8) + *buf++;
		size -= 1;
		sum += temp;
	}

	return sum;
}
//------------------------------------------------------------------------------


namespace BPrivate {	// Only putting these here because Be did
//------------------------------------------------------------------------------
status_t entry_ref_flatten(char* buffer, size_t* size, const entry_ref* ref)
{
/*	memcpy((void*)buffer, (const void*)&ref->device, sizeof (ref->device));
	buffer += sizeof (ref->device);
	memcpy((void*)buffer, (const void*)&ref->directory, sizeof (ref->directory));
	buffer += sizeof (ref->directory);

	size_t len = 0;
	if (ref->name)
	{
		len = strlen(ref->name) + 1;	// extra for NULL terminator
		memcpy((void*)buffer, (const void*)ref->name, len);
	}

	*size = sizeof (ref->device) + sizeof (ref->directory) + len;
	return B_OK;
*/
	/*	TODO: implement correctly
	*/
	return B_OK;
}
//------------------------------------------------------------------------------
status_t entry_ref_unflatten(entry_ref* ref, const char* buffer, size_t size)
{
/*	if (size < (sizeof (ref->device) + sizeof (ref->directory)))
	{
		*ref = entry_ref();
		return B_BAD_VALUE;
	}

	memcpy((void*)&ref->device, (const void*)buffer, sizeof (ref->device));
	buffer += sizeof (ref->device);
	memcpy((void*)&ref->directory, (const void*)buffer,
		   sizeof (ref->directory));
	buffer += sizeof (ref->directory);
	
	if (ref->device != -1 &&
		size > (sizeof (ref->device) + sizeof (ref->directory)))
	{
		ref->set_name(buffer);
		if (ref->name == NULL)
		{
			*ref = entry_ref();
			return B_NO_MEMORY;
		}
	}
	else
	{
		ref->set_name(NULL);
	}
	
	return B_OK;*/

	/*	TODO: implement correctly
	*/
	return B_OK;
}
//------------------------------------------------------------------------------
status_t entry_ref_swap(char* buffer, size_t size)
{
/*	if (size < (sizeof (dev_t) + sizeof (ino_t)))
	{
		return B_BAD_DATA;
	}

	dev_t* dev = (dev_t*)buffer;
	*dev = B_SWAP_INT32(*dev);
	buffer += sizeof (dev_t);

	ino_t* ino = (ino_t*)buffer;
	*ino = B_SWAP_INT64(*ino);

	return B_OK;*/
	
	/*	TODO: implement correctly
	*/
	return B_OK;	
}
//------------------------------------------------------------------------------

}	// namespace BPrivate

//------------------------------------------------------------------------------
int32 TChecksumHelper::CheckSum()
{
	 return _checksum_(fBuffer, fBufPtr - fBuffer);
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */
