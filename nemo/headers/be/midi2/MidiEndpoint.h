
#ifndef _MIDI_ENDPOINT_H
#define _MIDI_ENDPOINT_H

#include <BeBuild.h>
#include <Midi2Defs.h>
#include <OS.h>
#include <String.h>

class BMidiProducer;
class BMidiConsumer;

class BMidiEndpoint 
{
public:

	const char* Name() const;
	void SetName(const char* name);
	
	int32 ID() const;

	bool IsProducer() const;
	bool IsConsumer() const;
	bool IsRemote() const;
	bool IsLocal() const;
	bool IsPersistent() const;
	bool IsValid() const;

	status_t Release();
	status_t Acquire();

	status_t SetProperties(const BMessage* properties);
	status_t GetProperties(BMessage* properties) const;

	status_t Register(void);
	status_t Unregister(void);
						
private:

	friend class BMidiConsumer;
	friend class BMidiLocalConsumer;
	friend class BMidiLocalProducer;
	friend class BMidiProducer;
	friend class BMidiRoster;
	friend class BMidiRosterLooper;
	
	BMidiEndpoint(const char* name);
	virtual	~BMidiEndpoint();

	virtual void _Reserved1();
	virtual void _Reserved2();
	virtual void _Reserved3();
	virtual void _Reserved4();
	virtual void _Reserved5();
	virtual void _Reserved6();
	virtual void _Reserved7();
	virtual void _Reserved8();
	
	status_t SendRegisterRequest(bool);
	status_t SendChangeRequest(BMessage*);

	bool IsRegistered() const;
	bool LockLooper() const;
	void UnlockLooper() const;

	int32 id;
	BString name;	
	int32 refCount;
	BMessage* properties;
	bool isLocal;
	bool isConsumer;
	bool isRegistered;
	bool isAlive;

	uint32 _reserved[4];
};

#endif // _MIDI_ENDPOINT_H
