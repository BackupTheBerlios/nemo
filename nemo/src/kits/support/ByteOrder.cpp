// Quick and dirty implementation of the byte order functions.
// Just here to be able to compile and test BFile.
// To be replaced by the OpenBeOS version to be provided by the IK Team.

#include <ByteOrder.h>

#include "LibBeAdapter.h"

// swap_data
status_t
swap_data(type_code type, void *data, size_t length, swap_action action)
{
	/*	TODO: implement correctly;
		the original implementation calls swap_data_adapter(), which
		in turn calls this function again, causing a stack overflow
	*/
	//return swap_data_adapter(type, data, length, action);
	return B_OK;
}

// is_type_swapped
bool
is_type_swapped(type_code type)
{
	/*	TODO: implement correctly;
		the original implementation calls is_type_swapped_adapter(), which
		in turn calls this function again, causing a stack overflow
	*/
	//return is_type_swapped_adapter(type);
	return true;
}
