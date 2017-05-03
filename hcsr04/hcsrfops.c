#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>





int main ()
{
  int fd,ret,i ;
  char distance;
  
	
	fd = open("/dev/testhcsr04device",O_RDONLY);

	if(fd>=0)
	{
		printf("open success\n");

	}
	 usleep(1000);
	for(i=0;i<=9;i++)
	{ret=read(fd,&distance,1);
 	if(ret!=1)
 	{
		printf("read failed\n ret = %d",ret);
		return ret;
	}
	
	printf("distance =%d\n",distance);
        usleep(1000);
}
close(fd);	
return 0 ;
}
