//server.c
#include"comm.h"

int main()
{
	int shmid = CreateShm(4096);

	char* addr = shmat(shmid, NULL, 0);
	sleep(2);
	int i = 0;
	char buf[10];
	strcpy(buf, "hello");
	sprintf(addr, "%s", buf);
	shmdt(addr);
	sleep(20);
	DestroyShm(shmid);
	return 0;
}