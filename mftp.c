#include "mftp.h"
//#define MY_PORT_NUMBER 49999	

int getsockfd(char* hostname, char* port);
int cd_cmd(char* pathname){
	if(pathname!=NULL){
		printf("%s\n",pathname);
	}
	if(chdir(pathname)<0){
		printf("%s\n",strerror(errno));
	}
	return 1;
}
int rcd_cmd(char* pathname, int socketfd){
	char buffer[512];
	bzero(buffer,512);
	char command[strlen(pathname)+2];
	command[0]='C';
	command[1]='\0';
	strcat(command,pathname);
	write(socketfd,command,strlen(command));
	read(socketfd,buffer,sizeof(buffer));
	bzero(buffer,512);
	return 1;
}
int ls_cmd(){
	int pid;
	int fd[2];
	char *lscmd[3]={"ls","-la",NULL};
	char *morecmd[3]={"more","-20",NULL};
	if(!fork()){
		if(pipe(fd)<0){
			perror("pipe");
		}
		if(!fork()){
			if(close(fd[0])<0){
				printf("%s\n",strerror(errno));
			}
			if(fd[1] != 1){
				if((dup2(fd[1],1)<0)){
					printf("%s\n",strerror(errno));
				}
			}
			if(execvp(lscmd[0],lscmd)==-1){
				printf("%s\n",strerror(errno));
			}
		}
		else{
			wait(NULL);
			if(close(fd[1])<0){
				printf("%s\n",strerror(errno));
			}
			if(fd[0]!=0){
				if((dup2(fd[0],0)<0)){
					printf("%s\n",strerror(errno));
				}
			}
			if(execvp(morecmd[0],morecmd)==-1){
				printf("%s\n",strerror(errno));
			}
		}
	}
	else{
		wait(NULL);
	}
	return 1;
}
int rls_cmd(int socketnum, char* hostname){
	int pid;
	int fd[2];
	int Dsocket;
	char buffer[512];
	int readnum;
	char* morecmd[3]={"more","-20",NULL};
	pipe(fd);
	write(socketnum,"D\n",2);
	read(socketnum,buffer,512);
	char* port=&buffer[1];
	Dsocket=getsockfd(hostname,port);
	bzero(buffer,512);
	write(socketnum,"L\n",2);
	read(socketnum,buffer,512);
	if(buffer[0]!='A'){
		return -1;
	}
	else{
		if(!fork()){
			if(dup2(Dsocket,0)<0){
				printf("%s\n",strerror(errno));
			}
			if(execvp(morecmd[0],morecmd)==-1){
				printf("%s\n",strerror(errno));
			}
		}
		else{
			wait(NULL);
		}

	}
	bzero(buffer,512);
	return 1;
}
int get_cmd(int socketnum,char* hostname,char* pathname, char* savepath){
	int Dsocket;
	int fileDesc;
	char buffer[512];
	int numread;
	write(socketnum,"D\n",2);
	read(socketnum,buffer,512);
	char* port=&buffer[1];
	port[strlen(port)-1]='\0';
	Dsocket=getsockfd(hostname,port);
	fflush(stdout);
	bzero(buffer,512);
	char command[strlen(pathname)+2];
	command[0]='G';
	command[1]='\0';
	strcat(command,pathname);
	//Dsocket=getsockfd(hostname,port);
	fileDesc=open(savepath,O_WRONLY|O_CREAT|O_APPEND, 0755);
	if(fileDesc<0){
		printf("%s\n",strerror(errno));
		//close(Dsocket);
		close(fileDesc);
		return -1;
	}
	/*struct stat fstatus;
	if(fstat(fileDesc,&fstatus)!=0){
		printf("%s\n",strerror(errno));
		close(Dsocket);
		close(fileDesc);
		return -1;
	}
	if(S_ISDIR(fstatus.st_mode)){
		printf("tried to open Directory\n");
		close(Dsocket);
		close(fileDesc);
		return -1;
	}
	if(S_ISREG(fstatus.st_mode)!=0){
		printf("non regular file\n");
		close(Dsocket);
		close(fileDesc);
		return -1;
	}*/
	bzero(buffer,512);
	write(socketnum,command,strlen(command));
	read(socketnum,buffer,512);
	if(buffer[0]=='A'){
		bzero(buffer,512);
		while((numread=read(Dsocket,buffer,sizeof(buffer)))>0){
			write(fileDesc,buffer,numread);
		}
		if(numread<0){
			printf("%s\n",strerror(errno));
			//close(Dsocket);
			close(fileDesc);
			return -1;
		}
	}
	else{
		return -1;
	}
	bzero(buffer,512);
	close(Dsocket);
	close(fileDesc);
	return 1;
}
int show_cmd(int socketnum,char* hostname, char* pathname, char* savepath){
	int Dsocket;
	int fileDesc;
	char buffer[512];
	int readnum;
	write(socketnum,"D\n",2);
	read(socketnum,buffer,512);
	char* port=&buffer[1];
	port[strlen(port)-1]='\0';
	Dsocket=getsockfd(hostname,port);
	fflush(stdout);
	bzero(buffer,512);
	char command[strlen(pathname)+2];
	char* morecmd[3]={"more","-20",NULL};
	command[0]='G';
	command[1]='\0';
	strcat(command,pathname);
	//Dsocket=getsockfd(hostname,port);
	fileDesc=open(savepath,O_WRONLY|O_CREAT|O_APPEND,0755);
	bzero(buffer,512);
	write(socketnum,command,strlen(command));
	read(socketnum,buffer,512);
	if(buffer[0]=='A'){
		printf("received A\n");
		if(!fork()){
			if(dup2(Dsocket,0)>0){
				printf("%s\n",strerror(errno));
				return -1;
			}
			if(execvp(morecmd[0],morecmd)==-1){
				printf("%s\n",strerror(errno));
				return -1;
			}
		}
		else{
			wait(NULL);
		}
		/*while(readnum=read(Dsocket,buffer,1)>0){
			write(1,buffer,readnum);
			//printf("stuck\n");
			fflush(stdout);
			//pipe into more in the future
		}
		if(readnum<0){
			printf("%s\n",strerror(errno));
		}*/
	}
	else{
		printf("error receiving acknowledgement\n");
		return -1;
	}
	bzero(buffer,512);
	close(fileDesc);
	return 1;
}
int put_cmd(int socketnum,char* hostname, char* pathname, char* savepath){
	int Dsocket;
	int fileDesc;
	int readnum;
	char buffer[512];
	write(socketnum,"D\n",2);
	read(socketnum,buffer,512);
	char* port=&buffer[1];
	port[strlen(port)-1]='\0';
	savepath[strlen(savepath)]='\n';
	savepath[strlen(savepath)+1]='\0';
	Dsocket=getsockfd(hostname,port);
	fflush(stdout);
	bzero(buffer,512);
	char command[strlen(pathname)+2];
	command[0]='P';
	command[1]='\0';
	strcat(command,savepath);
	//Dsocket=getsockfd(hostname,port);
	pathname[strlen(pathname)-1]='\0';
	fileDesc=open(pathname,O_RDONLY);
	if(fileDesc<0){
		printf("%s\n",strerror(errno));
	}
	struct stat fstatus;
	if(fstat(fileDesc,&fstatus)!=0){
		printf("%s\n",strerror(errno));
		//close(Dsocket);
		close(fileDesc);
		return -1;
	}
	if(S_ISDIR(fstatus.st_mode)){
		printf("tried to open Directory\n");
		//close(Dsocket);
		close(fileDesc);
		return -1;
	}
	if(S_ISREG(fstatus.st_mode!=0)){
		printf("non regular file\n");
		//close(Dsocket);
		close(fileDesc);
		return -1;
	}
	bzero(buffer,512);
	write(socketnum,command,strlen(command));
	printf("done writing command\n");
	fflush(stdout);
	read(socketnum,buffer,512);
	if(buffer[0]=='A'){
		printf("received A\n");
		/*while(read(fileDesc,buffer,512)){
			write(Dsocket,buffer,512);
			printf("stuck\n");
			fflush(stdout);
			//pipe into more in the future
		}*/
		while((readnum=(read(fileDesc,buffer,sizeof(buffer))))>0){
			write(Dsocket,buffer,readnum);
			fflush(stdout);
		}
		if(readnum<0){
			printf("%s\n",strerror(errno));
			close(Dsocket);
			return -1;
		}
		//printf("%d\n",readnum);
		printf("stopped writing zero\n");
		fflush(stdout);
	}
	else{
		printf("error receiving acknowledgement\n");
		return -1;
	}
	close(fileDesc);
	close(Dsocket);
	return 1;
}
int getsockfd(char* hostname,char* port){
	printf("started getting socket\n");
	fflush(stdout);
	char hostName[NI_MAXHOST];
	//printf("%s hostname %s port \n",hostname, port);
	for(int i=0;i<512;i++){
		if(port[i]=='\n'){
			port[i]='\0';
			break;
		}
	}
	printf("%s hostname %s port \n",hostname, port);
	int hostEntry;
	struct sockaddr_in clientAddr;
	/*hostEntry = getnameinfo((struct sockaddr*)&clientAddr,
			sizeof(clientAddr),
			hostName,
			sizeof(hostName),
			NULL,
			0,
			NI_NUMERICSERV);*/
	int socketfd;
	struct addrinfo hints, *actualdata;
	memset(&hints,0,sizeof(hints));
	int err;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_family=AF_INET;
	err=getaddrinfo(hostname,port,
			&hints,
			&actualdata);
	if(err!=0){
		printf("ERROR: %s\n",gai_strerror(err));
		exit(1);
	}
	socketfd=socket(actualdata->ai_family,actualdata->ai_socktype,0);
	if(socketfd==-1){
		printf("error %s\n",strerror(errno));
		close(socketfd);
		return -1;
	}
	if(connect(socketfd,actualdata->ai_addr,actualdata->ai_addrlen)==-1){
			printf("error:%s\n",strerror(errno));
			close(socketfd);
			return -1;
	}
	return socketfd;
}

