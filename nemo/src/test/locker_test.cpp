#include <OS.h>
#include <Locker.h>
#include <stdio.h>
#include <unistd.h>

BLocker *locker = NULL;

int32 func(void *data) {

	
	printf("thread %d entered func\n", find_thread(NULL));
	if(locker->Lock()) {
		printf("thread %d acquired func\n", find_thread(NULL));
		sleep(1);
		locker->Unlock();
	}
	else {
		printf("thread %d attempted to acquire func but failed\n", find_thread(NULL));
	}
	printf("thread %d exiting func\n", find_thread(NULL));
}

int main( void ) {
	
	locker = new BLocker();
	
	thread_id t1, t2;
	t1 = spawn_thread(func, "thread 1", B_NORMAL_PRIORITY, NULL);
	t2 = spawn_thread(func, "thread 1", B_NORMAL_PRIORITY, NULL);
		
	status_t rv;
	wait_for_thread(t1, &rv);
	wait_for_thread(t2, &rv);
		
	delete locker;
	
	return 0;
}
