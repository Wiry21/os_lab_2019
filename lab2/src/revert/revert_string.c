#include "revert_string.h"

void RevertString(char *str)
{
    int size=0;
    for (int i=1; str[i] != '\0'; i++){
        size++;
    }
    int stop=(size+1)/2;
    char m;
    
    for (int i=0; i<stop;i++){
        m=str[i];
        str[i]=str[size-i];
        str[size-i]=m;
    }
	// your code here
}

