
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

	
	//读取json文件内容

	//打开文件并读取数据
	//打开保存JSON数据的文件
	int fd = open("JSON_MESSAGE.json", O_RDWR);
	if (fd < 0)
	{
		perror("open fail\n");
		return -1;
	}

	//读取文件中的数据
	char buf[2048] = { 0 };
	int ret = read(fd, buf, sizeof(buf));
	if (ret == -1)
	{
		perror("read error");
		return -1;
	}

	close(fd);
	printf("%s", buf);
	//把该字符串数据转换成JSON数据(对象)  开始的对象为最外层的对象
	cJSON* root = cJSON_Parse(buf);
	if (root == NULL)
	{
		printf("parse error\n");
		return -1;
	}
	printf("1\n")；
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
		cJSON* value = cJSON_GetObjectItem(type_value, "歌曲名");//获取键"name"对应的JSON值
		if (value == NULL)
		{
			printf("GetObjec error\n");
			return -1;
		}
		char str[100] = { 0 };
		memcpy(str, value->valuestring, strlen(value->valuestring));
		printf("%s\n", str);

		cJSON* value1 = cJSON_GetObjectItem(type_value, "风格");//获取键"name"对应的JSON值
		if (value1 == NULL)
		{
			printf("GetObjec error\n");
			return -1;
		}
		char str1[100] = { 0 };
		memcpy(str1, value1->valuestring, strlen(value1->valuestring));
		printf("%s\n", str1);

		cJSON* value2 = cJSON_GetObjectItem(type_value, "歌手");//获取键"url"对应的JSON值
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




