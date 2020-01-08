#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int depth=0;
void printdir(char *dir,int depth,long long *total);
void ls(struct stat statbuf,struct dirent *entry,long long *total);
void tab(int tab);
void permission(mode_t mode);

int main(int argc, char *argv[]){
	char *topdir = ".";
	printf("Directory scan of %s\n", topdir);
	long long total=0;
    printdir(topdir,depth,&total);
	printf("total %lld\n",total);
	printf("Done.");
	exit(0);
}

void printdir(char *dir,int depth,long long *total){
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	//��dirĿ¼ʧ�� 
	if(((dp=opendir(dir)))==NULL) //DIR *opendir��const char *name)�� ͨ��·����һ��Ŀ¼������һ��DIR�ṹ��ָ�루Ŀ¼������ʧ�ܷ���NULL 
	{
		fprintf(stderr, "Can`t open directory %s\n", dir);
		return;
	}
	//int chdir(const char *path);
	//�ı�Ŀ¼�����û�ͨ��cd����ı�Ŀ¼һ��������Ҳ����ͨ��chdir���ı�Ŀ¼
	//��Ҫ·����ϵͳ���ã�����ʹ������ڵ�ǰĿ¼�����·�����ļ���Ŀ¼�� 
	chdir(dir); //��dir����Ϊ��ǰĿ¼ 
	// ��ȡһ��Ŀ¼�� 
	while((entry=readdir(dp))!=NULL)	
	{
		//int lstat(const char *path,struct stat *buf);  
		//�Ը�Ŀ¼�������Ϊ����������lstat�õ���Ŀ¼��������Ϣ 
		lstat(entry->d_name, &statbuf);
		//S_ISDIR�ж�ĳ�ļ�s�Ƿ�ΪĿ¼�ļ� 
		if(S_ISDIR(statbuf.st_mode)){  //�ж���Ŀ¼ 
			if(strcmp(entry->d_name,"..")==0||strcmp(entry->d_name,".")==0) //���Ŀ¼���������".."/"." 
				continue; //������Ŀ¼�� 
			tab(depth);
			ls(statbuf,entry,total);
			/*printf("DIR DEPTH: %d ",depth); //��ӡĿ¼������
			printf("DIR TYPE: %c\n",entry->d_type);
			printf("DIR NAME: %s\n",entry->d_name); //��ӡĿ¼���Ŀ¼�� */
			printdir(entry->d_name,depth+4,total); //�ݹ����printdir����ӡ��Ŀ¼����Ϣ������depth+4 
		}
		else{
			tab(depth);
			ls(statbuf,entry,total);
		}
	}
	chdir("..");	//���ظ�Ŀ¼ 
	closedir(dp);	//�ر�Ŀ¼�� 
}

void ls(struct stat statbuf,struct dirent *entry,long long *total)
{
    struct passwd *psw;
    struct group *grp;
    psw = getpwuid(statbuf.st_uid); //getpwuid(); ͨ��uid�����û���passwd���ݣ��������򷵻�һ����ָ�벢����errno��ֵ�� 
    grp = getgrgid(statbuf.st_gid); //getgrgid(); ������gidָ������ʶ������һ�������ļ����ҵ�ʱ�㽫����������group�ṹ���� 
    permission(statbuf.st_mode);
    printf(" %hu ",statbuf.st_nlink);
    printf("%s ",psw->pw_name);
    printf("%s ",grp->gr_name);
    printf("%lu ",statbuf.st_size);
    printf("%s ",entry->d_name);
    *total += statbuf.st_size;
	printf("%s",ctime(&statbuf.st_mtime));
}

void tab(int tab){
    int i=0;
    for(;i<tab;++i) putchar(' ');
}

void permission(mode_t mode){
    mode_t ruler = 0x100; 
    mode_t tmp;
    int i;
    if(S_ISLNK(mode)) putchar('l'); //�Ƿ���һ������  
    else if(S_ISDIR(mode)) putchar('d'); //�Ƿ���һ��Ŀ¼ 
    else if(S_ISCHR(mode)) putchar('c'); //�Ƿ���һ���ַ��豸 
    else if(S_ISBLK(mode)) putchar('b'); //�Ƿ���һ�����豸 
    else if(S_ISSOCK(mode)) putchar('s'); //�Ƿ���һ��SOCKET�ļ� 
    else if(S_ISFIFO(mode)) putchar('p'); //�Ƿ���һ��FIFO�ļ� 
    else if(S_ISREG(mode)) putchar('-'); //�Ƿ���һ�������ļ� 
    for(i=1;i<=9;++i,ruler>>=1){
	tmp = ruler & mode;
	if(tmp==0) putchar('-');
    else{
	    switch(i%3){
         	case 0: putchar('x'); break;
	        case 1: putchar('r'); break;
			case 2: putchar('w'); break;
	    	}
		}
    }
}

