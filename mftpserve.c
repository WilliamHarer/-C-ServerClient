#include "mftp.h"
int quit_command(){
	int pid=getpid();
	printf("Child %d: Quitting\n",pid);
	return 1;
}
int C_command(char* pathname,int socketnum){
	pathname[strlen(pathname)-1]='\0';
	int pid=getpid();
	if(pathname!=NULL){
		printf("%s\n",strerror(errno));
	}
	if(chdir(pathname)<0){
		printf("%s\n",strerror(errno));
	}
	write(socketnum,"A\n",2);
	printf("Child %d: changed current directory to %s\n",pid,pathname);
	return 1;
}
int G_command(char*pathname,int socketnum,int Dsocket){
	int fileDesc;
	char* buffer[512];
	int readnum;
	char* delimit="/";
	char* pathsave;
	char* pathdup;
	int pid=getpid();
	struct stat fstatus;
	pathname[strlen(pathname)-1]='\0';
	if(Dsocket<0){
		printf("no data connection established\n");
		close(Dsocket);
		return -1;
	}
	fileDesc=open(pathname,O_RDONLY);
	if(fileDesc<0){
		printf("%s\n",strerror(errno));
		close(Dsocket);
		return -1;
	}
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
	if(S_ISREG(fstatus.st_mode!=0)){
		printf("non regular file\n");
		close(Dsocket);
		close(fileDesc);
		return -1;
	}
	bzero(buffer,512);
	printf("Child %d: Writing file %s\n",pid,pathname);
	while((readnum=read(fileDesc,buffer,sizeof(buffer)))>0){
		write(Dsocket,buffer,readnum);
		fflush(stdout);
		close(Dsocket);
		return -1;
	}
	if(readnum<0){
		printf("%s\n",strerror(errno));
		close(Dsocket);
		return -1;
	}
	//fflush(stdout);
	close(fileDesc);
	close(Dsocket);
	return 1;
}
int P_command(char*pathname,int socketnum,int Dsocket){
	int fileDesc;
	char* buffer[512];
	int readnum;
	int pid=getpid();
	pathname[strlen(pathname)-1]='\0';
	if(Dsocket<0){
		printf("no data connection established\n");
		close(Dsocket);
		return -1;
	}
	fileDesc=open(pathname,O_WRONLY|O_CREAT|O_APPEND);
	if(fileDesc<0){
		printf("%s\n",strerror(errno));
		close(Dsocket);
		return -1;
	}
	printf("Child %d: reading file %s\n",pid,pathname);
	while((readnum=read(Dsocket,buffer,sizeof(buffer)))>0){
		write(fileDesc,buffer,readnum);
	}
	if(readnum<0){
		printf("%s\n",strerror(errno));
		close(Dsocket);
		return -1;
	}
	bzero(buffer,512);
	close(fileDesc);	
	return 1;
}
int D_command(int socketnum){
	struct sockaddr_in servAddr;
	struct sockaddr_in popServAddr;
	struct sockaddr_in clientAddr;
	int listenfd;
	int Dconnect;

	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("%s\n",strerror(errno));
		close(listenfd);
		return -1;
	}
	setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int));
	memset(&servAddr,0,sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_port=htons(0);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0){
		perror("bind");
		close(listenfd);
		exit(1);
	}
	if(listen(listenfd,1)!=0){
		printf("%s\n",strerror(errno));
		close(listenfd);
		return -1;
	}
	int size=sizeof(popServAddr);
	getsockname(listenfd,(struct sockaddr *) &popServAddr,&size);
	char buffer[100];
	int portlen;
	size=sizeof(struct sockaddr_in);
	portlen=sprintf(buffer,"A%d\n",htons(popServAddr.sin_port));
	size=sizeof(struct sockaddr_in);
	write(socketnum,buffer,portlen);
	if(((Dconnect=accept(listenfd,(struct sockaddr *) &clientAddr,&size))<0)){
		printf("%s   ACCEPT ERROR\n",strerror(errno));
		close(listenfd);
		close(Dconnect);
		return -1;
	}
	write(socketnum,"A\n",2);
	close(listenfd);
	return Dconnect;
}
int L_command(int Dsocket,int socketnum){
	char* lscmd[3]={"ls","-la",NULL};
	if(Dsocket<0){
		write(socketnum,"EError no Datasocket\n",strlen("EError no Datasocket\n"));
		return -1;
	}
	if(!fork()){
		if(dup2(Dsocket,1)<0){
			printf("%s\n",strerror(errno));
			return -1;
		}
		if(execvp(lscmd[0],lscmd)==-1){
				printf("%s\n",strerror(errno));
				return -1;
		}
	}
	else{
		wait(NULL);
	}
	close(Dsocket);
	return -1;
}
int main(int argc, char const *argv[]){
	struct sockaddr_in clientAddr;
	int listenfd;
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR, &(int){1}, sizeof(int));
	struct sockaddr_in servAddr;
	memset(&servAddr, 0 , sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	servAddr.sin_port = htons(MY_PORT_NUMBER);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind( listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind");
		exit(1);
	}
	listen(listenfd, 4);
	int connectfd;
	int length = sizeof(struct sockaddr_in);
	int increment=0;
	int readnum;
	char* clientname;
	char buf[512];
	char* pathdup;
	char* delimit="\n";
	char* command;
	char* path;
	while(1){
		if((connectfd =accept(listenfd,(struct sockaddr *) &clientAddr, &length))>0){

		}
		if(connectfd<0){
			printf("%s\n",strerror(errno));
		}
		char hostName[NI_MAXHOST];
		int hostEntry;
		hostEntry=getnameinfo((struct sockaddr*)&clientAddr,sizeof(clientAddr),
				hostName,
				sizeof(hostName),
				NULL,
				0,
				NI_NUMERICSERV);
		if(!fork()){//child
			int Dsocket=-1;
			int pid=getpid();
			while(1){
				if(((readnum=read(connectfd,buf,sizeof(buf))<0))){
					printf("%s",strerror(errno));
					close(connectfd);
					return -1;
				}
				else{	
					//printf("%c command\n",command[0]);
					fflush(stdout);
					pathdup=strdup(buf);
					path=&pathdup[1];
					command=strtok(buf,delimit);
					if(command!=NULL){
						if(command[0]=='D'){
							printf("child %d: received Datasocket request\n",pid);
							Dsocket=D_command(connectfd);
							fflush(stdout);
						}
						if(command[0]=='G'){
							printf("child %d: received get/show request for %s\n",pid,path);
							Dsocket=G_command(path,connectfd,Dsocket);
							fflush(stdout);
						}
						if(command[0]=='P'){
							printf("child %d: received put request for %s\n",pid,path);
							Dsocket=P_command(path,connectfd,Dsocket);
							fflush(stdout);
						}
						if(command[0]=='Q'){
							printf("quitting\n");
							Dsocket=quit_command();
							fflush(stdout);
							return 1;
						}
						if(command[0]=='L'){
							printf("child %d: received list request\n",pid);
							Dsocket=L_command(Dsocket,connectfd);
							fflush(stdout);
						}
						if(command[0]=='C'){
							printf("Child %d: changing Directorys to%s\n",pid,path);
							Dsocket=C_command(path,connectfd);
							fflush(stdout);
						}
					}
					bzero(buf,sizeof(buf));


				}
				bzero(buf,sizeof(buf));
			}
		}
		else{
			close(connectfd);
		}
	}
	close(connectfd);
	//waitpid(-1,NULL,0);
	return 1;
}
