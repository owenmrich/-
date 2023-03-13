//-------tcp相关头文件------
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> //close()


static int socket_fd = 0;

//tcp_client_init()
//1、创建socket
//2、配置为客户端
//3、配置要连接的服务器ip和端口以及协议类型
//4、连接服务器
//5、收发数据
//6、关闭连接
int tcp_client_init(char* ip, int port)
{
	int ret;
	//1 2
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		printf("create socket fail\n");
		return -1;
	}
	//3 
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;//IPv4协议	
	servaddr.sin_port = htons(port);//服务器端口号	
	servaddr.sin_addr.s_addr = inet_addr(ip);//设置服务器
	//4
	ret = connect(socket_fd, &servaddr, sizeof(servaddr));
	if (ret == -1)
	{
		printf("connect %s fail\n", ip);
		return -1;
	}
}

//tcp_client_send()
int tcp_client_send(char* buff, int len)
{
	int ret = 0;
	ret = write(socket_fd, buff, len);
	return ret;
}

//tcp_client_rcv()
int tcp_client_rcv(char* buff, int len)
{
	int ret;

	ret = read(socket_fd, buff, len);

	return ret;
}

//tcp_client_close()
int tcp_client_close()
{
	close(socket_fd);
}


#define CLENT_NUM 2
struct sockaddr_in sSever_c_sd[CLENT_NUM];
static int socket_s_fd = 0;
static int socket_c_fd[CLENT_NUM] = { 0 };
//tcp_server_init()
#if 0
1、创建socket
2、设置本地ip和端口以及协议类型
3、绑定
4、监听
5、等待客户端连接
6、收发数据
7、关闭连接
#endif

int tcp_server_init(int port)
{
	int ret;
	//1 
	socket_s_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_s_fd == -1)
	{
		printf("create socket fail\n");
		return -1;
	}
	else
	{
		printf("create socket ok\n");
	}
	//2 
	struct sockaddr_in local_addr, c_addr;
	local_addr.sin_family = AF_INET;//IPv4协议	
	local_addr.sin_port = htons(port);//服务器端口号	
	local_addr.sin_addr.s_addr = "127.0.0.1";//设置服务器ip
	//3
	ret = bind(socket_s_fd, &local_addr, sizeof(local_addr));
	if (ret == -1)
	{
		printf("bind fail\n");
		close(socket_s_fd);
		return -1;
	}
	else
	{
		printf("bind ok\n");
	}
	//4
	ret = listen(socket_s_fd, 3);
	if (ret == -1)
	{
		printf("listen fail\n");
		close(socket_s_fd);
		return -1;
	}
	else
	{
		printf("listen ok\n");
	}
	//5
	socklen_t addrlen = 0;
	while (1)
	{
		printf("wait client conect\n");
		socket_c_fd[0] = accept(socket_s_fd, &c_addr, &addrlen);
		if (addrlen != 0)
			break;
		sleep(1);
	}
	printf("client conect\n");

	return 0;
}

//tcp_server_send()
int tcp_server_send(char c, char* buff, int len)
{
	write(socket_c_fd[c], buff, len);
}

//tcp_server_rcv()
int tcp_server_rcv(char c, char* buff, int len)
{
	int ret;

	ret = read(socket_c_fd[c], buff, len);
	return ret;
}

//tcp_server_close()
int tcp_server_close()
{
	close(socket_s_fd);
}

