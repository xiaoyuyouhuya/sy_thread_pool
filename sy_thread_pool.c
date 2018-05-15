#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sy_thread_pool.h"

sy_thread_pool *pool;


void* sy_thread_pool_entrance(void* arg){
	int thread_id = (int) arg;
	printf("thread %d is created!\n", thread_id);
	while(1){
		pthread_mutex_lock(&(pool->queue_mutex));
		while(pool->task_num == 0 && !pool->is_distoried){		//while循环判断 防止假唤醒
			pthread_cond_wait(&(pool->queue_cond), &(pool->queue_mutex));
		}

		if(pool->is_distoried){						//检查标志位 若销毁标志位为1 解锁 退出
			printf("thread %d exited!\n", thread_id);
			pthread_mutex_unlock(&(pool->queue_mutex));
			pthread_exit(NULL);
		}

		pool->idle_thread_num--;
		task *work;
		work = pool->task_queue_head;
		pool->task_queue_head = pool->task_queue_head->next;
		if(pool->task_queue_head == NULL){
			pool->task_queue_tail = NULL;
		}
		pool->task_num--;
		pthread_mutex_unlock(&(pool->queue_mutex));

		(*(work->taskfunc))(work->arg);
		pool->idle_thread_num++;

	}
}


typedef void *(*taskfunc)(void* arg);

int sy_add_task(taskfunc func, void* arg){
	task* input_task;
	input_task = (task*)malloc(sizeof(task));
	input_task->taskfunc = func;
	input_task->arg = arg;
	input_task->next = NULL;
	pthread_mutex_lock(&(pool->queue_mutex));
	if(pool->task_queue_head == NULL){
		pool->task_queue_tail = input_task;
		pool->task_queue_head = pool->task_queue_tail;
	}
	else{
		pool->task_queue_tail->next = input_task;
		pool->task_queue_tail = pool->task_queue_tail->next;
	}
	pool->task_num++;
	pthread_cond_signal(&(pool->queue_cond));
	pthread_mutex_unlock(&(pool->queue_mutex));
	
	return 0;
}


void* sy_thread_pool_init(int thread_pool_size){
	pool = (sy_thread_pool*)malloc(sizeof(sy_thread_pool));

	pool->task_queue_head = NULL;
	pool->task_queue_tail = NULL;
	pool->task_num = 0;

	pool->thread_num = thread_pool_size;
	pool->idle_thread_num = thread_pool_size;

	pool->is_distoried = 0;

	pool->thread_queue = (pthread_t*)malloc(thread_pool_size * sizeof(pthread_t));


	int i, ret;
	for(i = 0; i < thread_pool_size; i++){
		ret = pthread_create(&(pool->thread_queue[i]), NULL, sy_thread_pool_entrance, (void*)i);
		if(ret == -1){
			fprintf(stderr, "create error!\n");
			sy_thread_pool_destroy();
			return -1;
		}
	}

	pthread_mutex_init(&(pool->queue_mutex), NULL);
	pthread_cond_init(&(pool->queue_cond), NULL);
}


int sy_thread_pool_destroy(){
	if(pool->is_distoried){
		return -1;	
	}
	int retc;

	pthread_mutex_lock(&(pool->queue_mutex));		//加锁将销毁标志位置为1
	pool->is_distoried = 1;
	retc = pthread_cond_broadcast(&(pool->queue_cond));
	pthread_mutex_unlock(&(pool->queue_mutex));
	
	int i;
	int ret[pool->thread_num];
	for(i = 0; i < pool->thread_num; i++){
		ret[i] = pthread_join(pool->thread_queue[i], NULL);
		if(ret[i] != 0)  
		{  
			/*if(ret[i] == ESRCH)  
			{  
				printf("pthread_join():ESRCH 没有找到与给定线程ID相对应的线程\n");  
			}  
			else if(ret[i] == EDEADLK)  
			{  
				printf("pthread_join():EDEADLKI 产生死锁\n");  
			}  
			else if(ret[i] == EINVAL)  
			{  
				printf("pthread_join():EINVAL 与给定的县城ID相对应的线程是分离线程\n");  
			}  
			else  
			{ */ 
				printf("pthread_join():unknow error\n");  
			//}  
			return -1;  
		}
	}

	//销毁任务队列
	task* t;
	while(pool->task_queue_head){
		t = pool->task_queue_head;
		pool->task_queue_head = pool->task_queue_head->next;
		free(t);
	}
	pool->task_queue_head = NULL;
	pool->task_queue_tail = NULL;
	
	//销毁线程队列
	free(pool->thread_queue);
	retc = pthread_mutex_destroy(&(pool->queue_mutex));
	if(!retc){
		fprintf(stderr, "mutex destroy error!\n");
		return -1;
	}
	retc = pthread_cond_destroy(&(pool->queue_cond));
	if(!retc){
		fprintf(stderr, "cond destroy error!\n");
		return -1;
	}
	pool = NULL;
	return 1;
}
