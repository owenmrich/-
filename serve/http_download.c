#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include "tcp.h"


typedef struct {
    int status_code;//HTTP/1.1 '200' OK
    char content_type[128];//Content-Type: application/gzip
    long content_length;//Content-Length: 11683079
    char file_name[256];
}resp_header_def;

resp_header_def resp;//头信息

#if 0
下载分为以下几个过程

1、解析出下载地址中的域名和文件名
2、通过域名获取服务器的IP地址
3、与目标服务器建立连接
4、构建http请求头并将其发送到服务器
5、等待服务器响应然后接收响应头
6、解析响应头, 判断返回码, 分离开响应头, 并且响应的正文内容以字节形式写入文件, 正文内容与头部用两个\n\r分开

#endif

#define print_LOG(format, ...)     {\
	printf(format, ##__VA_ARGS__);}

//解析网址
//输入url，输出域名、端口、文件名
static int Parsing_urls(char* url, char* domain, int* port, char* filename)
{
    int i, j, start;
    char* patterns[] = { "http://", "https://", NULL };
    *port = 80;
    //解析域名，就是http://或者https://到/的内容
    for (i = 0; patterns[i]; i++)
    {
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
        {
            start = strlen(patterns[i]);
        }
    }
    //复制域名
    j = 0;
    for (i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
    {
        domain[j] = url[i];
    }
    domain[i] = '\0';

    //解析端口,冒号后面就是端口
    char pos = strstr(domain, ":");
    if (pos)
    {
        sscanf(pos, ":%d", port);
    }

    //如果有端口，需要删掉
    for (i = 0; i < (int)strlen(domain); i++)
    {
        if (domain[i] == ':')
        {
            domain[i] = '\0';
            break;
        }
    }

    //解析下载文件名,/后面就是文件名
    j = 0;
    for (i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            j = i + 1;
            memcpy(filename, &url[j], strlen(&url[j]));
        }
    }
    i = strlen(&url[j]);
    filename[i] = '\0';

    return 0;
}

//通过域名获取ip
static int Domain_to_ip(char* domain, char* ip)
{
    int i;
    struct hostent* host = gethostbyname(domain);

    if (host == 0)
    {
        *ip = NULL;
        return -1;
    }

    //找到不为空的地址
    for (i = 0; host->h_addr_list[i]; i++)
    {
        strcpy(ip, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
        break;
    }

    return 0;
}

//构建请求头信息
static int Set_request_headers(char* header, char* url, char* domain)
{
    int ret = 0;
    sprintf(header, \
        "GET %s HTTP/1.1\r\n"\
        "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
        "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
        "Host:%s\r\n"\
        "Connection:close\r\n"\
        "\r\n"\
        , url, domain);
    ret = strlen(header);

    return ret;
}
static char* get_response()
{
    int len, lencnt = 0, mem_size = 4096;
    char* rcvbuff = (char*)malloc(mem_size * sizeof(char));
    char* response = (char*)malloc(mem_size * sizeof(char));

    while ((len = tcp_client_rcv(rcvbuff, 1)) != 0)
    {
        //动态调整缓存大小
        if ((lencnt + len) > mem_size)//判断缓存是否超限
        {
            //重新分配内存
            mem_size *= 2;
            char* tmp = (char*)realloc(response, mem_size * sizeof(char));
            if (tmp == NULL)
            {
                printf("realloc fail\n");
                exit(-1);
            }
            response = tmp;
        }
        rcvbuff[len] = '\0';
        strcat(response, rcvbuff);
        //找到响应头的头部信息, 两个"\n\r"为分割点
        int flag = 0;
        int i = strlen(response) - 1;
        for (; response[i] == '\n' || response[i] == '\r';
            i--, flag++);
        {
            if (flag == 4)//最多找4次，没找到
                break;
        }

        lencnt += len;
    }

    free(rcvbuff);

    return response;
}

//获取回复头的信息
static int get_resp_header(const char* response, resp_header_def* resp)
{
    //查找HTTP/
    char* pos = strstr(response, "HTTP/");
    if (pos)
        sscanf(pos, "%*s %d", &resp->status_code);//返回状态码

    pos = strstr(response, "Content-Type:");//返回内容类型
    if (pos)
        sscanf(pos, "%*s %s", resp->content_type);

    pos = strstr(response, "Content-Length:");//内容的长度(字节)
    if (pos)
        sscanf(pos, "%*s %ld", &resp->content_length);

    return 0;
}

//打印进度
int progress_bar(int x)
{
    int i;
    char tmp[100] = { 0 };
    static int x_old = 0;

    if (x == x_old)
    {
        return 0;
    }
    x_old = x;
    i = x / 2;
    if (i > 50)
        i = 50;
    memset(tmp, '=', i);
    printf("\r%d%[%s]", x, tmp);
    fflush(stdout);//立刻输出

    return 0;
}

static int download_writefile()
{
    int length = 0;
    int mem_size = 4096;//mem_size might be enlarge, so reset it
    int buf_len = mem_size;//read 4k each time
    int len = 0;

    int fd = open(resp.file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
    if (fd < 0)
    {
        print_LOG("Create file failed\n");
        exit(0);
    }
    //申请4k缓存
    char* buf = (char*)malloc(mem_size * sizeof(char));

    //读取文件
    while ((len = tcp_client_rcv(buf, buf_len)) != 0 && length < resp.content_length)
    {
        write(fd, buf, len);
        length += len;
        progress_bar((length * 100 / resp.content_length));
    }

    if (length == resp.content_length)
    {
        print_LOG("\nDownload successful ^_^\n\n");
    }
    else
    {
        print_LOG("Finished length:%d,resp.content_length:%d\n",
            length, resp.content_length);
    }

    close(fd);

    return 0;
}

//http下载文件函数
void* http_download_file(char* url)
{
    //char url[2048] = "127.0.0.1";
    char domain[64] = { 0 };
    char ip_addr[16] = { 0 };
    int port = 80;
    char file_name[256] = { 0 };
    char header[2048] = { 0 };
    char* response = 0;

    //解析域名
    Parsing_urls(url, domain, &port, file_name);

    //解析ip
    Domain_to_ip(domain, ip_addr);
    print_LOG("download:\n url:%s\n domain:%s\n ip:%s\n port:%d\n filename:%s\n",
        url, domain, ip_addr, port, file_name);

    //发送下载请求
    Set_request_headers(header, url, domain);
    if (tcp_client_init(ip_addr, port) == -1)
    {
        print_LOG("connect server fail\n");
        return -1;
    }
    tcp_client_send(header, strlen(header));
    print_LOG("\nsend request\n");
    //print_LOG("send request:%s\n", header);

    //解析回复
    response = get_response();
    get_resp_header(response, &resp);
    print_LOG("response:%s\n", response);
    free(response);

    strcpy(resp.file_name, file_name);
    print_LOG("\nres:\n content_length:%d\n file_name:%s\n Content-Type:%s\n",
        resp.content_length,
        resp.file_name,
        resp.content_type);

    //下载文件
    print_LOG("\ndownload %s start ...\n", resp.file_name);
    download_writefile();

}

