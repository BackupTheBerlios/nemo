#include <Locker.h>

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


BLocker *locker = NULL;

void* func(void *data) {

	
	printf("thread %d entered func\n", pthread_self());
	if(locker->Lock()) {
		printf("thread %d acquired func\n", pthread_self());
		sleep(1);
		locker->Unlock();
	}
	else {
		printf("thread %d attempted to acquire func but failed\n", pthread_self());
	}
	printf("thread %d exiting func\n", pthread_self());
}

int main( void ) {
	
	locker = new BLocker();
	
	pthread_t t1, t2;
	pthread_create(&t1, NULL, func, NULL);
	pthread_create(&t2, NULL, func, NULL);
	
	void *rv;
	pthread_join(t1, &rv);
	pthread_join(t2, &rv);
	
	delete locker;
	
	return 0;
}

