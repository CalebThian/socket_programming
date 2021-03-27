#include <stdio.h>
#include <time.h>

void time_now(){
	time_t seconds = time(NULL);
	printf("second=%d\n",seconds);
	
	time_t minutes = seconds/60;
	printf("min=%d,sec=%d\n",minutes,seconds);
	seconds %= 60;
	printf("min=%d,sec=%d\n",minutes,seconds);
	time_t hours = minutes/60;
	printf("hours=%d,min=%d,sec=%d\n",hours,minutes,seconds);
	minutes %= 60;
	printf("hours=%d,min=%d,sec=%d\n",hours,minutes,seconds);
	time_t days = hours/24;
	printf("days = %d,hours=%d,min=%d,sec=%d\n",days,hours,minutes,seconds);
	hours %= 24;
	printf("days = %d,hours=%d,min=%d,sec=%d\n",days,hours,minutes,seconds);
	int start_year;
	for(start_year=1970;days>365;start_year++){
		if((start_year%4==0 && start_year%100!=0)||(start_year%100==0 && start_year%400==0))
			days-=366;
		else
			days-=365;
	}
	
	days++;
	int current_month=1;
	int month[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	if((start_year%4==0 && start_year%100!=0)||(start_year%100==0 && start_year%400==0))
		month[2]++;

	for(current_month=1;days>month[current_month-1];++current_month){
		days-=month[current_month-1];
	}

	printf("%ld/%ld/%ld %02d:%02d:%02d",start_year,current_month,days,hours+8,minutes,seconds);
}

int main(){
	//time_now();
	char buf[256]={0};
	time_t rawtime = time(NULL);
	strftime(buf,256,"%D\n",localtime(&rawtime));
	puts(buf);
	return 0;
}
