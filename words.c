#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h> 
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#define INITIAL_CAPACITY 10

typedef struct wordCount {
    char* word; 
    int count; 
} wordCount;

typedef struct wordArray {
    wordCount *counts; 
    int length;    
    int arrayMem; 
} wordArray; 

int findWord(wordArray *arr, const char *word) {
    for (int i = 0; i < arr->length; i++) {
        if (strcmp(arr->counts[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}

wordArray mergeWordArrays(wordArray *arr1, wordArray *arr2) {
    wordArray merged = {NULL, 0, 0}; 

    merged.arrayMem = arr1->arrayMem + arr2->arrayMem;
    merged.counts = malloc(sizeof(wordCount) * merged.arrayMem);

    if (!merged.counts) {
        perror("Failed to allocate memory in  mergeWordArrays");
        exit(EXIT_FAILURE);  
    }

    for (int i = 0; i < arr1->length; i++) {
        merged.counts[merged.length] = (wordCount){strdup(arr1->counts[i].word), arr1->counts[i].count}; 

        if (!merged.counts[merged.length].word) { 
            perror("Failed to duplicate word");
            exit(EXIT_FAILURE);
        }
        merged.length++;
    }

    for (int i = 0; i < arr2->length; i++) {
        int index = findWord(&merged, arr2->counts[i].word); 

        if (index != -1) {
            merged.counts[index].count += arr2->counts[i].count;

        } else {
            if (merged.length == merged.arrayMem) { 

                merged.arrayMem *= 2;
                wordCount *temp = realloc(merged.counts, sizeof(wordCount) * merged.arrayMem);

                if (!temp) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
                merged.counts = temp;
            }

            merged.counts[merged.length] = (wordCount){strdup(arr2->counts[i].word), arr2->counts[i].count};

            if (!merged.counts[merged.length].word) {
                perror("Failed crate word");
                exit(EXIT_FAILURE);
            }

            merged.length++; 
        }
    } 

    return merged;
}


void freeWordArray(wordArray *arr) {
    for (int i = 0; i < arr->length; i++) {
        free(arr->counts[i].word);
    }
    free(arr->counts);
    arr->counts = NULL;
    arr->length = 0;
}


void printStructArray(wordCount *array, int length) {
    for (int i = 0; i < length; i++) {

        printf("Word: %s\n", array[i].word);
        printf("Count: %d\n", array[i].count);
    }
}

int isValidFirstchar(char c) {
    return isalpha(c) || c == '\'' || c == '-'; 
}

int isValidChar(char c, char prev_c) {
    if (isalpha(c)) {
        return 1; 
    } else if ((c == '\'' && prev_c != '\'') || (c == '-' && prev_c != '-')) {
        return 1; 
    }
    return 0; 
}


int addWord(wordCount **wordArray, int *size, int *capacity, const char *word) {
    for (int i = 0; i < *size; i++) { 
        if (strcmp((*wordArray)[i].word, word) == 0) { 
            (*wordArray)[i].count++;                   
            return 1;                                 
        }
    }

    if (*size >= *capacity) {                   
        *capacity *= 2;                             
        wordCount *new_array = realloc(*wordArray, *capacity * sizeof(wordCount)); 
        if (!new_array) {                         
            return 0; 
        }
        *wordArray = new_array;                      
    }

    (*wordArray)[*size].word = strdup(word);           
    if (!(*wordArray)[*size].word) {                 
        return 0; 
    }
    (*wordArray)[*size].count = 1;                   
    (*size)++;                                   
    return 1;                                          
}


wordArray processFile(const char *filepath) {
    int fd = open(filepath, O_RDONLY); 

    if (fd < 0) {
        perror("Error opening file");
        return (wordArray){ NULL, 0, 0};   
    }

    int capacity = INITIAL_CAPACITY; 
    wordCount *words = malloc(capacity * sizeof(wordCount)); 


    if (!words) { 
        close(fd);
        return (wordArray){ NULL, 0}; 
    }

    char buffer[256];
    char wordBuff[256];
    int wordLength = 0;
    ssize_t bytesRead; 
    size_t arraysize; 
    int wordCountSize = 0; 

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {

        for (ssize_t i = 0; i < bytesRead; i++) {

            if (i == 0 ){
                if (isValidFirstchar(buffer[i])) {
                    wordBuff[wordLength++] = buffer[i];
                }
            }

            else if (isValidChar(buffer[i], buffer[i-1])) { 
                wordBuff[wordLength++] = buffer[i];
            }

            else if (wordLength > 0) { 
                wordBuff[wordLength] = '\0'; 

                if (!addWord(&words, &wordCountSize, &capacity, wordBuff)) { 
                    close(fd); 
                    fprintf(stderr, "Failed to add word '%s' to the collection.\n", wordBuff); 

                    for (int i = 0; i < wordCountSize; ++i) { 
                        free(words[i].word);
                    }
                    free(words);
                    return (wordArray){ NULL, 0}; 
                }
                arraysize++; 
                wordLength = 0; 
            } 
        }
    }

    if (bytesRead < 0) { 
        perror("Error: Bytes were not read.");
        close(fd);                                           
        for (int i = 0; i < wordCountSize; ++i) {         
            free(words[i].word);
        }
        free(words);
        return (wordArray){ NULL, 0}; 
    }

    close(fd);             
    return (wordArray){ words, wordCountSize, capacity}; 
}


int isTextFile(const char *filename) {
    const char *checker = strrchr(filename, '.');
    return checker && !strcmp(checker, ".txt");
}


wordArray processDirectory(const char *dirpath) {

    DIR *dir = opendir(dirpath);                     

    if (dir == NULL) {                                  
        perror("Failed to open directory");
        return (wordArray){ NULL, 0, 0 };
    }

    struct dirent *entry;                              
    wordArray merged = { NULL, 0, 0 };

    while ((entry = readdir(dir)) != NULL) {
        
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { 
            continue;
        }

        char pathName[1024];                                                             
        snprintf(pathName, sizeof(pathName), "%s/%s", dirpath, entry->d_name);        

        struct stat path_stat;
        if (stat(pathName, &path_stat) != 0) {
            perror("Error getting file stats");
            continue;
        }

        wordArray result; 

        if (S_ISREG(path_stat.st_mode) && isTextFile(pathName)) {
            result = processFile(pathName);
        } 
        
        else if (S_ISDIR(path_stat.st_mode)) {
            result = processDirectory(pathName);                  
        } 
        
        else {
            continue;  
        }

        if (result.counts != NULL) {            
            if (merged.counts == NULL) {
                merged = result;                 
            } else {
                wordArray temp = mergeWordArrays(&merged, &result);   
                freeWordArray(&merged);                              
                merged = temp;                                         
                freeWordArray(&result);                              
            }
        }
    }

    closedir(dir);                                                  
    return merged;                                                  
}


wordArray processArgument(const char *argument) {
    struct stat fileType; 

    if (stat(argument, &fileType) != 0) {
        perror("Error Processing the file argument");
        return (wordArray){ NULL, 0}; 
    }

    if (S_ISDIR(fileType.st_mode)) {
        return processDirectory(argument);      
    } 
    
    else if (S_ISREG(fileType.st_mode) && isTextFile(argument)) {
        return processFile(argument);
    }

    else{
        return (wordArray){ NULL, 0, 0};  
    }
}


int compareWordCount(const void *a, const void *b) {
    wordCount *firstWord = (wordCount *)a;
    wordCount *secondWord = (wordCount *)b;

    if (firstWord->count != secondWord->count) {
        return secondWord->count - firstWord->count; 
    }

    return strcmp(firstWord->word, secondWord->word);
}

void sortWordArray(wordArray *arr) {
    qsort(arr->counts, arr->length, sizeof(wordCount), compareWordCount);
}

void printWordArray(wordArray *arr) {
    for (int i = 0; i < arr->length; i++) {
        char buffer[1024];
        int len = snprintf(buffer, sizeof(buffer), "%s: %d\n", arr->counts[i].word, arr->counts[i].count);
        
        if (len > 0 && len < sizeof(buffer)) {
            ssize_t bytes_written = write(STDOUT_FILENO, buffer, len);
            if (bytes_written < 0) {
                perror("write");
                return;
            }
        }
    }
}

void writeWordArrayToFile(wordArray *arr, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < arr->length; i++) {
        fprintf(file, "%s: %d\n", arr->counts[i].word, arr->counts[i].count);
    }

    fclose(file);
}


int main(int argc, char *argv[]) { 
    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <ERROR: Must enter at least one file or directory as an argument!>\n", argv[0]); 
        return EXIT_FAILURE;
    }

    wordArray wordCounter = {malloc(sizeof(wordCount) * 10), 0, 10}; 

    for (int i = 1; i < argc; i++) {
        wordArray processedcount = processArgument(argv[i]);  
        wordArray tempMerged = mergeWordArrays(&wordCounter, &processedcount);
        freeWordArray(&wordCounter);  
        wordCounter = tempMerged;     
        freeWordArray(&processedcount); 
    }

    sortWordArray(&wordCounter);
    printWordArray(&wordCounter);
    freeWordArray(&wordCounter); 

    return EXIT_SUCCESS;
}
