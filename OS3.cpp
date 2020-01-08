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

int semid; //信号灯
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
	//创建或打开一块共享内存区
    //key：进程间通信键值，ftok()的返回值
	//size：该共享存储段的长度(字节)
	//shmflg：标识函数的行为及共享内存的权限，其取值如下：
	//IPC_CREAT：如果不存在就创建
	//IPC_EXCL：  如果已经存在则返回失败
	//位或权限位：共享内存位或权限位后可以设置共享内存的访问权限，格式和 open() 函数的 mode_t 一样，但可执行权限未使用。
  	int shmid = shmget(key, 128, IPC_CREAT | 0660); //创建共享内存 
  	return shmid;
}


int GetShmId(int id) {
	key_t key;
  	key = ftok(PATH_BUF,id);
  	if(key == -1) {
    	cout << "FTOK ERROR!" << endl;
    	exit(1);
  	}
	int shmid = shmget(key, 0, IPC_CREAT | 0660); //获得共享内存 
  	return shmid;
} 

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

int main(void) {
   int flag;
   key_t key, key_sem;

   key_sem = ftok("./", 2015); //创建key值 
   if(key_sem == -1) {
     perror("ftok");
	 exit(1);
   }
   
   //创建共享内存 
   for(int i = 0; i < 10; i++) {
     int shmid = CreateShmId(i); //shmid：共享内存标识  
   }
   
   int semid = semget(key_sem, 2, IPC_CREAT | 0660);
   flag = semctl(semid, 1, SETALL, 1); //信号灯赋值 

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
        char *shared = (char*)shmat(shmid, NULL, 0); //共享内存映射  
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
   //等待子进程结束 
   waitpid(write_buf, &flag, 0);
   waitpid(read_buf, &flag, 0);
   //删除信号量：IPC_RMID将信号量集从内存中删除 
   semctl(semid,0,IPC_RMID,NULL);
   for(int i = 0; i < 10; i++) {
   		//int shmid = GetShmId(i);
    	//int shmctl(int shmid, int cmd, struct shmid_ds *buf); 
        //shmid：共享内存标识符
        //cmd：函数功能的控制： IPC_RMID：删除
        //buf：shmid_ds 数据类型的地址(具体类型请点此链接 )，用来存放或修改共享内存的属性 
     	shmctl(shmid, IPC_RMID, 0); //释放共享内存区 
	}
    return 0;
}






