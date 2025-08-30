#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define ANSI_BLACK   "\x1b[30m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_WHITE   "\x1b[37m"
#define ANSI_RESET   "\x1b[0m"
#define DEL "\n\t \v\f\r"
#define childShell 0 // Fork returns 0, if in the child process

int status = 0; 


void shellLaunch(char **args){
  if (fork() == childShell) {
   execvp(args[0], args); // This function should never return something.
  }
  else {
    wait(&status); 
  }
}


int shellExit(char **args) {
  (void)args;

  exit(EXIT_SUCCESS); 
}

int shellEnv(char **args) {
  extern char **environ; 
  (void)args;
  if (!environ){
    return (1);
  }
  for (int i = 0; environ[i]; i++)
    printf("%s\n", environ[i]);
  return(0); 
}

typedef struct builtinfunction {
    const char *builtinName; 
    int (*foo)(char **args); // added so .foo works
} BuiltInFunctions;


BuiltInFunctions g_builtin[] = {
   // { .builtinName = "echo", .foo = shellEcho },
    { .builtinName = "env",  .foo = shellEnv  },
    { .builtinName = "exit", .foo = shellExit },
    { .builtinName = NULL,   .foo = NULL } // Sentinel
};


// Built In Functions: echo, env, exit
void argumentExecution(char **args) { // Basically an array of strings {
  /*Checks to run: 
    --> Is it a built in function(e.g. ls, cd, cat)
        --> If yes, just call the funtion directly
        --> If No, fork + execvp() + wait in the child process. */
  const char *current;
  int i;
  i = 0;
  while((current = g_builtin[i].builtinName)) {
    if (!strcmp(current, args[0])){
      g_builtin[i].foo(args); 
      return; 
    } //strcmp returns 0 if strings are equal
  i++;
   
  }
  shellLaunch(args); //Fork, Execute, Wait. 
}

// Figure out what is going on,
// WHat is **char 
char **tokeniseLine(char *line){
  char **tokens; // An array of tokens, simmilar to [av]
  tokens = malloc(BUFSIZ * sizeof(*tokens));
  unsigned int position; // Only stores positive numbers.
  size_t tokenBufferSize = BUFSIZ; 
  position = 0;  

  for (char *token = strtok(line, DEL);
       token;
       token = strtok(NULL, DEL)) {
   tokens[position++] = token;
   if (position >= tokenBufferSize){
    tokenBufferSize * 2; //Doubles Buffer Size in case user input is too long.
    tokens = realloc(tokens, tokenBufferSize * sizeof(*tokens)); 

    }
   }
   
   tokens[position] = NULL; //Sentinal value. 
   return tokens; //retuns the address to the array of tokens
  }
  



void getCWD(char *bufferCWD, size_t sizeofBufferCWD){ //Wrapper for getcwd()
 if(getcwd(bufferCWD, sizeofBufferCWD) == NULL)  // getCWD is called here. If these addresses are empty: 
 {
  perror(ANSI_RED "Failed to get Current Working Directory" ANSI_RESET);
 }

}

char *getShellLine(void){
  char *buffer;  // Address of buffer which will hold the charecters
  size_t bufferSize; // Size of the buffer 
  buffer = NULL; // GetLIne function will assign a size to this.

  char cwdBuffer[BUFSIZ];  // An array of charecters (string) holding CWD.
  
  getCWD(cwdBuffer, sizeof(cwdBuffer)); // GETCWD arg1 = address for buffer to store cwd, arg2 - self explanatory
  if (isatty(fileno(stdin))){ // fileno(stdin) returns FD for stdin.
      printf(" \n %s --> ", cwdBuffer); 

  }



  if (getline(&buffer, &bufferSize, stdin) == -1) { 
    free(buffer);
    buffer = NULL; // Have to clear buffer again incase getline() makes it pointer to an empty string. 
    if (feof(stdin)){ // Returns a non-zero value if EOF is reached.
      printf(ANSI_GREEN "End of File" ANSI_RESET);
    }
    else {
      printf(ANSI_RED "Getline() Failed :(" ANSI_RESET); 
         } 
    }
       
  return buffer; /* *buffer --> Value stord at Address
                     buffer --> A memory address (location) for the buffer varaible     */
 }



int main(int ac, char **av) {
  // Read "Crafting Interpreters" !!! 
  char *shellLine; // Pointer to location where user input is stored.
  char **arguments; 
  while (shellLine = getShellLine()) { //Keep reading until user inputs a line
   //1. Take line from user via getline() --> used for when size of input is unknown. 
   // printf("\n %s", shellLine); //Proof that the program is succesfully reading a line
                                //This is where we can tokenize the line and evaluate it.  
   //2. Tokenise line (gettoken)

   arguments = tokeniseLine(shellLine);

   for (int i = 0; arguments[i]; i++){
     printf("%s\n", arguments[i]);
   }
  

   //3. Execute
   argumentExecution(arguments); 

   //4. Free Memory at the end of each loop.
   free(shellLine);
   free(arguments); 
  }
 
 }
