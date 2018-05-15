#ifndef SY_PTHREAD_POOL
#define SY_PTHREAD_POOL

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


typedef struct task{
	void *(*taskfunc)(void *arg);
	void *arg;
	struct task *next;
}task;


typedef struct sy_thread_pool{
	task *task_queue_head;	//任务队列头
	task *task_queue_tail;	//任务队列尾
	int task_num;			//任务个数

	int thread_num;			//线程总数
	int idle_thread_num;	//空闲线程数

	pthread_t *thread_queue;//线程队列

	pthread_mutex_t queue_mutex;
	pthread_cond_t queue_cond;

	int is_distoried;		//标志位 标志是否被销毁

}sy_thread_pool;


#ifdef __cplusplus
extern "C"{
#endif

extern void* sy_thread_pool_entrance(void *arg);
extern void* sy_thread_pool_init(int thread_pool_size);
extern int sy_thread_pool_destroy();
extern int sy_add_task(void *(*taskfunc)(void *arg), void* arg);

#ifdef __cplusplus
}
#endif

#endif

