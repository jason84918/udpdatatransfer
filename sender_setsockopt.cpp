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
#include<signal.h>
using namespace std;
void dg_cli(FILE*,int ,struct sockaddr*,socklen_t);
int main(int argc,char **argv){
    if(argc!=4){
        cout<<"input error"<<endl;
        return 0;
    }
	int sockfd;
	struct sockaddr_in servaddr;

	//socket
	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){
        perror("socket error");
        exit(0);
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	int PORT;
	PORT=atoi(argv[2]);
	servaddr.sin_port=htons(PORT);
	inet_pton(AF_INET,argv[1],&servaddr.sin_addr.s_addr);
	cout<<"socket success"<<endl;
	FILE* fp;
	fp=fopen(argv[3],"rb");
    if(fp==NULL){
        perror("source error");
        exit(1);
    }
	dg_cli(fp,sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	close(sockfd);
	fseek(fp,0,SEEK_END);
    long filelen=ftell(fp);
    cout<<"filelen:"<<filelen<<endl;
    fclose(fp);
}

void dg_cli(FILE* fp,int sockfd,struct sockaddr* servaddr,socklen_t servlen){
	int n,ack=0,number=1,x=0,nbyte,packetlost=0;
	struct timeval tv;
	tv.tv_sec=1;
	tv.tv_usec=0;
	char sendline[1000],recvline[256],order[33];
	bzero(sendline,sizeof(sendline));
	bzero(recvline,sizeof(recvline));
	bzero(order,sizeof(order));
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
	while((nbyte=read(fileno(fp),&sendline,1000))){
        cout<<"nbyte:"<<nbyte<<endl;
        int temp=number;
        for(int i=0;i<32;i++){
            char c=(temp%2)+'0';
            order[i]=c;
            temp/=2;
            //cout<<order[i]<<endl;
        }
        //cout<<"order:"<<strlen(order)<<endl;
        char sending[1500];
        bzero(sending,sizeof(sending));
        sprintf(sending,"%s",order);
        for(int i=0;i<nbyte;i++){
            sending[i+32]=sendline[i];
        }
        cout<<"size of sending:"<<nbyte+32<<endl;
		while(!ack){
//            x+=strlen(sendline);
//            cout<<"x:"<<x<<endl;
//            cout<<"sending:"<<strlen(sending)<<endl;
            sendto(sockfd,sending,nbyte+32,0,servaddr,servlen);
            if((n=recvfrom(sockfd,recvline,256,0,servaddr,&servlen))<0){
                if(errno==EWOULDBLOCK){
                    cout<<"No ack! Retransmit!"<<endl;
                    packetlost++;
                }
                else{
                    perror("recvfrom error");
                }
            }
            else if(!strcmp(recvline,order)){
                cout<<recvline<<endl;
                ack=1;
            }
		}
		bzero(sendline,sizeof(sendline));
        bzero(recvline,sizeof(recvline));
        bzero(sending,sizeof(sending));
		ack=0;
		number++;
	}
	char finish[]="finish!!!";
	while(!ack){
        sendto(sockfd,finish,strlen(finish),0,servaddr,servlen);
        if((n=recvfrom(sockfd,recvline,256,0,servaddr,&servlen))<0){
            if(errno==EWOULDBLOCK){
                cout<<"No ack! Retransmit!"<<endl;
                packetlost++;
            }
            else{
                perror("recvfrom error");
            }
        }
        else{
            cout<<recvline<<endl;
            ack=1;
        }
	}
	cout<<"packetlost: "<<packetlost<<endl;
}
