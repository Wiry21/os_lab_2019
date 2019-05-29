#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "revert_string.h"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s string_to_revert\n", argv[0]);
		return -1;
	}

	char *reverted_str = malloc(sizeof(char) * (strlen(argv[1]) + 1));
	strcpy(reverted_str, argv[1]);

    char *str1=malloc(sizeof(char) * (strlen(argv[1]) + 1));
    char *str2=malloc(sizeof(char) * (strlen(argv[1]) + 1));
    
    int size=0;
    for (int i=1; reverted_str[i] != '\0'; i++){
        size++;
    }
    
    int stop=(size+1)/2;
    
    for (int i=0; i<stop;i++){
        str1[i]=reverted_str[i];
    }

    for (int i=(size+1)/2; i<(size+1);i++){
        str2[i-(size+1)/2]=reverted_str[i];
    }
    
	RevertString(str1);
	printf("Reverted: %s\n", str1);
	RevertString(str2);

	printf("Reverted: %s\n", str2);
	free(reverted_str);
	free(str1);
	free(str2);
	return 0;
}

