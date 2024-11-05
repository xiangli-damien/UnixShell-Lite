#include "shell.h"
#include <string.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>

void verify_separate_args(char *line, const char *expected_argv[], int expected_argc) {
    static int test_num = 0; 
    int got_argc=-100; 
    bool buit_in = false; 

    if(strcmp(line,"")== 0) {
            char **got_argv = separate_args("",&got_argc,&buit_in);
            if (got_argv != NULL) {
                printf("\tTest %d failed: separate_args(\"\") the function should return NULL.\n",test_num);
                return; 
            }
    }else {
        char line_cpy[strlen(line) + 1]; 
        strcpy(line_cpy,line); 
        char **got_argv = separate_args(line_cpy,&got_argc,&buit_in);
        if(got_argc != expected_argc) {
                printf("\tTest %d failed: separate_args(%s) the argc returned is incorrect.\n",test_num,line);
                printf("Expected:%d\n",expected_argc); 
                printf("Got:%d\n", got_argc); 
                return; 
        }

        for(int i =0; i < expected_argc; i++) {
            if(strcmp(got_argv[i],expected_argv[i]) != 0){
                printf("\tTest %d failed: separate_args(%s), argv[%d] do not match.\n",test_num,line,i);
                printf("Expected:%s\n",expected_argv[i]); 
                printf("Got:%s\n", got_argv[i]); 
                return; 
            }

        }
        if(got_argv[expected_argc] != NULL) {
                printf("\tTest %d failed: separate_args(%s), the argv[argc] must be NULL.\n",test_num,line);
                return; 
        }
        free(got_argv);
    }
    printf("Test %d passed.\n", test_num); 
    test_num++; 
}
int main() { 

    verify_separate_args("ls",(const char *[]){"ls"},1);  
    verify_separate_args("     ls",(const char *[]){"ls"},1);  
    verify_separate_args("ls    ",(const char *[]){"ls"},1);  
    verify_separate_args("  ls  ",(const char *[]){"ls"},1);
    verify_separate_args("",NULL,0);
    verify_separate_args("ls -la ~/mpcs51082-aut23",(const char *[]){"ls","-la","~/mpcs51082-aut23"},3);  
    verify_separate_args("   ls -la      ~/mpcs51082-aut23",(const char *[]){"ls","-la","~/mpcs51082-aut23"},3);  
    verify_separate_args("   ls -la      ~/mpcs51082-aut23      ",(const char *[]){"ls","-la","~/mpcs51082-aut23"},3);  
    verify_separate_args("   echo bob sally   joe   tim  ben heather          sam     jane   larry              ",(const char *[]){"echo","bob","sally","joe","tim","ben","heather","sam","jane","larry"},10); 
    return 0; 
}