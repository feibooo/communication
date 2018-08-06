#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <process.h>
#pragma pack(8)
#include <winsock2.h>
#pragma pack()
using namespace std;


#pragma comment(lib,"ws2_32.lib")
#define PORT 7000
#define IP_ADDRESS "192.168.31.227"
#define MAX_LEN 1024


struct  sendmsg
{
	char name[MAX_LEN];
	char msg[MAX_LEN];	
};

void recv(PVOID pt)
{
	SOCKET s_HOST=*((SOCKET *)pt);
	while(1)
	{
		char recvbuf[MAX_LEN];
		memset(recvbuf,0,sizeof(recvbuf));
		struct sendmsg s3;
		int retval;
		retval=recv(s_HOST,(char*)&s3,sizeof(sendmsg),0);
		//retval=recv(s_HOST,recvbuf,sizeof(recvbuf),0);
		if(retval==SOCKET_ERROR)
		{
			int err=WSAGetLastError();
			if(err==WSAEWOULDBLOCK)
			{
				Sleep(1000);
				
				continue;
			}
			else if(err==WSAETIMEDOUT||err==WSAENETDOWN||err==WSAECONNRESET)
			{
				cout<<"recv error"<<endl;
				closesocket(s_HOST);
				WSACleanup();
				return ;

			}
		}
		Sleep(100);
		cout<<s3.name<<" : "<<s3.msg<<endl;
		
		//break;
		//cout<<recvbuf;
		
	}

}


int main()
{
	WSADATA s;
	SOCKET ClientSocket;
	struct sockaddr_in ClientAddr;
	int ret=0;
	char SendBuffer[MAX_LEN];
	fd_set rfds;
	struct timeval tv;
	int retval,maxfd;
	int i=0;
	struct sendmsg s1;
	


	memset(s1.name,0,sizeof(s1.name));
	cout<<"input your name:"<<endl;
	//while(s1.name[i-1]!=':')
	//{
		cin>>s1.name;
	//	i++;
	//}
	
	


	//初始化WINDOWS SOCKET
	if(WSAStartup(MAKEWORD(2,2),&s)!=0)
	{
		cout<<"Init windows socket error"<<GetLastError();
		
		return -1;
	}


	//定义socketfd
	//定义sockaddr_in
	//connect
	//recv&&send
	//关闭
	ClientSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	/*if (ClientSocket==INVALID_SOCKET)
	{
		cout<<"create socket error"<<GetLastError()<<endl;
		
		return -1;
	}
	*/

	ClientAddr.sin_family=AF_INET;
	ClientAddr.sin_port=htons(PORT);
	ClientAddr.sin_addr.s_addr=inet_addr(IP_ADDRESS);


	int iM=1;
	retval=ioctlsocket(ClientSocket,FIONBIO,(u_long FAR*)&iM);
	if(retval==SOCKET_ERROR)
	{
		cout<<"ioctlsocket error"<<endl;
		WSACleanup();
		return -1;

	}


	while(1)
	{
		retval=connect(ClientSocket,(LPSOCKADDR)&ClientAddr,sizeof(ClientAddr));
		if(retval==SOCKET_ERROR)
		{
			int err=WSAGetLastError();
			if(err==WSAEWOULDBLOCK||err==WSAEINVAL)
			{
				Sleep(1);
				cout<<"CHECK connect"<<endl;
				continue;
			}
			else if(err==WSAEISCONN)
			{
				break;
			}
			else
			{
				cout<<"connect error"<<endl;
				closesocket(ClientSocket);
				WSACleanup();
				return -1;
			}

		}
	}


	unsigned long threadId=_beginthread(recv,0,&ClientSocket);


	while(1)
	{
		//char SendBuffer[MAX_LEN];
		//struct sendmsg s1;
		//memset(SendBuffer,0,sizeof(SendBuffer));
		//fgets(s1.msg,sizeof(s1.msg),stdin);
		//strcpy(s1.name,"feibooo");
		//memcpy(SendBuffer,&s1,sizeof(sendmsg));
		//cout<<s1.name<<s1.msg<<endl;
		cout<<"input your message"<<endl;
		cin>>s1.msg;
		//cout<<s1.name<<s1.msg;

		


		while(1)
		{
			
			retval=send(ClientSocket,(char*)&s1,sizeof(sendmsg),0);
			if (retval==SOCKET_ERROR)
			{
				int err=WSAGetLastError();
				if (err==WSAEWOULDBLOCK)
				{
					Sleep(5);
					continue;
				}
				else
				{
					cout<<"send error"<<endl;
					closesocket(ClientSocket);
					WSACleanup();
					return -1;
				}
			}
			break;
		}

	}
	return 0;

}