int main(int argc,char const *argv[]){
	char hostName[NI_MAXHOST];
	int hostEntry;
	struct sockaddr_in clientAddr;
	hostEntry = getnameinfo((struct sockaddr*)&clientAddr,
		sizeof(clientAddr),
			hostName,
			sizeof(hostName),
			NULL,
			0,
			NI_NUMERICSERV);
	int socketfd;
	struct addrinfo hints, *actualdata;
	memset(&hints, 0, sizeof(hints));
	int err;
	char* delimit=" ";
	char *token;
	char *path;
	char *hostname;
	char *pathforsave;
	char *pathdelimit="/";
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family=AF_INET;
	if(argc>2){
		err=getaddrinfo(argv[2],argv[1],
			&hints,
			&actualdata);
		if(err!=0){
			printf("Error: %s\n",gai_strerror(err));
			exit(1);
		}
		socketfd=socket(actualdata->ai_family,actualdata->ai_socktype,0);
		if(connect(socketfd,actualdata->ai_addr,actualdata->ai_addrlen)==-1){
			printf("Error: %s\n",strerror(errno));
			exit(1);
		}
		char buffer[512];
		char *dupstr;
		int n;
		int i;
		int errorcheck;
		char* pathdup;
		while(1){
			bzero(buffer,512);
			printf("> ");
			fflush(stdout);
			read(0,buffer,512);
			dupstr=strdup(buffer);
			token=strtok(buffer,delimit);
			path=strtok(NULL,delimit);
			//printf("%s\n",token);
			/*if(path!=NULL){
				printf("%s\n",path);	
			}*/
			if(strcmp("exit\n",token)==0){
				write(socketfd,"Q\n",2);
				return 1;
			}
			if(strcmp("ls\n",token)==0){
				ls_cmd();
				wait(NULL);
			}
			if(strcmp("rls\n",token)==0){
				hostname=strdup(argv[2]);
				rls_cmd(socketfd,hostname);
				printf("rls finished running\n");
			}
			if(strcmp("get",token)==0){
				printf("get received \n");
				fflush(stdout);
				hostname=strdup(argv[2]);
				char* temp;
				pathdup=strdup(path);
				temp=strtok(pathdup,pathdelimit);
				while(temp!=NULL){
					pathforsave=temp;
					printf("%s\n",temp);
					fflush(stdout);
					temp=strtok(NULL,pathdelimit);
				}
				pathforsave[strlen(pathforsave)-1]='\0';
				get_cmd(socketfd,hostname,path,pathforsave);
				printf("finished get");
				fflush(stdout);
			}
			if(strcmp("put",token)==0){
				printf("put received \n");
				fflush(stdout);
				hostname=strdup(argv[2]);
				char* temp;
				pathdup=strdup(path);
				temp=strtok(pathdup,pathdelimit);
				while(temp!=NULL){
					pathforsave=temp;
					printf("%s\n", temp);
					fflush(stdout);
					temp=strtok(NULL,pathdelimit);
				}
				pathforsave[strlen(pathforsave)-1]='\0';
				put_cmd(socketfd,hostname,path,pathforsave);
				printf("finished put");
				fflush(stdout);
			}
			if(strcmp("show",token)==0){
				printf("show received \n");
				fflush(stdout);
				hostname=strdup(argv[2]);
				char* temp;
				pathdup=strdup(path);
				temp=strtok(pathdup,pathdelimit);
				while(temp!=NULL){
					pathforsave=temp;
					printf("%s\n",temp);
					fflush(stdout);
					temp=strtok(NULL,pathdelimit);
				}
				pathforsave[strlen(pathforsave)-1]='\0';
				show_cmd(socketfd,hostname,path,pathforsave);
				fflush(stdout);
			}
			if(strcmp("cd",token)==0){
				path[strlen(path)-1]='\0';
				cd_cmd(path);
				printf("cd'd\n");
				fflush(stdout);
			}
			if(strcmp("rcd",token)==0){
				rcd_cmd(path,socketfd);
				printf("cd'd remote\n");
				fflush(stdout);
			}		
		//n=write(socketfd,buffer,strlen(buffer));
			if(n<0){
				printf("readerror\n");
			}
		/*while(read(socketfd,buffer,255)<0){
			write(0,buffer,strlen(buffer));
			printf("stuck");
			fflush(stdout);
		}*/
			if(n<0){
				printf("writerror\n");
			}
			fflush(stdout);
		}
	}
	
	return 0;
}

