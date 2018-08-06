/*************************************************************************
	> File Name: BOBO_Server.cpp
	> Author: 
	> Mail: 
	> Created Time: Thu 19 Jul 2018 08:04:40 AM EDT
 ************************************************************************/

#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<thread>
#include<iostream>
#include<thread>
#include<list>
using namespace std;
#define PORT 7000
#define QUEUE 20
#define IP "127.0.0.1"
struct sockaddr_in servaddr;
struct sockaddr_in clientaddr;
socklen_t len=sizeof(clientaddr);
int s;
int z=0;
list<int> li;
int conns[2]={};
int a=0;
void sendMess();
struct sendmsg
{
    char name[1024];
    char msg[1024];

};

struct sendmsg s3;

void getconn(){
    while(1){
        int conn=accept(s,(struct sockaddr*)&clientaddr,&len);
        li.push_back(conn);
        // conns[z]=conn;
        //z++;
        cout<<conn<<endl;
    }
    
}


void getData(){
    struct timeval tv;
    tv.tv_sec=5;
    tv.tv_usec=0;
    while(1){
        std::list<int>::iterator it;
            
         for(it=li.begin();it!=li.end();++it){
        
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(0,&rfds);
            int maxfd=0;
            int retval=0;
            FD_SET(*it,&rfds);
            FD_ISSET(*it,&rfds);
            if(maxfd<*it){
                maxfd=*it;
            }
            // cout<<maxfd<<endl;
            retval=select(maxfd+1,&rfds,NULL,NULL,&tv);
            if(retval==-1){
                cout<<"error"<<endl;
            }
            else if(retval==0){
            
            }
            else{
                //char buf[1024];
                struct sendmsg s2;
                
                //memset(buf,0,sizeof(buf));
                memset(&s2,0,sizeof(struct sendmsg));
                int len=recv(*it,(char*)&s2,sizeof(struct sendmsg),0);
                //memcpy(&s2,buf,sizeof(struct sendmsg));
                s3=s2;
                cout<<s2.name<<"  :  "<<s2.msg<<endl;
               sendMess(); 

            }
        }
        sleep(1);
    }
    
}

void sendMess(){
  //  while(1){
        //char buf[1024];
        //fgets(buf,sizeof(buf),stdin);
        

        std::list<int>::iterator it;
        for(it=li.begin();it!=li.end();++it){

        //for(int j=0;j<z;j++){

            send(*it,(char*)&s3,sizeof(struct sendmsg),0);

            //send(*it,buf,sizeof(buf),0);
            a++;
        }
        cout<<"time:"<<a;
   // }

}

int main(){
    int opt=1;
    s=socket(AF_INET,SOCK_STREAM,0);
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    //servaddr.sin_addr.s_addr=inet_addr(IP);
    servaddr.sin_addr.s_addr=htons(INADDR_ANY);
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(bind(s,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1){
        perror("bind");
        exit(1);
    }
    if(listen(s,20)==-1){
        perror("listen");
        exit(1);
    }
    len=sizeof(clientaddr);

    std::thread t(getconn);
    t.detach();
   // std::thread t1(sendMess);
   // t1.detach();
    std::thread t2(getData);
    t2.detach();
    while(1){
        
    }
    return 0;

    
}

