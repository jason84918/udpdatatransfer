#include<iostream>
#include<cstdio>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/errno.h>
#include<fcntl.h>
#include<netdb.h>
#include<cstring>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstdlib>
#include<string>
#include<vector>
#include<sstream>
#include<climits>
using namespace std;
int used[1000000];
char ack[]="ack";
char finish[]="finish!!!";
void dg_echo(int,struct sockaddr*,socklen_t,char*);
int main(int argc,char ** argv){
    if(argc!=3){
        cout<<"input error"<<endl;
        return 0;
    }
    for(int i=0;i<1000000;i++){
        used[i]=0;
    }
	int sockfd;
	struct sockaddr_in servaddr,cliaddr;
	const int on=1;
	//socket
	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){
        perror("socket error");
        exit(0);
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	int PORT;
	PORT=atoi(argv[1]);
	servaddr.sin_port=htons(PORT);
	cout<<"socket success"<<endl;

	//bind
	//setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	if((bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)))<0){
        perror("bind error");
        exit(0);
	}
	cout<<"bind success"<<endl;
	cout<<"sockfd"<<sockfd<<endl;
	dg_echo(sockfd,(struct sockaddr*)&cliaddr,sizeof(cliaddr),argv[2]);
	close(sockfd);
}

void dg_echo(int sockfd,struct sockaddr* cliaddr, socklen_t clilen,char* dest){
	int n,x=0,packet=0,record=0;
	socklen_t len;
	char message[1500];
	char order[33];
	char receive[1000];
	bzero(message,sizeof(message));
	bzero(order,sizeof(order));
	bzero(receive,sizeof(receive));
	FILE *fp;
	fp=fopen(dest,"wb");
    if(fp==NULL){
        perror("destination error");
        exit(1);
    }
	for(;;){
		len=clilen;
		if((n=recvfrom(sockfd,message,1500,0,cliaddr,&len))<0) perror("recvfrom error");
		if(!strcmp(message,finish)){
            sendto(sockfd,ack,strlen(ack),0,cliaddr,len);
            break;
		}
		cout<<"n:"<<n<<endl;
		for(int i=0;i<32;i++){
            order[i]=message[i];
		}
		for(int i=32;i<1032;i++){
            receive[i-32]=message[i];
		}
//		cout<<"order:"<<strlen(order)<<endl;
//		cout<<"receive:"<<strlen(receive)<<endl;
//		x+=strlen(receive);
//		cout<<"x:"<<x<<endl;
		int sum=0,base=1;
		for(int i=0;i<32;i++){
            sum+=(order[i]-'0')*base;
            base*=2;
		}
		cout<<"sum:"<<sum<<endl;
		if(used[sum]==1){
            //cout<<"-------A-------"<<endl;
            sendto(sockfd,order,strlen(order),0,cliaddr,len);
            bzero(message,sizeof(message));
            bzero(order,sizeof(order));
            bzero(receive,sizeof(receive));
            continue;
		}
		else{
		    //cout<<"-------B-------"<<endl;
            packet++;
            if(n!=1032){
                record++;
            }
            used[sum]=1;
            if(write(fileno(fp),receive,n-32)<0) perror("write error");
            sendto(sockfd,order,strlen(order),0,cliaddr,len);
            bzero(message,sizeof(message));
            bzero(order,sizeof(order));
            bzero(receive,sizeof(receive));
		}
	}
	fseek(fp,0,SEEK_END);
    long filelen=ftell(fp);
    cout<<"filelen:"<<filelen<<endl;
	fclose(fp);
	for(int i=0;i<packet;i++){
        if(used[i]==0){
            cout<<"NO."<<i<<"lost"<<endl;
        }
	}
	cout<<"total packet: "<<packet<<endl;
	cout<<"record: "<<record<<endl;
}
