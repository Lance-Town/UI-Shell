/*
 * Lance Townsend
 *
 * University of Idaho Shell - uish
 *
 * This is a program to implement a simple shell, similar to a simple
 * bash shell.
 *
 * Usage: ./uish
 *
 * Then start entering in the commands, and control-c to quit the
 * shell.
 *
 */

// System Includes
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// set the maximum size for the input string from the user
#define INPUT_STRING_SIZE 256
#define MAX_PATH_SIZE 256

// batch processing function
int processCommand();

// get the number of arguments in a string with single spaces
// as deliminators
int getNumArgs(char* s);

// function to handle setting environment variables
int handleEnvironmentVariables(char* line);

// get the number of args using a selected deliminator
int getNumArgsWith(char* s, char deliminator);

// returns true or false if the character is the deliminator or not
int isDeliminator(char c, char delim);

// run the command with all paths and see if the command exists or not
int runCommands(char** argv, int argc, char** paths, int numPaths);

// expand the environment value
int expandValue(char line[], int n);

// returns if there are characters in a string or not
int hasChars(char line[], int MAX_SIZE);

// strip the whitespace from a string
char *stripWhitespace(char *s);

// function to tokenize a string
int makearg(char *s, char ***args, char deliminator);

// main driver function
int main(int argc, char* argv[]) {
   char line[INPUT_STRING_SIZE];
   // process the commands
   while (1) {
      printf("uish $ ");
      int hold = processCommand();

      if (hold == -10) {
         // the shell found the EOF character, so exit the shell
         printf("\n");
         return 0;
      }
   }
   return 0;
}

// run the command with all paths and see if the command exists or not
int runCommands(char** argv, int argc, char** paths, int numPaths) {
   for (int i = 0; i < numPaths; i++) {
      char concatPath[MAX_PATH_SIZE];
      snprintf(concatPath, sizeof(concatPath), "%s/%s", paths[i], argv[0]);
      if (execv(concatPath, argv) >= 0) {
         return 0;
      }
   }

   return 1;
}

// function to handle setting environment variables
int handleEnvironmentVariables(char* line) {
   char *eq = strchr(line, '=');

   if (eq == NULL) {
      // no equals sign, return false
      return 0;
   }

   int l = 0;
   int r = strlen(line) - 1;

   // check that the string is smaller than the max size
   if (r >= INPUT_STRING_SIZE) {
      return -1;
   }

   // find the first character on left side
   while (isspace(line[l]) && l < r) {
      l++;
   }

   // find first character on right side
   while (isspace(line[r]) && r > l) {
      r--;
   }

   if (l == r) {
      // there is no character in the entire line
      return -2;
   }

   for (int i = l; i < r; i++) {
      if (isspace(line[i])) {
         // there is a space between the assignment so this is not
         // a valid way to set environment variables
         return -3;
      }
   }

   // get the index with pointer arithmetic
   int index = eq - line;
   int size = strlen(line);

   char var[INPUT_STRING_SIZE], val[INPUT_STRING_SIZE];

   if (index >= INPUT_STRING_SIZE) {
      // the input string was too large, return false
      return 0;
   }

   strncpy(var, line, index);
   var[index] = '\0';

   strncpy(val, &line[index+1], strlen(line) - index);
   val[strlen(val)] = '\0';

   setenv(var, val, 1);

   return 1;
}

// expand the environment value
int expandValue(char line[], int n) {
   char* dSign = strchr(line, '$');

   // check if dollar sign present
   if (dSign == NULL) {
      return 0;
   }

   // we have a dollar sign
   char* right = dSign;
   while (!isspace(*right) && *right != '\0') {
      right++;
   }

   char envName[right-dSign+1];
   // get a pointer to the next value AFTER the $
   char* p = dSign+1;

   int i = 0;
   for (i = 0; i < right-dSign+1; i++) {
      envName[i] = *p;
      p++;
   }
   envName[i] = '\0';

   char *envExpanded = getenv(envName);

   if (envExpanded == NULL) {
      printf("\n");
      return -1;
   }

   int newLineSize = INPUT_STRING_SIZE;
   char newLine[newLineSize];

   // copy over everything before the $
   strncpy(newLine, line, dSign - line);
   newLineSize -= (dSign - line);
   newLine[dSign - line] = '\0';
   // copy over the expanded environment variable
   strncat(newLine, envExpanded, newLineSize);
   newLineSize -= strlen(envExpanded);

   // copy everything after the expanded variable
   strncat(newLine, right, newLineSize);

   strncpy(line, newLine, n);
   return 0;
}

// returns if there are characters in a string or not
int hasChars(char line[], int MAX_SIZE) {
   int left = 0;

   while ((left < MAX_SIZE) && (line[left] != '\0')) {
      if (!isspace(line[left])) {
         // found a non-whitespace character
         return 1;
      }
      left++;
   }

   return 0;
}

