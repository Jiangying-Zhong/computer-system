#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "sys/types.h" 
#include "linux/sem.h"

int semid; //�źŵ�
pthread_t p1,p2; //�߳̾�� 
int a;

//P��V�������� 
void P(int semid,int index){
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0; //������ǣ�0��IPC_NOWAIT�� 
	semop(semid,&sem,1); //1����ʾִ������ĸ��� 
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

//�߳�ִ�к��� 
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
	//��һ��key������ֵ������صĽ��̿�ͨ��������һ���ź��� 
	//������ͨ������semget()�������ṩһ����������ϵͳ����һ����Ӧ���źű�ʶ����semget()�����ķ���ֵ�� 
	//�ڶ���num_semsΪ�źŵƸ���
	//������semflgΪIPC_CREAT|0666 
	//semget()�����ɹ�����һ����Ӧ�źű�ʶ�������㣩��ʧ�ܷ���-1. 
	//Ҫ�ڸ��ӽ��̶��ɼ��ĵط����ã����ڴ����ӽ���֮ǰ����������ʵ���ڴ�Ĺ���
	semid = semget(IPC_PRIVATE,2,IPC_CREAT|0666);
	//int semctl(int semid,int semnum,int cmd,union semun arg);
	//����ɹ�����Ϊһ������
	//SETVAL�����ź������е�һ���������ź�����ֵ 
	//arg.val = 0;
	semctl(semid,0,SETVAL,0);
	//arg.val = 1;
	semctl(semid,1,SETVAL,1);
	//�̴߳��� 
	// pthread_create(pthread_t *thread, pthread_attr_t *attr,void *(*start_routine)(void *),void *arg);
	pthread_create(&p1,NULL,subp1,NULL);
	pthread_create(&p2,NULL,subp2,NULL);
	//�̹߳���
	//pthread_join(pthread_t th, void **thread_retrun);
	//����ǰ�߳�ֱ���ɲ���thָ�����̱߳���ֹΪֹ
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	//ɾ���źŵ�
	//int semctl(int semid,int semnum,int cmd,union semun arg);
	//IPC_RMID���ź��������ڴ���ɾ��
	semctl(semid,0,IPC_RMID,NULL);
	semctl(semid,1,IPC_RMID,NULL); 
	return;
}
