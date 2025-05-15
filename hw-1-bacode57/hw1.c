#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

static void methods()
{
    printf("error_handler\n");
    printf("is_invalid\n");
    printf("signature\n");
    printf("usage\n");
}


static void vars()
{
    printf("ERRORS\n");
    printf("FULL\n");
    printf("FULL_ERRORS\n");
    printf("MODE\n");
    printf("RESTRICTED\n");
    printf("STATUSCODE\n");
}

// TODO define functions and variables you found in lib.o
extern int signature(char * input, int len);
extern void usage();

extern int STATUSCODE;
extern int RESTRICTED;
extern int FULL;
extern int* MODE;


int main(int argc, char * argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Program needs at least one argument\n");
        return 1;
    }

    char * arg = argv[1];
    if (!strcmp(arg, "methods")) {
        methods();
    } else if (!strcmp(arg, "vars")) {
        vars();
    } else if (!strcmp(arg, "help")) {
        // assert(0); // NOT IMPLEMENTED remove this line once implemented
        usage();
    } else {
        const int result = -1;
        int length = strlen(arg); //Get length of string
        if(is_invalid(arg,length)) { //Check if string is valid to set the mode
            MODE = &FULL;
        } else {
            MODE = &RESTRICTED;
        }
        int res;
        res = signature(arg,length); //Get signature after mode was set
        if(STATUSCODE == 0) {
            printf("%d",res); 
        } else {
            for(int i = length - 1; i >= 0; i--) {
                printf("%c",arg[i]); //Print argument in reverse
            }
            
        }
        
    }

    return 0;
    
}
