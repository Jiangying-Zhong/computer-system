#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "sys/wait.h"
#include "sys/types.h" 
#include "unistd.h"

pid_t pid1,pid2;
int pipefd[2];
int x = 1;
char buf[50];

void myfunc(int sig_no){
	if(sig_no == SIGUSR1){
		printf("Child Process 1 is Killed by Parent!\n");
		exit(0);
	}
	else if(sig_no == SIGUSR2){
		printf("Child Process 2 is Killed by Parent!\n");
		exit(0);		
	}
	else if(sig_no == SIGINT){
		kill(pid1,SIGUSR1);
		kill(pid2,SIGUSR2);
	}
}

int main(){
	pipe(pipefd);
	while((pid1=fork())==-1);
    if(pid1==0){ //son1
    	//signal->注册信号处理函数的函数
		//void signal(int SigNo(信号编号),void (*Handle) int(自定义信号处理函数)) 
		signal(SIGINT,SIG_IGN);//SIG_IGN->忽略，如果进程特地设置该信号的配置为SIG_IGN,则调用进程的子进程将不产生僵死进程。 
		signal(SIGUSR1,myfunc);
        while(1){
        	//int sprintf( char *buffer, const char *format, [ argument] … );
        	//buffer：char型指针，指向将要写入的字符串的缓冲区。
			//format：格式化字符串。
			//[argument]...：可选参数，可以是任何类型的数据。
	    	sprintf(buf,"I send you %d times",x++);
           	write(pipefd[1],buf,sizeof(buf) ); //将数据写入管道 
           	sleep(1); //每隔1秒子进程1通过管道向子进程2发送数据 
        }
	}
	while((pid2=fork())==-1);
    if(pid2==0){ //son2
    	x++;
  		//signal->注册信号处理函数的函数
		//void signal(int SigNo(信号编号),void (*Handle) int(自定义信号处理函数)) 
		signal(SIGINT,SIG_IGN);
		signal(SIGUSR2,myfunc);
		while(1){
			read(pipefd[0],buf,sizeof(buf)); //从管道读取数据 
			printf("%s\n",buf);
		}
    }
    wait(&pid1);
    wait(&pid2);
    printf("Parent process is killed!!\n");
    close(pipefd[0]);
    close(pipefd[1]);
    exit(0);
}

