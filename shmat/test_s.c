//client.c
#include"comm.h"

int main()
{
	int shmid = GetShm(4096);
	sleep(1);
	char* addr = shmat(shmid, NULL, 0);
	sleep(2);
	int i = 0;
	printf("%s", addr);
	shmdt(addr);
	sleep(2);
	return 0;
}