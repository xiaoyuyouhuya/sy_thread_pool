#include <stdio.h>
#include "sy_thread_pool.h"


void test_task(void* arg){
	pthread_t t_id;
	t_id = pthread_self();
	printf("%dworking!----%d\n", arg, t_id);
}


/*void sy_producer(void){
}*/


int main(int argc, char** argv){
	int thread_pool_size;
	thread_pool_size = 5;
	sy_thread_pool_init(thread_pool_size);

	pthread_t pro_id;
	int ret;
	//ret = pthread_create(&pro_id, NULL, (void *)sy_producer, NULL);
	int k;
	for(k = 0; k < 3; k++){
		int i;
		for(i = 0; i < 7; i++){
			sy_add_task((void *)test_task, (void *)i);
			sleep(1);
		}
		printf("---------------------\n");
		sleep(5);
	}
	/*if(!ret){
		printf("create producer error!\n");
	}*/
	//pthread_join(pro_id,NULL);
	ret = sy_thread_pool_destroy();
	return 0;
}
