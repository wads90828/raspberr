#include <stdio.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(void)
{
struct input_event event_hcsr;
int fd,value,type,ret;

	fd = open("/dev/input/event0",O_RDWR);
	if(fd==-1)
		{
		printf("open hcsr fail ! \n");
		return -1;
		}
	else
		{
		printf("open '/dev/input/event0' success \n");
		}

	
while(1){	
	ret=read(fd,&event_hcsr,sizeof(event_hcsr));
	if(ret!=sizeof(event_hcsr))
		{	
			printf("read error");
			return 0;
		}
	else{	
		
		switch(event_hcsr.type){
				case EV_ABS:
					printf("ABS\n");
					if(event_hcsr.code == ABS_X)
					{
						printf("event_hcsr.code = %d\n",event_hcsr.code);
						printf("event_hcsr.value = %d\n",event_hcsr.value);
					}
						break;
				case EV_SYN:
					printf("EV_SYN\n");
					break;
					}
	   // close(fd);
	    usleep(1000);
		}
	}
	return 0;
	
}