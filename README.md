# Word Occurence Count. by Stephen Wassum

## Running the Program

To run this program open an terminal at the location of this README file and other files within this folder.
Once here run the make command in the terminal. The Makefile will compile words.c. Then use the command 
./words in the terminal followed by a txt file or directory as an argument.

## words.c

**File Overview:**
This program takes in txt files or directories as arguments and outputs a word count of every word encountered in every detetected
text file within the arguments in the console.

### Project Implementation

*project structs*

```c
typedef struct wordCount {
    char* word; 
    int count; 
} wordCount;

typedef struct wordArray {
    wordCount *counts; 
    int length;    
    int arrayMem;   
} wordArray; 
```

- Two structs are used in this project to store information read from input arguments
- The first struct contains a char* word and int count. word is a dynamically allocated a string that is read from a file,
  count is the number of times that this string has been encountered within a file or files
- The second struct WirdArray has three values wordCount *counts, length and arrayMem. counts is an array of wordCount structs,
  length is the number of wordCount structs present in counts, and arrayMem is the amount of memory that is currently allocated
  for counts.


*main function* 

```c
int main(int argc, char *argv[]) { 

    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <ERROR: Must enter at least one file or directory as an argument!>\n", argv[0]); 
        return EXIT_FAILURE;
    }

    wordArray wordCounter = {malloc(sizeof(wordCount) * 10), 0, 10}; 

    for (int i = 1; i < argc; i++) {
        wordArray processedcount = processPath(argv[i]);  
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
```

The main function performs several operations

- First it checks if there is at least one argument made in the console that is either a txt file or a directory
- It then creates an empty wordArray to be written into. It writes into wordArray by calling on the processArgument function detailed
  below. ProcessArgument is run on every argument inputed and the mergeWordArrays funciton is called to merge a wordArray processed from
  a file or directory with other wordArrays processed from previous files and directorys. Once an array is merged into the main
  wordArray using the freeWordArray funciton. Once arguments are processed, the wordArray struct is sorted based first decreasing
  count, and secondly lexiographically. The results are then printed to the console using printWordArray, and the wordArray is
  free'd before exiting the program.

*ProcessArgument function* 

```c
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
```

-The process Argument function first creates a stat struct filetype. The struct stat is included in #include <sys/stat.h>, it checks the file type.
-The function then checks if there was an error retriving an argument or creating filetype if so an error is printed and the function returns
 an empty WordArray.
-If no error is encounterd the funciton uses macros defined in <sys/stat.h> along with the isTextFile function to check if the argument a directory
 or a text file and calls either processDirectoy, or processFile function. If the argument is neither a txt file or directory, a null word array
 is returned. Returns a word Array sruct.

*Is Text File function* 

```c
int isTextFile(const char *filename) {
    const char *checker = strrchr(filename, '.');
    return checker && !strcmp(checker, ".txt");
}
```

-The isTextFile function checks if a file is txt file by calling using #include <string.h> to call the strrchr and strcmp functions to check whether
 or not a txt file is encounterd.

*ProcessFile function* 

```c
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
```

-The Process file reads a txt file and returns a wordArray struct.
-uses the open() function to read from a file. If their is on error opening the file returns a null array. Othewise it initalizes an empty wordCount
 array to store words from file in. Then it creates several buffers to temporarily store words in and use in read(), the function then reads from the
 file passed into the function. It uses a function isValidFirstChar to check if the first charcter is valid, then isValidChar function to check if 
 remaining chars are valid. Once a non valid char is detected and the word length or current number of detected valid chars is not 0, the function
 uses the function addWord to add the detected character string to the words array. If addword function fails an ERROR is printed and the words 
 array is freed. Finally the function detects if there were any bytes not read. If so the the function frees words closes the file and returns
 a null array. Finally the function returns a wordArray consisting of the created words array, the amount of words added to that array, and an int
 representing the amount of memory allocated for the words array.

*isValidFirstChar and isValidChar functions* 

```c
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
```

-These functions use the standard library function is alpha to check if the given character is any character from 'a' to 'z' or from 'A' to 'Z'.
 Additionally '-', and '\' are considered valid characters so long as they are not repeated in a word. The isValidChar function checks the previous
 char in the file to check if these characters are repeated whereas is isvalidFirstChar simply detects if these are encountered. If the char 
 is valid returns 1, else returns 0.

*addWord function* 

```c

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
```

-The addWord function takes a wordCount array, array length size, array capacity, and a word string as an input. And returns an int to tell whether
 the functions operation was succesful. The function first compares the string to every string in the wordCount array. If the same string is 
 encountered in a word of the struct then that structs count is incremented. Otherwise, that word is added to the array. Additionally, the function
 dynamically allocated memory, and checks the size of the array by using malloc when adding a word and checking the capacity. The variables passed 
 into this function from process file function modified accordingly.

*processDirectory function* 

```c
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
```

-The process dierctory function uses the function opendir to open an encoutered directory argument. It then reads through every entry in the directory
 searching for txt files to read from. If a txt file is encountered the processFile function is called and the returned wordArray is stored in a temporarily
 stored wordArray struct result, result of current entry. If a directory is encountered this function is called recursivley. The returned wordArrays. 
 are merged with a greater wordArray that stores the merged result of all the arrays from every txt entry found. The temporary arrays are free'd using
 freeWordArray function. Once every txt file is found and read from the directory is closed using closedir() and the final merged array, WordArray struct
 is returned from this function.

*mergeWordArrays function* 

```c

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
```

-This function merges two arrays together. starts by creating an empty to array to be written to. First the count and arrayMem are determined based upon
 the variables of the passed in wordArray's. If this inital array fails to be created the function prints out an error and stops. elsewise, all elements
 from the first array are copied into the placeholder array. Then the second array is iterated over and the findWord function is called on the first 
 array using words from the second. If a word is found the count is incremented, elsewise the word is added to the merged array, allocating memory as 
 necessary.

*findWord function* 

```c
int findWord(wordArray *arr, const char *word) {
    for (int i = 0; i < arr->length; i++) {
        if (strcmp(arr->counts[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}
```

-Compares word passed into function with every word from the wordArray object passed into the funtion. 

### Other Project functions

*sortWordArray function* 

```c
int compareWordCount(const void *a, const void *b) {
    wordCount *firstWord = (wordCount *)a;
    wordCount *secondWord = (wordCount *)b;

    if (firstWord->count != secondWord->count) {
        return secondWord->count - firstWord->count; // For descending order
    }

    return strcmp(firstWord->word, secondWord->word);
}

void sortWordArray(wordArray *arr) {
    qsort(arr->counts, arr->length, sizeof(wordCount), compareWordCount);
}
```

-This function uses qsort stdlib to sort the a list in a WordArray decedingly then lexiographically. The function used a custom comparer function as
 an argument. The custom compareret checks two words from a list first decendingly, then lexiographically and uses their counts or strcmp to compare them.

*printWordArray function* 

```c
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
```

-Takes in a wordArray argunment and uses the write() function to print to every word and count in the struct to the console.

*printWordArray function* 

```c
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
```

-Writes every word in a wordArray to a txt file.


