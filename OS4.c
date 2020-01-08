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
	//打开dir目录失败 
	if(((dp=opendir(dir)))==NULL) //DIR *opendir（const char *name)； 通过路径打开一个目录，返回一个DIR结构体指针（目录流），失败返回NULL 
	{
		fprintf(stderr, "Can`t open directory %s\n", dir);
		return;
	}
	//int chdir(const char *path);
	//改变目录，与用户通过cd命令改变目录一样，程序也可以通过chdir来改变目录
	//需要路径的系统调用，可以使用相对于当前目录的相对路径打开文件（目录） 
	chdir(dir); //将dir设置为当前目录 
	// 读取一个目录项 
	while((entry=readdir(dp))!=NULL)	
	{
		//int lstat(const char *path,struct stat *buf);  
		//以该目录项的名字为参数，调用lstat得到该目录项的相关信息 
		lstat(entry->d_name, &statbuf);
		//S_ISDIR判断某文件s是否为目录文件 
		if(S_ISDIR(statbuf.st_mode)){  //判断是目录 
			if(strcmp(entry->d_name,"..")==0||strcmp(entry->d_name,".")==0) //如果目录项的名字是".."/"." 
				continue; //跳过该目录项 
			tab(depth);
			ls(statbuf,entry,total);
			/*printf("DIR DEPTH: %d ",depth); //打印目录项的深度
			printf("DIR TYPE: %c\n",entry->d_type);
			printf("DIR NAME: %s\n",entry->d_name); //打印目录项的目录名 */
			printdir(entry->d_name,depth+4,total); //递归调用printdir，打印子目录的信息，其中depth+4 
		}
		else{
			tab(depth);
			ls(statbuf,entry,total);
		}
	}
	chdir("..");	//返回父目录 
	closedir(dp);	//关闭目录项 
}

void ls(struct stat statbuf,struct dirent *entry,long long *total)
{
    struct passwd *psw;
    struct group *grp;
    psw = getpwuid(statbuf.st_uid); //getpwuid(); 通过uid查找用户的passwd数据，若出错则返回一个空指针并设置errno的值。 
    grp = getgrgid(statbuf.st_gid); //getgrgid(); 依参数gid指定的组识别码逐一搜索组文件，找到时便将该组数据以group结构返回 
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
    if(S_ISLNK(mode)) putchar('l'); //是否是一个连接  
    else if(S_ISDIR(mode)) putchar('d'); //是否是一个目录 
    else if(S_ISCHR(mode)) putchar('c'); //是否是一个字符设备 
    else if(S_ISBLK(mode)) putchar('b'); //是否是一个块设备 
    else if(S_ISSOCK(mode)) putchar('s'); //是否是一个SOCKET文件 
    else if(S_ISFIFO(mode)) putchar('p'); //是否是一个FIFO文件 
    else if(S_ISREG(mode)) putchar('-'); //是否是一个常规文件 
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

