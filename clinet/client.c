#include <pthread.h>
#include "tcp_socket.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "http_download.h"
#include <semaphore.h>
#include<sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
struct tcp_fifo
{
    int socket;
    int fd_r;
};
static pthread_t cli_data_proce_thread_tid;

void* tranFile(void* sendSocket, char* file_name);

void* process_client_data(void* arg)
{
    struct tcp_fifo *tmp;
    tmp = (struct tcp_fifo*)arg;
    int client_fd = tmp->socket;
    int fd_r = tmp->fd_r;
    char web_data[2048] = { 0 };
    read(fd_r, web_data, sizeof(web_data));//��ȡcjson�����������б�
    char buf[256] = { 0 };
    int send_len;
    char ret;
    printf("this is client\n");
    printf("%s\n", web_data);
    while (1)
    {
        printf("star\n");
        bzero(buf, sizeof(buf));
        int recv_len = tcp_blocking_recv(client_fd, buf, sizeof(buf));
        if (recv_len <= 0)
        {
            printf("recv error!\n");
            tcp_close(client_fd);
            return NULL;
        }
        printf("client_fd = %d, recv : %s\n", client_fd, buf);
        //��������б�,�����б�
        //�б�һ��һ�з���
        if (strcmp(buf, "list") == 0)
        {
            printf("send list\n");
            //���������б������Ϣ
            printf("send list %s\n", web_data);
            send_len = tcp_send(client_fd, web_data, strlen(web_data));
            if (send_len <= 0)
            {
                printf("send error!\n");
                tcp_close(client_fd);
                return NULL;
            }
            else
            {
                printf("send success! send\n");
            }
        }
        else if ((ret = strstr(web_data, buf)) != NULL)
        {
            //��ø����������󣬴����Ӧ�ĸ���
            printf("send file\n");
            //tranFile(&client_fd, ret);
        }
        else if (strcmp(buf, "json") == 0)
        {
            //��ø����������󣬴����Ӧ�ĸ���
            printf("send file\n");
            //tranFile(&client_fd, "JSON_MESSAGE.json");
        }
        else
        {
            printf("erro\n");
            bzero(buf, sizeof(buf));
            sprintf(buf, "error get\n");
            send_len = tcp_send(client_fd, buf, strlen(buf));

        }
        printf("nono\n");
        sleep(1);
    }
}

//��ʼ����ȡ�ܵ�
int fifo_read_init(const char* str)
{
    /* ����1:���������ܵ� */
    if (access(str, F_OK) != 0)
    {
        if (mkfifo(str, 0666) < 0)
        {
            perror("cannot create fifo!");
            exit(1);
        }
    }

    int fd = -1;
    /* ����2:�������ܵ��������÷�������־ */
    fd = open(str, O_RDONLY | O_NONBLOCK, 0);
    if (fd < 0)
    {
        perror("open fifo ui faild! ");
        exit(1);
    }
    return fd;
}

int main(int argc, char* argv[])
{
    int server_fd = tcp_init(NULL, 4321);
    int fd_r = -1;
    fd_r = fifo_read_init("/home/owen/test/1.txt");
    struct tcp_fifo tmp;
    tmp.fd_r = fd_r;
    while (1)
    {
        int new_fd = tcp_accept(server_fd);
        //��ͻ��˷���
        // �����ͻ������ݴ����߳�
        tmp.socket = new_fd;
        if (new_fd >= 0)
        {
            int ret = pthread_create(&cli_data_proce_thread_tid, NULL, process_client_data, (void*)&tmp);
            if (ret != 0)
            {
                perror("pthread_create from client");
                exit(EXIT_FAILURE);
            }
        }
    }
    close(fd_r);
    tcp_close(server_fd);
    pthread_join(cli_data_proce_thread_tid, NULL);
    return 0;
}



//�����ļ�����
void* tranFile(void* sendSocket, char* file_name)
{
    int sock = *(int*)sendSocket;
    unsigned char buf[1024];
    int buflen = 1024, k = 0;
    char file[256] = { 0 };
    sprintf(file, file_name);
    FILE* source;
    source = fopen(file, "rb");
    if (source == NULL)
    {
        printf("open file error\n");
        close(sock);
    }
    else
    {
        printf("begin tran file %s\n", file);
        //�Ȱ��ļ������͹�ȥ
        int iSendLen = write(sock, file, strlen(file));
        if (iSendLen < 0)
        {
            printf("send file name error\n");
            fclose(source);
            close(sock);
            return 0;
        }
        //�ȴ��Է�׼����
        memset(buf, 0, buflen);
        int iRealLength = read(sock, buf, 1024);
        printf("get string: %s , %d\n", buf, iRealLength);
        //��ʼ�����ļ�����
        memset(buf, 0, buflen);
        while ((k = fread(buf, 1, buflen, source)) > 0)
        {
            printf("tran file length:%d", k);
            //ѭ������ֱ�����ļ�����
            int iSendLen = write(sock, buf, k);
            if (iSendLen < 0)
            {
                printf("send file erroe\n");
                break;
            }
            memset(buf, 0, buflen);
        }
        printf("tran file finish\n");
        fclose(source);
        close(sock);
    }
    return 0;
}