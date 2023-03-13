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

resp_header_def resp;//ͷ��Ϣ

#if 0
���ط�Ϊ���¼�������

1�����������ص�ַ�е��������ļ���
2��ͨ��������ȡ��������IP��ַ
3����Ŀ���������������
4������http����ͷ�����䷢�͵�������
5���ȴ���������ӦȻ�������Ӧͷ
6��������Ӧͷ, �жϷ�����, ���뿪��Ӧͷ, ������Ӧ�������������ֽ���ʽд���ļ�, ����������ͷ��������\n\r�ֿ�

#endif

#define print_LOG(format, ...)     {\
	printf(format, ##__VA_ARGS__);}

//������ַ
//����url������������˿ڡ��ļ���
static int Parsing_urls(char* url, char* domain, int* port, char* filename)
{
    int i, j, start;
    char* patterns[] = { "http://", "https://", NULL };
    *port = 80;
    //��������������http://����https://��/������
    for (i = 0; patterns[i]; i++)
    {
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
        {
            start = strlen(patterns[i]);
        }
    }
    //��������
    j = 0;
    for (i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
    {
        domain[j] = url[i];
    }
    domain[i] = '\0';

    //�����˿�,ð�ź�����Ƕ˿�
    char pos = strstr(domain, ":");
    if (pos)
    {
        sscanf(pos, ":%d", port);
    }

    //����ж˿ڣ���Ҫɾ��
    for (i = 0; i < (int)strlen(domain); i++)
    {
        if (domain[i] == ':')
        {
            domain[i] = '\0';
            break;
        }
    }

    //���������ļ���,/��������ļ���
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

//ͨ��������ȡip
static int Domain_to_ip(char* domain, char* ip)
{
    int i;
    struct hostent* host = gethostbyname(domain);

    if (host == 0)
    {
        *ip = NULL;
        return -1;
    }

    //�ҵ���Ϊ�յĵ�ַ
    for (i = 0; host->h_addr_list[i]; i++)
    {
        strcpy(ip, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
        break;
    }

    return 0;
}

//��������ͷ��Ϣ
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
        //��̬���������С
        if ((lencnt + len) > mem_size)//�жϻ����Ƿ���
        {
            //���·����ڴ�
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
        //�ҵ���Ӧͷ��ͷ����Ϣ, ����"\n\r"Ϊ�ָ��
        int flag = 0;
        int i = strlen(response) - 1;
        for (; response[i] == '\n' || response[i] == '\r';
            i--, flag++);
        {
            if (flag == 4)//�����4�Σ�û�ҵ�
                break;
        }

        lencnt += len;
    }

    free(rcvbuff);

    return response;
}

//��ȡ�ظ�ͷ����Ϣ
static int get_resp_header(const char* response, resp_header_def* resp)
{
    //����HTTP/
    char* pos = strstr(response, "HTTP/");
    if (pos)
        sscanf(pos, "%*s %d", &resp->status_code);//����״̬��

    pos = strstr(response, "Content-Type:");//������������
    if (pos)
        sscanf(pos, "%*s %s", resp->content_type);

    pos = strstr(response, "Content-Length:");//���ݵĳ���(�ֽ�)
    if (pos)
        sscanf(pos, "%*s %ld", &resp->content_length);

    return 0;
}

//��ӡ����
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
    fflush(stdout);//�������

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
    //����4k����
    char* buf = (char*)malloc(mem_size * sizeof(char));

    //��ȡ�ļ�
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

//http�����ļ�����
void* http_download_file(char* url)
{
    //char url[2048] = "127.0.0.1";
    char domain[64] = { 0 };
    char ip_addr[16] = { 0 };
    int port = 80;
    char file_name[256] = { 0 };
    char header[2048] = { 0 };
    char* response = 0;

    //��������
    Parsing_urls(url, domain, &port, file_name);

    //����ip
    Domain_to_ip(domain, ip_addr);
    print_LOG("download:\n url:%s\n domain:%s\n ip:%s\n port:%d\n filename:%s\n",
        url, domain, ip_addr, port, file_name);

    //������������
    Set_request_headers(header, url, domain);
    if (tcp_client_init(ip_addr, port) == -1)
    {
        print_LOG("connect server fail\n");
        return -1;
    }
    tcp_client_send(header, strlen(header));
    print_LOG("\nsend request\n");
    //print_LOG("send request:%s\n", header);

    //�����ظ�
    response = get_response();
    get_resp_header(response, &resp);
    print_LOG("response:%s\n", response);
    free(response);

    strcpy(resp.file_name, file_name);
    print_LOG("\nres:\n content_length:%d\n file_name:%s\n Content-Type:%s\n",
        resp.content_length,
        resp.file_name,
        resp.content_type);

    //�����ļ�
    print_LOG("\ndownload %s start ...\n", resp.file_name);
    download_writefile();

}

