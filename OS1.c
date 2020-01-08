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
    	//signal->ע���źŴ������ĺ���
		//void signal(int SigNo(�źű��),void (*Handle) int(�Զ����źŴ�����)) 
		signal(SIGINT,SIG_IGN);//SIG_IGN->���ԣ���������ص����ø��źŵ�����ΪSIG_IGN,����ý��̵��ӽ��̽��������������̡� 
		signal(SIGUSR1,myfunc);
        while(1){
        	//int sprintf( char *buffer, const char *format, [ argument] �� );
        	//buffer��char��ָ�룬ָ��Ҫд����ַ����Ļ�������
			//format����ʽ���ַ�����
			//[argument]...����ѡ�������������κ����͵����ݡ�
	    	sprintf(buf,"I send you %d times",x++);
           	write(pipefd[1],buf,sizeof(buf) ); //������д��ܵ� 
           	sleep(1); //ÿ��1���ӽ���1ͨ���ܵ����ӽ���2�������� 
        }
	}
	while((pid2=fork())==-1);
    if(pid2==0){ //son2
    	x++;
  		//signal->ע���źŴ������ĺ���
		//void signal(int SigNo(�źű��),void (*Handle) int(�Զ����źŴ�����)) 
		signal(SIGINT,SIG_IGN);
		signal(SIGUSR2,myfunc);
		while(1){
			read(pipefd[0],buf,sizeof(buf)); //�ӹܵ���ȡ���� 
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

