/*************************************************************************
	> File Name: Csocket.cpp
	> Author: 
	> Mail: 
	> Created Time: Tue 07 Aug 2018 11:19:55 AM EDT
 ************************************************************************/

#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<netdb.h>
#include<stdio.h>
#include<thread>
#include<list>
#include<netinet/tcp.h>


class CservSocket
{
    public:
        CservSocket();
        virtual ~CservSocket();
        std::string getLocalAddr();
        std::string getClientAddr();
        int getAcceptSock();
        bool Accept();
        bool Listen(int port);
        bool Send(char* sendbuf,int len);
        bool Recv(char* recvbuf,int len,int timeout);


    private:
        bool Close();
        bool setSolinger(bool dolinger,int seconds);
        bool setIntOptions(int option,int value);
        bool setTimeout(int option,int millisenconds);
        bool setNonBlock(bool isnonsocket);
        bool setNoDelay(bool nodelay);
    private:
        char ip[20];
        int port;
        int sockfd;
        int connsockfd;
        struct sockaddr_in servAddr,cliAddr;

};


//初始化
CservSocket::CservSocket():port(0),sockfd(-1),connsockfd(-1)
{
    memset(ip,0,20);
}


CservSocket::~CservSocket()
{
    if(!Close())
    {
        std::cout<<"Close error"<<std::endl;
    }
}

//关闭
bool CservSocket::Close()
{
    if(sockfd)
        close(sockfd);
    else
        return false;
    if(connsockfd)
        close(connsockfd);
    else
        return false;
    sockfd=-1;
    connsockfd=-1;
    port=-1;
    return true;
}

//监听端口
bool CservSocket::Listen(int port)
{
    if(port<0)
    {
        std::cout<<"port error"<<std::endl;
        return false;
    }
    sockfd=::socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        std::cout<<"create socket error"<<std::endl;
        return false;
    }

    getLocalAddr();
    servAddr.sin_family=AF_INET;
    if(ip)
        servAddr.sin_addr.s_addr=inet_addr(ip);
    else
        servAddr.sin_addr.s_addr=htonl(INADDR_ANY);

    servAddr.sin_port=htons(port);
    setSolinger(false,0);
    setIntOptions(SO_REUSEADDR,1);
    setIntOptions(SO_KEEPALIVE,1);
    setIntOptions(SO_SNDBUF,64000);
    setIntOptions(SO_RCVBUF,64000);
    setNoDelay(true);
    if(::bind(sockfd,(struct sockaddr*)&servAddr,sizeof(servAddr))<0)
    {
        std::cout<<"bind error"<<std::endl;
        return false;
    }
    if(::listen(sockfd,0)<0)
    {
        std::cout<<"listen error"<<std::endl;
        return false;
    }
    return true;
}

//等待连接
bool CservSocket::Accept()
{
    socklen_t len=sizeof(cliAddr);
    if((connsockfd=::accept(sockfd,(struct sockaddr*)&cliAddr,&len))<0)
    {
        return false;

    }
    return true;
}

//获取本机ip
std::string CservSocket::getLocalAddr()
{
    char ipaddr[20]={'\0'};
    const char*shellstr="ifconfig | sed -n '2p' | awk -F'[ :]+''printf $4'";
    FILE *fp=popen(shellstr,"r");
    fread(ipaddr,sizeof(char),sizeof(ipaddr),fp);
    if(ipaddr)
    {
        strcpy(ip,ipaddr);
    }
    pclose(fp);
    return std::string(ip);
}


std::string CservSocket::getClientAddr()
{
    char cli_ip[16];
    socklen_t size=sizeof(cliAddr);
    if(getpeername(sockfd,(sockaddr*)&cliAddr,&size))
    {
        strcpy(cli_ip,"0.0.0.0");
    }
    else
    {
        sprintf(cli_ip,"%d,%d,%d,%d",((unsigned char *)&cliAddr.sin_addr)[0],
                                    ((unsigned char *)&cliAddr.sin_addr)[1],
                                    ((unsigned char *)&cliAddr.sin_addr)[2],
                                    ((unsigned char *)&cliAddr.sin_addr)[3]);
    }
    return cli_ip;
}


int CservSocket::getAcceptSock()
{
    return connsockfd;
}


bool CservSocket::setIntOptions(int option,int value)
{
    bool res=false;
    if(sockfd)
    {
        res=(setsockopt(sockfd,SOL_SOCKET,option,(const void*)&value,sizeof(value))==0);
    }
    return res;
}


bool CservSocket::setSolinger(bool dolinger,int seconds)
{
    bool res=false;
    if(sockfd)
    {
        struct linger ling;
        ling.l_onoff=dolinger?1:0;
        ling.l_linger=seconds;
        res=(setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(const char*)&ling,sizeof(struct linger))==0);
        res=true;
    }
    return res;
}


bool CservSocket::setTimeout(int option,int millisenconds)
{
    bool res=false;
    if(sockfd)
    {
        struct timeval timeout;
        timeout.tv_sec=millisenconds/1000;
        timeout.tv_usec=(millisenconds%1000)*1000000;
        res=(setsockopt(sockfd,SOL_SOCKET,option,(const void*)&timeout,sizeof(timeout))==0);
        res=true;
    }
    return res;
}


bool CservSocket::setNonBlock(bool isnonsocket)
{
    bool res=false;
    if(sockfd)
    {
        int oldfd=fcntl(sockfd,F_GETFL);
        res=(fcntl(sockfd,F_SETFL,oldfd | O_NONBLOCK)<0);
        res=true;
    }
    return res;
}


bool CservSocket::setNoDelay(bool nodelay)
{
    bool res=false;
    if(sockfd)
    {
        int ndelay=nodelay?1:0;
        res=(setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,(const void*)&ndelay,sizeof(ndelay))==0);
        res=true;
    }
    return res;
}


bool CservSocket::Send(char *sendbuf,int len)
{
    if(sockfd<0 || sendbuf==NULL || len<0)
    {
        return false;
    }
    int dataleft=len,total=0,ret=0;
    for(;dataleft>0;)
    {
        ret=::send(connsockfd,sendbuf+total,dataleft,0);
        if(ret<0)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR)
            {
                usleep(500000);
                ret=0;
            }
        }
        total+=ret;
        dataleft=len-total;
    }
    return total==len;
}


bool CservSocket::Recv(char * recvbuf,int len,int timeout)
{
    if(sockfd<0 || recvbuf==NULL || len<0)
    {
        return false;
    }
    fd_set rfds;
    struct timeval interval;
    interval.tv_sec=timeout;
    interval.tv_usec=0;
    int recvlen=0;
    for(;;)
    {
        FD_ZERO(&rfds);
        FD_SET(connsockfd,&rfds);
        int res=::select(connsockfd+1,&rfds,NULL,NULL,&interval);
        if(res==0)
        {
            continue;
        }
        if(res<0)
        {
            ::close(connsockfd);
            connsockfd=-1;
            return false;
        }
        else
        {
            if(FD_ISSET(connsockfd,&rfds))
            {
                recvlen=::recv(connsockfd,recvbuf,len,0);
                break;
            }

        }
    }
    return recvlen==len;
}
