#include <pthread.h>
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
#include "cJSON.h"

//��ʼ��д��ܵ�
int fifo_write_init(const char* str)
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
	fd = open(str, O_WRONLY, 0);
	if (fd < 0)
	{
		perror("open fifo ui faild! ");
		exit(1);
	}
	return fd;
}

struct cjson_data
{
	char name[100];
	char style[100];
	char singer[100];
};
int main(int argc, char* argv[])
{
	struct cjson_data data[4];
	int fd_w = -1;
	fd_w = fifo_write_init("/home/owen/test/1.txt");
	char flag [1];
	char flag_final [1];
	strcpy(flag, ":");
	strcpy(flag_final, "|");
    if (argc < 2)
    {
        printf("./cs url\n");
		return -1;
    }
    while (1)
    {

        //�Ȼ���ļ�url ��ͨ��url�����ļ��б�
        http_download_file(argv[1]);
		int fd = open("JSON_MESSAGE.json", O_RDWR);
		if (fd < 0)
		{
			perror("open fail\n");
			return -1;
		}

		//��ȡ�ļ��е�����
		char buf[2048] = { 0 };
		int ret = read(fd, buf, sizeof(buf));
		if (ret == -1)
		{
			perror("read error");
			return -1;
		}

		close(fd);
		printf("%s", buf);
		//�Ѹ��ַ�������ת����JSON����(����)  ��ʼ�Ķ���Ϊ�����Ķ���
		cJSON* root = cJSON_Parse(buf);
		if (root == NULL)
		{
			printf("parse error\n");
			return -1;
		}
		printf("1\n");
			int len = cJSON_GetArraySize(root);
		printf("\"love\"len=%d\n", len);
		int i = 0;
		cJSON* type_value = NULL;
		for (i = 0; i < len; i++)
		{
			type_value = cJSON_GetArrayItem(root, i);
			printf("star %d\n", i);
			if (type_value == NULL)
			{
				printf("GetObjectItem error\n");
				return -1;
			}
			cJSON* value = cJSON_GetObjectItem(type_value, "������");//��ȡ��"name"��Ӧ��JSONֵ
			if (value == NULL)
			{
				printf("GetObjec error\n");
				return -1;
			}

			memcpy(data[i].name, value->valuestring, strlen(value->valuestring));
			printf("%s\n", data[i].name);
			write(fd_w, &data[i].name, strlen(value->valuestring));
			write(fd_w, flag, sizeof(flag));


			cJSON* value1 = cJSON_GetObjectItem(type_value, "���");//��ȡ��"name"��Ӧ��JSONֵ
			if (value1 == NULL)
			{
				printf("GetObjec error\n");
				return -1;
			}

			memcpy(data[i].style, value1->valuestring, strlen(value1->valuestring));
			printf("%s\n", data[i].style);
			write(fd_w, &data[i].style, strlen(value1->valuestring));
			write(fd_w, flag, sizeof(flag));

			cJSON* value2 = cJSON_GetObjectItem(type_value, "����");//��ȡ��"url"��Ӧ��JSONֵ
			if (value2 == NULL)
			{
				printf("GetObjec error\n");
				return -1;
			}
			memcpy(data[i].singer, value2->valuestring, strlen(value2->valuestring));
			printf("%s\n", data[i].singer);
			printf("\n\n");
			write(fd_w, &data[i].singer, strlen(value2->valuestring));
			write(fd_w, flag, sizeof(flag));
			write(fd_w, flag_final, sizeof(flag_final));
		}
		


        sleep(60 * 60);
    }


    return 0;
}


