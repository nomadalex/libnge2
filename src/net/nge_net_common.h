#ifndef NGE_NET_COMMON_H_
#define NGE_NET_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 创建一个socket
 *@param int af,地址簇指定,通常填写AF_INET
 *@param int type,socket类型,可填写SOCK_STREAM(TCP)和SOCK_DGRAM(UDP)
 *@param int protocol,协议簇,通常填0
 *@return int,socket句柄,用此句柄做后续操作,<=0代表出错
 */
int socket_create(int af,int type,int protocol);
/**
 * 设置socket选项,通常是不需要设置选项的
 *@param int fd,socket handle,填入由socket_create()创建的handle
 *@param int level,选项定义的层次:目前仅支持SOL_SOCKET和IPPROTO_TCP层次.
 *@param int optname,需设置的选项,较常用的是SO_REUSEADDR
 *@param const char* optval,指向存放选项值的缓冲区
 *@param int optval,缓冲区的长度
 *@return int,返回值小于等于0出错
 */
int socket_setopt(int fd,int level,int optname,const char* optval,int optlen);
/**
 * 设置网络为非阻塞模式
 *@param int fd,socket handle,填入由socket_create()创建的handle
 *@return int, 返回值小于等于0出错
 */
int socket_setnoblock(int fd);
/**
 *@param int fd,socket handle,填入由socket_create()创建的handle
 *@param const char* buf,接收buf缓存
 *@param int len,buf的长度
 *@return int, 返回实际接收的长度,<=0为出错,对TCP来说(等于0为Client正常断开链接,小于0为发生其他错误)
 */
int socket_recv(int fd,const char *buf,int len);
/**
 *@param int fd,socket handle,填入由socket_create()创建的handle
 *@param const char* buf,发送buf缓存
 *@param int len,buf的长度
 *@return int, 返回实际发送的长度,<=0为出错,在设置noblock后直接返回否则为阻塞发送
 */
int socket_send(int fd,const char *buf,int len);
/**
 * 关闭一个socket
 *@param int fd,socket handle,填入由socket_create()创建的handle
 *@return int,返回值大于0成功
 */
int socket_close(int fd);

#ifdef __cplusplus
}
#endif

#endif