// function to execute commands entered into the shell
int processCommand() {
   char line[INPUT_STRING_SIZE];
   char** argv;
   static char** paths = NULL;
   static char* fullPath = NULL;
   static int numPaths = -1;

   if (fullPath == NULL) {
      // full path is not initialized yet
      fullPath = getenv("PATH");
   }

   if (paths == NULL || numPaths == -1) {
      // the paths have not been initialized yet
      numPaths = makearg(fullPath, &paths, ':');
   }

   memset(line, '\0', INPUT_STRING_SIZE);

   // get the line from user
   if (fgets(line, sizeof(line), stdin) == NULL) {
      // end of file has been reached
      return -10;
   }

   // strip any new line characters from end of line
   line[strcspn(line, "\n")] = '\0';

   // check that the line is not empty
   if (!hasChars(line, INPUT_STRING_SIZE )) {
      return -3;
   }


   if (handleEnvironmentVariables(line)) {
      // there was an assignment of an environment variable
      return 0;
   }

   // check for a & in the line
   char* processBg = strchr(line, '&');

   if (processBg != NULL) {
      // remove the & so execv can process the command
      *processBg = '\0';
   }

   // expand all environment variables
   if (expandValue(line, sizeof(line))) {
      // there was an error expanding the environment variables
      return -5;
   }

   // tokenize the line string
   int argc = makearg(line, &argv, ' ');

   // null terminate last element in argv
   argv[argc] = NULL;

   // check for parsing errors
   if (argc < 0) {
      return -3;
   }

   // spawn a child
   pid_t child = fork();

   // child executes the command from text file
   if (child == 0) {
      // try to run the commands and see if there was an issue
      int commandExists = runCommands(argv, argc, paths, numPaths);
      if (commandExists) {
         printf("-uish: %s: command not found\n", argv[0]);
      }
      exit(1);
   } else {
      // is the parent
      if (processBg == NULL) {
         int status;
         waitpid(child, &status, 0);
      }
   }

   // free memory
   for (int i = 0; i < argc; i++) {
      free(argv[i]);
   }
   free(argv);

   return 0;
}

// get the number of arguments in a string with single spaces
// as deliminators
int getNumArgs(char* s) {
   char *c = s;
   // number of spaces (words) we have found
   int spaces = 1;

   while (*c != '\0') {
      // if a space is found, then we found a word
      if ( isspace(*c) ) {
         spaces++;
      }
      c++;
   }

   return spaces;
}

// get the number of args using a selected deliminator
int getNumArgsWith(char* s, char deliminator) {
   char* c = s;

   int args = 1;

   while (*c != '\0') {
      if (*c == deliminator) {
         args++;
      }
      c++;
   }

   return args;
}

// remove all extraneous whitespace from a string
char *stripWhitespace(char *s) {
   // remove leading white space
   while (isspace(*s)) {
      s++;
   }

   // remove trailing white space
   int len = strlen(s);
   while (len > 0 && isspace(s[len-1])) {
      len--;
   }

   // allocate enough memory for the string
   char *result = (char*)malloc(sizeof(char) * len + 1);

   // remove any extra whitespace between words
   char *tmp = result;
   int prevSpace = 0;

   // remove extra whitespace between words
   for (int i = 0; i < len; i++) {
      if (isspace(s[i])) {
         if (!prevSpace) {
            // found the first space
            *tmp = ' ';
            tmp++;
            // set previous space to ignore all spaces found after
            // this current space found
            prevSpace = 1;
         }
      } else {
         // there is no spaces, copy over regular values
         *tmp = s[i];
         tmp++;
         // set no previous spaces found
         prevSpace = 0;
      }
   }

   // null terminate string
   *tmp = '\0';

   return result;
}

// returns true or false if the character is the deliminator or not
int isDeliminator(char c, char delim) {
   if (isspace(delim)) {
      return isspace(c);
   }
   return (c == delim);
}

// function to tokenize a string based on a deliminator
int makearg(char *s, char ***args, char deliminator) {
   int argc = 0;
   int start = 0;
   int end = 0;

   // strip the whitespace
   s = stripWhitespace(s);

   // check if the input is not at least 1 character
   if (strlen(s) <= 0) {
      //printf("ERROR: Input is not at least 1 character\n");
      return -1;
   }

   // pointer to iterate through input string
   char *c = s;

   int numArgs = 0;
   // get number of arguments in the input
   if (isspace(deliminator)) {
      numArgs = getNumArgs(s);
   } else {
      numArgs = getNumArgsWith(s, deliminator);
   }

   // allocate enough space for all the words in the string
   *args = (char **)malloc(sizeof(char *) * (numArgs + 1));

   // tokenize each word in the string
   while (*c != '\0') {
      end++;

      if (isDeliminator(*c, deliminator)) {
         // found the end of a word
         (*args)[argc] = (char*) malloc(sizeof(char) * (end - start + 1));
         memset( (*args)[argc], '\0', sizeof(char) * (end - start + 1));
         strncpy((*args)[argc], &s[start], end - start - 1);
         (*args)[argc][end - start] = '\0';
         start = end;
         argc++;
      }

      c++;
   }


   // tokenize the last word the while loop doesn't
   (*args)[argc] = (char*) malloc(sizeof(char) * (end - start + 1));
   memset( (*args)[argc], '\0', sizeof(char) * (end - start + 1));
   strncpy((*args)[argc], &s[start], end - start);
   start = end;
   argc++;

   // check if the number of words in the string
   // match number of words found
   if (numArgs != argc) {
      return -1;
   }

   return argc;
}
