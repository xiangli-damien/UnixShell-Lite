#include "history.h"
#include "signal_handlers.h"
#include "shell.h"
#include "job.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>


const char *HISTORY_FILE_PATH = "../data/.msh_history";

// Allocates and initializes a history_t structure
history_t *alloc_history(int max_history) {

    //printf("Allocating history with max_history: %d\n", max_history);
    // Allocate memory for the history_t structure
    history_t *history = (history_t *)malloc(sizeof(history_t));
    if (!history) {

        //perror("Failed to allocate memory for history structure");
        return NULL;
    }

    // Allocate memory for the array to store command line strings
    history->lines = (char **)malloc(max_history * sizeof(char *));
    if (!history->lines) {
        //perror("Failed to allocate memory for history lines");
        free(history); // Free allocated structure on failure
        return NULL;
    }

    // Initialize each element of history->lines to NULL
    for (int i = 0; i < max_history; i++) {
        history->lines[i] = NULL;
    }

    // Initialize the history structure fields
    history->max_history = max_history;
    history->next = 0;

    // Open the history file and read prior command lines
    FILE *file = fopen(HISTORY_FILE_PATH, "r");
    if (file) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        // Load lines from the file into the history array
        while ((read = getline(&line, &len, file)) != -1 && history->next < max_history) {
            if (read > 0 && line[read - 1] == '\n') {
                line[read - 1] = '\0';  // Remove newline character
            }
            history->lines[history->next++] = strdup(line); // Duplicate and store the line
        }
        free(line);
        fclose(file);
    }

    return history;
}

// Helper function to shift history lines to remove the oldest entry
void shift_history(char **lines, int max_history) {
    free(lines[0]); // Free the memory of the oldest line
    for (int i = 1; i < max_history; i++) {
        lines[i - 1] = lines[i];
    }
    lines[max_history - 1] = NULL; // Set the last element to NULL
}

// Adds a new line to the history
void add_line_history(history_t *history, const char *cmd_line) {
    // Ignore empty commands and the "exit" command
    if (!cmd_line || strlen(cmd_line) == 0 || strcmp(cmd_line, "exit") == 0) {
        return;
    }

    // Delete the oldest record if history is full
    if (history->next == history->max_history) {
        shift_history(history->lines, history->max_history);
        history->next--;
    }

    // Add the new command line to the history
    history->lines[history->next++] = strdup(cmd_line);
    //printf("New history entry added: %s\n", history->lines[history->next - 1]);

    //printf("History array after adding: ");
    // for (int i = 0; i < history->next; i++) {
    //     printf("[%s], ", history->lines[i]);
    // }
    //printf("\n");
}

void print_history(history_t *history) {
    //printf("Printing history, total entries: %d\n", history->next);
    for (int i = 0; i < history->next; i++) {
        printf("%d %s\n", i + 1, history->lines[i]);
    }
}

// Finds and returns the command line at the given index in history
char *find_line_history(history_t *history, int index) {
    // Check for out-of-range index
    if (index < 1 || index > history->next) {
        return NULL; // Return NULL if index is invalid
    }

    // Adjust for 1-based indexing in history
    return history->lines[index - 1];
}

// Frees the history structure and its associated resources
void free_history(history_t *history) {
    // Save history to file before freeing
    FILE *file = fopen(HISTORY_FILE_PATH, "w");
    if (file) {
        for (int i = 0; i < history->next; i++) {
            fprintf(file, "%s\n", history->lines[i]); // Write each line to the file
        }
        fclose(file);
    }

    // Free each string in the history array
    for (int i = 0; i < history->next; i++) {
        free(history->lines[i]);
    }

    // Free the history array and the history_t structure
    free(history->lines);
    free(history);
}
