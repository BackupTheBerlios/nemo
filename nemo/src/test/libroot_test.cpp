// Standard Includes -----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// System Includes -------------------------------------------------------------
#include <OS.h>

// Macros ----------------------------------------------------------------------

// Types -----------------------------------------------------------------------
struct msg_t {
	int32	cmd;
	char 	text[32];
};

// Globals ---------------------------------------------------------------------

// Function Prototypes ---------------------------------------------------------
void test_port(void);
void test_sem(void);
void test_thread(void);
void test_team(void);
int32 func1(void *);
int32 func2(void *);
int32 func3(void *);

// Main ------------------------------------------------------------------------
int main(void) {

/*	test_port();
	test_sem();
	test_thread();
	test_team();*/
	while(true) {
		snooze(500000);
	}

	return 0;
}

// Functions -------------------------------------------------------------------
void test_port(void) {

	printf("Testing ports:\n");
	printf("==============\n");

	port_id p = create_port(100, "test port");
	if(p >= B_OK) printf("port created successfully\n");

	printf("port id of port \"test port\" is %d\n", find_port("test port"));

	msg_t msg = {1, "ya rab"};
	if(write_port(p, msg.cmd, msg.text, strlen(msg.text)) == B_OK)
		printf("message written to port successfully\n");

	printf("port count is now %d\n", port_count(p));
	
	size_t size = port_buffer_size(p);
	printf("port buffer size = %d\n", size);

	if(read_port(p, &(msg.cmd), msg.text, 32) == strlen(msg.text))
		printf("message read successfully\n");

	printf("port count is now %d\n", port_count(p));

	if(close_port(p) == B_OK)
		printf("port closed\n");

	if(write_port(p, msg.cmd, msg.text, strlen(msg.text)) != B_OK)
		printf("couldn't write to port while it's closed\n");

	printf("port count is now %d\n", port_count(p));

	port_info info;
	if(get_port_info(p, &info) == B_OK) {
		printf("port info acquired: \n");
		printf("\tport = %d\n", info.port);
		printf("\tteam = %d\n", info.team);
		printf("\tname = %s\n", info.name);
		printf("\tcapacity = %d\n", info.capacity);
		printf("\tqueue count = %d\n", info.queue_count);
		printf("\ttotal count = %d\n", info.total_count);
	}
	
	if(delete_port(p) == B_OK) printf("port %d deleted successfully\n", p);

	printf("\n\n");
}
//------------------------------------------------------------------------------
void test_sem(void) {

	printf("Testing semaphores:\n");
	printf("===================\n");

	sem_id s = create_sem(1, "test sem");
	if(s >= B_OK) printf("semaphore created successfully\n");

	int32 count;
	if(get_sem_count(s, &count) == B_OK)
		printf("count is now %d\n", count);
		
	if(acquire_sem(s) == B_OK)
		printf("semaphore %d acquired successfully\n", s);

	if(acquire_sem_etc(s, 1, B_RELATIVE_TIMEOUT, 0) == B_WOULD_BLOCK)
		printf("couldn't acquire semaphore %d while it's already acquired\n", s);
		
	if(get_sem_count(s, &count) == B_OK)
		printf("count is now %d\n", count);

	if(release_sem(s) == B_OK)
		printf("semaphore %d released successfully\n", s);

	if(get_sem_count(s, &count) == B_OK)
		printf("count is now %d\n", count);

	sem_info info;
	if(get_sem_info(s, &info) == B_OK) {
		printf("semaphore info acquired: \n");
		printf("\tsem = %d\n", info.sem);
		printf("\tteam = %d\n", info.team);
		printf("\tname = %s\n", info.name);
		printf("\tcount = %d\n", info.count);
	}
	
	if(delete_sem(s) == B_OK) printf("semaphore %d deleted successfully\n", s);

	printf("\n\n");
}
//------------------------------------------------------------------------------
void test_thread(void) {

	printf("Testing threads:\n");
	printf("================\n");

	printf("thread %d spawning a new thread\n", find_thread(NULL));
	thread_id th1 = spawn_thread(func1, "thread 1", B_NORMAL_PRIORITY, NULL);

	printf("thread %d resuming thread %d\n", find_thread(NULL), th1);
	status_t error = resume_thread(th1);
	switch(error) {
		case B_OK: printf("func1() resumed\n"); break;
		case B_BAD_THREAD_STATE: printf("bad thread state\n"); break;
		case B_BAD_THREAD_ID: printf("bad thread id\n"); break;
	}

	printf("sleeping for 2 seconds\n");
	snooze(2000000);
	printf("woke up\n");
	
	/*if(kill_thread(th1) == B_OK)
		printf("func1() killed\n");

	printf("thread %d joining thread %d\n", find_thread(NULL), th1);
	wait_for_thread(th1, &error);
	printf("thread %d returned %d\n", th1, error);*/
	
	printf("\n\n");
}
//------------------------------------------------------------------------------
void test_team(void) {

	printf("Testing teams:\n");
	printf("==============\n");

	printf("committing suicide...\n");
	kill_team(getpid());

	printf("\n\n");
}
//------------------------------------------------------------------------------
int32 func1(void *arg) {

	while(true) {
		printf("in func1()\n");
		snooze(500000);
	}
}
//------------------------------------------------------------------------------
int32 func2(void *arg) {

	while(1) {
		printf("in func2()\n");
		snooze(500000);
	}
}
//------------------------------------------------------------------------------
int32 func3(void *arg) {

	while(1) {
		printf("in func3()\n");
		snooze(500000);
	}
}
//------------------------------------------------------------------------------
