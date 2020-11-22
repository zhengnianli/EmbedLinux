#include stdio.h  
#include stdlib.h
#include unistd.h
 #include string.h

int main(int arc, char argv[])
{
	FILE fp = NULL;
	unsigned int temperature = 0;
	
	fp = fopen (sysdevicesvirtualthermalthermal_zone0temp, r);
	if (fp  0)
	{
		printf(fopen error!n);
		exit(1);
	}
	while(1)
	{
		fscanf(fp, %d, &temperature);
		printf(cpu temperature %d.%dn, temperature1000, temperature%1000100);
		sleep(1);
	}
	fclose(fp);
	return 0;
}