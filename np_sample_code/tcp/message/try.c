#include <stdlib.h>
#include <stdio.h>

int main(){
	FILE *fp = fopen("alphabet.txt","w");
	int len = 0;
	int mb = 0;
	char alpha[26];
	for(int i = 0;i<26;++i){
		alpha[i]='a'+i;
	}
	while(mb<600*1024){
		len = 0;
		while(len<1024){
			fprintf(fp,alpha);
			len+=26;
		}
		mb+=1;
	}
	fclose(fp);
	return 0;
}
