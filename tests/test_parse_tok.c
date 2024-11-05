#include "shell.h"
#include <string.h>
#include <stdio.h> 

void verify_parse_tok(char *line, const char *expected[], int *job_types, int expected_len) {
    static int test_num = 0; 
    int commands_count = 0;  
    char line_cpy[strlen(line)+1]; 
    strcpy(line_cpy,line); 
    int job_type; 
    char *got = parse_tok(line_cpy, &job_type);
    while(got != NULL) {
        commands_count++; 

        if(strcmp(got,expected[commands_count-1]) != 0) {
            printf("\tTest %d failed: parse_tok(%s) for Job#%d\n",test_num,line,commands_count-1);
            printf("Expected:%s\n",expected[commands_count-1]); 
            printf("Got:%s\n", (got == NULL) ? "NULL" : got); 
            return; 
        }
        if(job_type != job_types[commands_count-1]){
            printf("\tTest %d failed: parse_tok(%s) invalid job_type for Job#%d\n",test_num,line,commands_count-1);
            printf("Expected:%d\n", job_types[commands_count-1]); 
            printf("Got:%d\n", job_type); 
            return; 
        }
        got = parse_tok(NULL, &job_type);
    }
    if(commands_count != expected_len) {
        printf("\tTest %d failed: parse_tok(%s) did not find the correct number of jobs on the line.\n", test_num,line);
        printf("Expected:%d\n", expected_len); 
        printf("Got:%d\n", commands_count); 
        return; 
    } 
    printf("Test %d passed.\n", test_num); 
    test_num++; 
}
int main() { 

    verify_parse_tok("ls -la & cd .. ; cat file.txt",(const char *[]){"ls -la "," cd .. "," cat file.txt"},(int []){0,1,1}, 3);                       
    verify_parse_tok("",NULL,NULL,0);  
    verify_parse_tok("ls&",(const char *[]){"ls"},(int []){0},1);  
    verify_parse_tok("ls",(const char *[]){"ls"},(int []){1},1);  
    verify_parse_tok("ls;",(const char *[]){"ls"},(int []){1},1);  
    verify_parse_tok("    ls;",(const char *[]){"    ls"},(int []){1},1);  
    verify_parse_tok("    ls;     ",(const char *[]){"    ls"},(int []){1},1);  
    verify_parse_tok("    ls&",(const char *[]){"    ls"},(int []){0},1);  
    verify_parse_tok("    ls& ",(const char *[]){"    ls"},(int []){0},1); 

    verify_parse_tok("cat file.txt     ;   ls    & cd ..      ;",(const char *[]){"cat file.txt     ","   ls    "," cd ..      "},(int []){1,0,1},3);  
    verify_parse_tok("echo hello&ls&cd ..&",(const char *[]){"echo hello","ls","cd .."},(int []){0,0,0},3);  
    verify_parse_tok("echo hello;               ls",(const char *[]){"echo hello","               ls"},(int []){1,1},2);  
    
    return 0; 
}