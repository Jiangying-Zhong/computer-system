#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>

using namespace std;

#define PATH_BUF "/home"

int semid; //�źŵ�
pid_t read_buf, write_buf;
int i,blocknum;
int shmid;


int CreateShmId(int id) {
	key_t key;
  	key = ftok(PATH_BUF,id);
  	if(key == -1) {
    	cout << "FTOK ERROR!" << endl;
    	exit(1);
  	}
  	//int shmget(key_t key, size_t size,int shmflg);
	//�������һ�鹲���ڴ���
    //key�����̼�ͨ�ż�ֵ��ftok()�ķ���ֵ
	//size���ù���洢�εĳ���(�ֽ�)
	//shmflg����ʶ��������Ϊ�������ڴ��Ȩ�ޣ���ȡֵ���£�
	//IPC_CREAT����������ھʹ���
	//IPC_EXCL��  ����Ѿ������򷵻�ʧ��
	//λ��Ȩ��λ�������ڴ�λ��Ȩ��λ��������ù����ڴ�ķ���Ȩ�ޣ���ʽ�� open() ������ mode_t һ��������ִ��Ȩ��δʹ�á�
  	int shmid = shmget(key, 128, IPC_CREAT | 0660); //���������ڴ� 
  	return shmid;
}


int GetShmId(int id) {
	key_t key;
  	key = ftok(PATH_BUF,id);
  	if(key == -1) {
    	cout << "FTOK ERROR!" << endl;
    	exit(1);
  	}
	int shmid = shmget(key, 0, IPC_CREAT | 0660); //��ù����ڴ� 
  	return shmid;
} 

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

int main(void) {
   int flag;
   key_t key, key_sem;

   key_sem = ftok("./", 2015); //����keyֵ 
   if(key_sem == -1) {
     perror("ftok");
	 exit(1);
   }
   
   //���������ڴ� 
   for(int i = 0; i < 10; i++) {
     int shmid = CreateShmId(i); //shmid�������ڴ��ʶ  
   }
   
   int semid = semget(key_sem, 2, IPC_CREAT | 0660);
   flag = semctl(semid, 1, SETALL, 1); //�źŵƸ�ֵ 

   if((read_buf = fork()) == -1) {
      cout << "fork 1 failed!" << endl;
      exit(1);
   }
   else if(read_buf == 0) {
      int fd = open("in.txt",O_RDONLY);
      int filesize = lseek(fd, 0, SEEK_END);
      if(filesize % 128 == 0)
         blocknum = filesize / 128;
      else
         blocknum = filesize / 128 + 1;

      for(int num = 0; num < blocknum; num++) {
        cout << "--------------------------read begin------------------------" << endl;
        shmid = GetShmId(i);
        char *shared = (char*)shmat(shmid, NULL, 0); //�����ڴ�ӳ��  
        lseek(fd, num*128, SEEK_SET);
        int len =  read(fd, shared, 128);
        cout << shared << endl;
        cout << "LENGTH:" << strlen(shared) << endl;
        cout << "--------------------------read over------------------------" << endl;
        i = (i+1) % 10;
        V(semid, 0);
        P(semid, 1);
      }
      kill(write_buf,SIGKILL);
      exit(0);
   }
   
   if((write_buf = fork()) == -1) {
      cout << "fork 2 failed!" << endl;
      exit(1);
   }
   else if(write_buf == 0) {
      int fd = open("out.txt", O_WRONLY|O_CREAT);
      while(1) {
        P(semid, 0);
        cout << "--------------------------write start----------------------" << endl;
        cout << "MEMNUM :" << i << endl;
        shmid = GetShmId(i);
        //cout << "SHMID : " << shmid << endl;
        char *shared = (char*)shmat(shmid, NULL, 0);
        cout << shared << endl;
        cout << "LENGTH:" << strlen(shared) << endl;
        //lseek(fd, blocknum*128, SEEK_SET);
        //int len = write(fd, shared, strlen(shared));
        //cout << "REAL LENGTH:" << len << endl;
        memset(shared, '\0', 128);
        cout << "--------------------------write end----------------------" << endl;
        i = (i+1) % 10;
        blocknum++;
        V(semid, 1);
      }
   }
   //�ȴ��ӽ��̽��� 
   waitpid(write_buf, &flag, 0);
   waitpid(read_buf, &flag, 0);
   //ɾ���ź�����IPC_RMID���ź��������ڴ���ɾ�� 
   semctl(semid,0,IPC_RMID,NULL);
   for(int i = 0; i < 10; i++) {
   		//int shmid = GetShmId(i);
    	//int shmctl(int shmid, int cmd, struct shmid_ds *buf); 
        //shmid�������ڴ��ʶ��
        //cmd���������ܵĿ��ƣ� IPC_RMID��ɾ��
        //buf��shmid_ds �������͵ĵ�ַ(���������������� )��������Ż��޸Ĺ����ڴ������ 
     	shmctl(shmid, IPC_RMID, 0); //�ͷŹ����ڴ��� 
	}
    return 0;
}






