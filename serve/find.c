
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

 //#define WRITE_
 //#define READ_
#define CHANG_

int main(void)
{

	
	//��ȡjson�ļ�����

	//���ļ�����ȡ����
	//�򿪱���JSON���ݵ��ļ�
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
	printf("1\n")��
	int len = cJSON_GetArraySize(root);
	printf("\"love\"len=%d\n", len);
	int i = 0;
	cJSON* type_value = NULL;
	for (i = 0; i < len; i++)
	{
		type_value = cJSON_GetArrayItem(root, i);
		printf("star %d\n",i);
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
		char str[100] = { 0 };
		memcpy(str, value->valuestring, strlen(value->valuestring));
		printf("%s\n", str);

		cJSON* value1 = cJSON_GetObjectItem(type_value, "���");//��ȡ��"name"��Ӧ��JSONֵ
		if (value1 == NULL)
		{
			printf("GetObjec error\n");
			return -1;
		}
		char str1[100] = { 0 };
		memcpy(str1, value1->valuestring, strlen(value1->valuestring));
		printf("%s\n", str1);

		cJSON* value2 = cJSON_GetObjectItem(type_value, "����");//��ȡ��"url"��Ӧ��JSONֵ
		if (value2 == NULL)
		{
			printf("GetObjec error\n");
			return -1;
		}
		char str2[100] = { 0 };
		memcpy(str2, value2->valuestring, strlen(value2->valuestring));
		printf("%s\n", str2);
		printf("\n\n");
	}

	return 0;
}




