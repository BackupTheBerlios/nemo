#include <Entry.h>
#include <Roster.h>


#include <RegistrarDefs.h>



#include <new>
#include <string.h>

using namespace std;


//! Creates an unitialized entry_ref. 
entry_ref::entry_ref()
		 : device((dev_t)-1),
		   directory((ino_t)-1),
		   name(NULL)
{
}


entry_ref::entry_ref(dev_t dev, ino_t dir, const char *name)
		 : device(dev), directory(dir), name(NULL)
{
	set_name(name);
}

/*! \brief Creates a copy of the given entry_ref.

	\param ref a reference to an entry_ref to copy
*/
entry_ref::entry_ref(const entry_ref &ref)
		 : device(ref.device),
		   directory(ref.directory),
		   name(NULL)
{
	set_name(ref.name);	
}

//! Destroys the object and frees the storage allocated for the leaf name, if necessary. 
entry_ref::~entry_ref()
{
	if (name != NULL)
		delete [] name;
}

/*! \brief Set the entry_ref's leaf name, freeing the storage allocated for any previous
	name and then making a copy of the new name.
	
	\param name pointer to a null-terminated string containing the new name for
	the entry. May be \c NULL.
*/
status_t entry_ref::set_name(const char *name)
{
	if (this->name != NULL) {
		delete [] this->name;
	}
	
	if (name == NULL) {
		this->name = NULL;
	} else {
		this->name = new(nothrow) char[strlen(name)+1];
		if (this->name == NULL)
			return B_NO_MEMORY;
		strcpy(this->name, name);
	}
	
	return B_OK;			
}

/*! \brief Compares the entry_ref with another entry_ref, returning true if they are equal.
	\return
	- \c true - The entry_refs are equal
	- \c false - The entry_refs are not equal
*/
bool
entry_ref::operator==(const entry_ref &ref) const
{
	return (device == ref.device
			&& directory == ref.directory
			&& (name == ref.name
				|| name != NULL && ref.name != NULL
					&& strcmp(name, ref.name) == 0));
}

/*! \brief Compares the entry_ref with another entry_ref, returning true if they are not equal.
	\return
	- \c true - The entry_refs are not equal
	- \c false - The entry_refs are equal
*/
bool
entry_ref::operator!=(const entry_ref &ref) const
{
	return !(*this == ref);
}

/*! \brief Makes the entry_ref a copy of the entry_ref specified by \a ref.
	\param ref the entry_ref to copy
	\return
	- A reference to the copy
*/
entry_ref&
entry_ref::operator=(const entry_ref &ref)
{
	if (this == &ref)
		return *this;	

	device = ref.device;
	directory = ref.directory;
	set_name(ref.name);
	return *this;
}


app_info::app_info()
		: thread(-1),
		  team(-1),
		  port(-1),
		  flags(B_REG_DEFAULT_APP_FLAGS),
		  ref()
{
	signature[0] = '\0';
}

/*!	\brief Does nothing.
*/
app_info::~app_info()
{
}
