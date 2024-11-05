#include "history.h"
#include <string.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h> 

const char *LINES[] = {"ls -la", "cd ..", "cat file.txt", "sleep 20", "mkdir temp", "echo Hello World", "touch myfile.txt"};  

bool check_find_line(int test_num, history_t *history, const char *expected, int index) {
    char *got = find_line_history(history,index);
    if(expected == NULL && got != NULL) {
        printf("----\n");
        printf("Test %d failed: find_line_history(history,%d) returned incorrect value.\n", test_num, index);
        printf("Expected:NULL\n"); 
        printf("Got:%s\n", got); 
        printf("----\n");
        return false; 
    } else if (expected == NULL && got == NULL) {
        return true; 
    } else if (expected != NULL && got == NULL) {
        printf("----\n");
        printf("Test %d failed: find_line_history(history,%d) returned incorrect value.\n", test_num, index);
        printf("Expected:%s\n",expected); 
        printf("Got:NULL\n"); 
        printf("----\n");
        return false; 
    } else {
        if(strcmp(expected, got) != 0) {
            printf("----\n");
            printf("Test %d failed: find_line_history(history,%d) returned incorrect value.\n", test_num, index);
            printf("Expected:%s\n", expected); 
            printf("Got:%s\n", got); 
            printf("----\n");
            return false; 
        }
    }
    return true; 
}
bool check_file(int test_num, const char *expected[], int length) {

    FILE *fp = fopen(HISTORY_FILE_PATH, "r"); 
    if (fp != NULL) {
        char *line = NULL;
        long int len = 0;
        long nRead = getline(&line, &len, fp);
        int i = 0; 
        while (nRead != -1) {
            if(line[nRead - 1] == '\n') {
                line[nRead -1] = '\0'; 
            }
            if(expected != NULL  && length != 0 && strcmp(line,expected[i]) != 0){
                printf("----\n");
                printf("Test %d failed: Checking ../data/.msh_history and found incorrect command found at line #%d\n", test_num, i);
                printf("Expected:%s\n", expected[i]); 
                printf("Got:%s\n", line); 
                printf("----\n");
                free(line); 
                fclose(fp); 
                return false; 
            }
            i++; 
            free(line); 
            line = NULL;
            nRead = getline(&line, &len, fp);
        }
        fclose(fp); 
        if(i != length) {
            printf("----\n");
            printf("Test %d failed: Checking ../data/.msh_history anbd found incorrect number of commands in file.\n", test_num);
            printf("Expected:%d\n", length); 
            printf("Got:%d\n", i); 
            printf("----\n");
            return false; 
        }
    }else {
        printf("----\n");
        printf("Test %d failed: Could not find history file: ../data/.msh_history\n", test_num);
        printf("----\n");
        return false; 
    }
    return true; 
}
void test1() {
    int test_num = 1; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(5); 
    free_history(history);    
    if(check_file(test_num, NULL, 0)) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test2() {
    int test_num = 2; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(5); 
    add_line_history(history,LINES[0]); 
    free_history(history);    
    if(check_file(test_num,((const char *[]){LINES[0]}), 1)) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test3() {
    int test_num = 3; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(5); 
    add_line_history(history,LINES[0]);
    add_line_history(history,LINES[1]);
    free_history(history);    
    if(check_file(test_num,((const char *[]){LINES[0],LINES[1]}), 2)) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test4() {
    int test_num = 4; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(5); 
    for(int i = 0; i < 5; i++){
        add_line_history(history,LINES[i]);
    }
    free_history(history);    
    if(check_file(test_num,LINES, 5)) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test5() {
    int test_num = 5; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(5); 
    for(int i = 0; i < 7; i++){
        add_line_history(history,LINES[i]);
    }
    free_history(history);    
    if(check_file(test_num,LINES + 2, 5)) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test6() {
    int test_num = 6; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(5); 
    for(int i = 0; i < 7; i++){
        add_line_history(history,LINES[i]);
    }
    free_history(history);    
    history = alloc_history(5); 
    add_line_history(history,LINES[0]);
    add_line_history(history,LINES[1]);
    free_history(history);    
    if(check_file(test_num,((const char *[]){LINES[4],LINES[5],LINES[6],LINES[0],LINES[1]}), 5)) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test7() {
    int test_num = 7; 
    bool passed = true; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(14); 
    //Add 14 entries to the histroy 
    for(int i = 0; i < 14; i++){
        int index = i % 7; 
        add_line_history(history,LINES[index]);
    }
    //Check all 14 are in the right locations 
    for(int i = 0; i < 14; i++){
        int index = i % 7; 
        passed = passed && check_find_line(test_num,history,LINES[index],i + 1); 
    }
    //Save the file with the 14 locations 
    free_history(history);   
    //Check all 14 are still there at the right spots  
    history = alloc_history(14); 
    for(int i = 0; i < 14; i++){
        int index = i % 7; 
        passed = passed && check_find_line(test_num,history,LINES[index],i + 1); 
    }
    if(passed) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test8() {
    int test_num = 8; 
    bool passed = true; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(14); 
    //Add 14 entries to the histroy 
    for(int i = 0; i < 14; i++){
        int index = i % 7; 
        add_line_history(history,LINES[index]);
    }
    //Save the file with the 14 locations 
    free_history(history);   

    //Check only 5 are loaded and at the right locations. 
    history = alloc_history(5); 

    for(int i = 0; i < 5; i++){ 
        passed = passed && check_find_line(test_num,history,LINES[i],i + 1); 
    }
    //Check that no other locations were added. 
    for(int i = 6; i < 14; i++){
        int index = i % 7; 
        passed = passed && check_find_line(test_num,history,NULL,i + 1); 
    } 
    if(passed) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test9() {
    int test_num = 9; 
    bool passed = true; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(14); 
    free_history(history);   
    //Check that all history locations from a negative number to a positive number return NULL 
    history = alloc_history(5); 
    for(int i = -5; i < 10; i++){ 
        passed = passed && check_find_line(test_num,history,NULL,i); 
    }
    if(passed) {
        printf("Test %d Passed\n", test_num); 
    }
}
void test10() {
    int test_num = 10; 
    bool passed = true; 
    remove(HISTORY_FILE_PATH);
    history_t *history = alloc_history(14); 
    add_line_history(history,LINES[3]);
    free_history(history);   
    history = alloc_history(1);
    //Check that all history locations from a negative number to a positive number return NULL with the exception 1 
    for(int i = -5; i < 14; i++){ 
        if(i == 1){
            continue; 
        }
        passed = passed && check_find_line(test_num,history,NULL,i); 
    }
    passed = passed && check_find_line(test_num,history,LINES[3],1); 
    if(passed) {
        printf("Test %d Passed\n", test_num); 
    }
}
int main() { 

    test1();  
    test2();
    test3();
    test4();
    test5();  
    test6(); 
    test7(); 
    test8(); 
    test9(); 
    test10(); 
    return 0; 
}