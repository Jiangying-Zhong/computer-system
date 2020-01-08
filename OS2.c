#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "sys/types.h" 
#include "linux/sem.h"

int semid; //信号灯
pthread_t p1,p2; //线程句柄 
int a;

//P，V操作函数 
void P(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0; //操作标记：0或IPC_NOWAIT等 
	semop(semid,&sem,1); //1：表示执行命令的个数 
	return;
}

void V(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = 1;
	sem.sem_flg = 0;
	semop(semid,&sem,1);
	return;
} 

//线程执行函数 
void *subp1(){
	int i;
	for(i=1;i<=100;i++){
		P(semid,0);                                                                       
		printf("i=%d sum=%d\n",i,a);
		V(semid,1);
	}
	return;
}

void *subp2(){
	int i;
	for(i=1;i<=100;i++){
		P(semid,1);
		a = a+i;
		V(semid,0);
	}
	return;
}

int main(){
	//int semget(key_t key, int num_sems, int sem_flags);
	//第一个key是整数值，不相关的进程可通过它访问一个信号量 
	//程序先通过调用semget()函数并提供一个键，再由系统生成一个相应的信号标识符（semget()函数的返回值） 
	//第二个num_sems为信号灯个数
	//第三个semflg为IPC_CREAT|0666 
	//semget()函数成功返回一个相应信号标识符（非零），失败返回-1. 
	//要在父子进程都可见的地方调用（即在创建子进程之前），否则不能实现内存的共享
	semid = semget(IPC_PRIVATE,2,IPC_CREAT|0666);
	//int semctl(int semid,int semnum,int cmd,union semun arg);
	//如果成功，则为一个正数
	//SETVAL设置信号量集中的一个单独的信号量的值 
	//arg.val = 0;
	semctl(semid,0,SETVAL,0);
	//arg.val = 1;
	semctl(semid,1,SETVAL,1);
	//线程创建 
	// pthread_create(pthread_t *thread, pthread_attr_t *attr,void *(*start_routine)(void *),void *arg);
	pthread_create(&p1,NULL,subp1,NULL);
	pthread_create(&p2,NULL,subp2,NULL);
	//线程挂起
	//pthread_join(pthread_t th, void **thread_retrun);
	//挂起当前线程直到由参数th指定的线程被终止为止
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	//删除信号灯
	//int semctl(int semid,int semnum,int cmd,union semun arg);
	//IPC_RMID将信号量集从内存中删除
	semctl(semid,0,IPC_RMID,NULL);
	semctl(semid,1,IPC_RMID,NULL); 
	return;
}
