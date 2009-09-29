#include "nge_net_common.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#endif

int socket_create(int af,int type,int protocol)
{
	return socket(af, type, protocol);
}

int socket_setopt(int fd,int level,int optname,const char* optval,int optlen)
{
	return setsockopt(fd, level, optname,optval,optlen);
}

int socket_setnoblock(int fd)
{
#ifdef WIN32//win32
	unsigned long argp =1;
	int ret = ioctlsocket(fd,FIONBIO,&argp);
	if(ret == SOCKET_ERROR)
	{
		socket_close(fd);
		return -1;//SET_SOCK_OPT_FAIL
	}
#else //unix
	int flags;
	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0){
		socket_close(fd);
		return -1;
	}
#endif
	return 1;
}

int socket_recv(int fd,const char *buf,int len)
{
	int rlen = 0;
#ifdef WIN32
	rlen = recv(fd,(char *)buf,len,0);
#else        
	rlen = read(fd, buf, len);
#endif
	return rlen;	
}

int socket_send(int fd,const char *buf,int len)
{
	int wlen;
#ifdef WIN32	
	wlen = send(fd,(const char *)buf,len,0);
#else
	wlen = write(fd, buf, len);
#endif
	return wlen;
}

int socket_close(int fd)
{
#ifdef WIN32	
	return closesocket(fd);
#else
	return close(fd);
#endif
}




