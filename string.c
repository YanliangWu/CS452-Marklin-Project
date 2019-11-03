#include "string.h"

void stringcpy(char * dest, char * orig, int len){
    int i;
    for(i = 0; i < len; i++){
        dest[i] = orig[i];
    }
}

int stringcmp(char * dest, char * orig, int len){
    int i;
    int res = 0;
    for(i=0 ; i<len ; i++){
        if(dest[i] != orig[i]){
            res = 1;
            break;
        }
        if(dest[i]=='\000'&&orig[i]=='\000'){
            break;
        }
    }
    return res;
}
