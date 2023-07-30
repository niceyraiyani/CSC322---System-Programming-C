#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_SIZE = 256;
const int MAX_SHORT_STRING = 80;
typedef char String[MAX_SIZE];
typedef char ShortString[MAX_SHORT_STRING];

// enumerator for the type of line range specification
typedef enum { none, text, range } lineType;

// union for line range specification
typedef union {
  // /<text>/ : all the lines that contain that text
  String cText;
  // range[0] : line number 1 start and range[1] : line number 2 end
  int range[2];
  // noSpec = 0 if TRUE, any other number if FALSE;
  int noSpec;
} lineRangeUnion;

// the stuct for the operators
typedef struct {
  lineType specification;
  lineRangeUnion lineRange;
  // will be one of {A,I,O,d,s}
  char operation;
  ShortString editText;
} EditCommands;

// Function declarations
int readCommands(FILE *commandFile, EditCommands edits[]);
EditCommands getEditCommands(String command);
int isPossible(EditCommands cstruct, String inputText, int lineNo);
void editInput(String Atext, String Itext, String Otext, int *pointer,
               String inputText, EditCommands editStruct);
char *replaceFunction(char *inputText, char *toReplace, char *fromReplace);

// Edit the commands
EditCommands getEditCommands(String inputC) {

  EditCommands cstruct;

  // breaking down the string and assigning it to the EditCommands struct
  // line range specification 1: /<text>/
  if (inputC[0] == '/') {
    cstruct.specification = text;
    strncpy(cstruct.lineRange.cText, inputC + 1,
            strlen(inputC) - (1 + strlen(strchr(inputC + 1, '/'))));
    cstruct.lineRange
        .cText[strlen(inputC) - strlen(strstr(inputC + 1, "/")) - 1] = '\0';
    //      printf("line range: %s\n", cstruct.lineRange.cText);

    
    cstruct.operation = inputC[strlen(cstruct.lineRange.cText) + 2];
    //   printf("operation = %c\n", cstruct.operation);

    strcpy(cstruct.editText, strchr(inputC, '/'));

    //   printf("editText: %s\n", cstruct.editText);
    return cstruct;
  }

  // line range specification 2: <1st line number>,<last line number>/
  if (isdigit(inputC[0])) {
    //  printf("goes to the range if loop");
    cstruct.specification = range;
    cstruct.lineRange.range[0] = atoi(&inputC[0]);
    cstruct.lineRange.range[1] = atoi(&inputC[2]);
  
    String temp1;
    strcpy(temp1, strchr(inputC, '/'));
    cstruct.operation = temp1[1];
    //    printf("operation: %c\n", cstruct.operation);
    strcpy(cstruct.editText, strchr(inputC + 1, '/') + 2);
    //  printf("editText: %s\n", cstruct.editText);
    return cstruct;
  }

  // line range specification 3: None: so applies to all
  // printf("Reaches the none line specification! inputc: %s", inputC);
  cstruct.specification = none;
  cstruct.operation = inputC[0];
  // printf("operation: %c",cstruct.operation);
  cstruct.lineRange.noSpec = 1; // true
  strncpy(cstruct.editText, inputC + 1, strlen(inputC));
  return cstruct;
}

// Read the edit commands from the file
int readCommands(FILE *file, EditCommands commands[]) {
  String inputcommand;
  int counter = 0;
  // read and store the edit commands into an array until its 100 or EOF

  while (counter < 100 && fgets(inputcommand, MAX_SIZE, file) != NULL) {
    commands[counter] = getEditCommands(inputcommand);

    counter++;
    // printf("counter: %s", counter);
  }

  return counter;
}

int isPossible(EditCommands cstruct, String inputText, int lineNo) {
  // 3 possibilies are possible: 1. text format: the text must exist 2. line
  // range: must be within the line range 3. none: all lines

  // 1. text format
  if (cstruct.specification == text) {
    if (strstr(inputText, cstruct.lineRange.cText) != NULL) {
      return 1;
    }
  }

  // 2. line range
  if (cstruct.specification == range) {
    if (lineNo >= cstruct.lineRange.range[0] &&
        lineNo <= cstruct.lineRange.range[1]) {
      return 1;
    }
  }

  // 3. none
  if (cstruct.specification == none) {
    return 1;
  }

  // if the command request is not possible:
  return 0;
}

char *replaceFunction(char *inputText, char *toReplace, char *fromReplace) {

  static char doubleString[512];
  char *p;
  // if there is no occurence of the toReplace in input then return
  if (!(p = strstr(inputText, toReplace))) {
    return inputText;
  }

  strncpy(doubleString, inputText, p - inputText);
  doubleString[p - inputText] = 0;
  sprintf(doubleString + (p - inputText), "%s%s", fromReplace,
          p + strlen(toReplace));
  // printf("double string after sprintf: %s\n", doubleString + (p -
  // inputText));
  return doubleString;
}
// void editInput(EditCommands edit, String input, String OLines, String ILines,
// String ALines, int *p1);

void editInput(String Atext, String Itext, String Otext, int *pointer,
               String inputText, EditCommands editStruct) {

  // A<text> Appends the <text> at the end of the line.
  if (editStruct.operation == 'A') {
    // printf("Reaches the edit operation A statement");
    strncat(Atext, editStruct.editText, strlen(editStruct.editText));
    Atext[strlen(Atext)] = '\0';
    return;
  }

  // I<text> Inserts the <text> at the start of the line.
  if (editStruct.operation == 'I') {
    String edited;
    strcpy(edited, editStruct.editText);
    strcat(edited, Itext);
    strcpy(Itext, edited);
    Itext[strlen(Itext) - 1] = '\0';
  }

  // O<text> Inserts the <text> on a new line before the current line.
  if (editStruct.operation == 'O') {
    strncat(Otext, editStruct.editText, strlen(editStruct.editText));
    Otext[strlen(Otext)] = '\0';
    return;
  }

  // d : Deletes the line from the file.
  if (editStruct.operation == 'd') {
    *pointer = 1;
    return;
  }

  // s/<old text>/<new text>/ Replaces the first occurence of <old text>, in the
  // line, with <new text>

  if (editStruct.operation == 's') {
    if(strlen(editStruct.lineRange.cText)>2){
      String tempstring;
      strncpy(tempstring, strchr(editStruct.editText+1,'/') + 3, strlen(strchr(editStruct.editText+1,'/') + 3));
      strncpy(editStruct.editText, tempstring, strlen(tempstring));
}
    String toReplace, fromReplace;
    char* tok;
    tok = strtok(editStruct.editText, "/");
    if(tok!=NULL){
      strcpy(toReplace,tok);
      tok = strtok(NULL,"/");
    }
    if(tok!=NULL){
      strcpy(fromReplace,tok);
    }
    fromReplace[strlen(fromReplace)] = '\0';
    toReplace[strlen(toReplace)] = '\0';
    while (strstr(inputText, toReplace)) {
      strcpy(inputText, replaceFunction(inputText, toReplace, fromReplace));
    }
  }
}

int main(int argc, char *argv[]) {

  // check if the user put in too less commands
  if (argc < 2) {
    printf("There must be 2 commands : edit command file and input file\n");
    return 0;
  }

  //--------------- initializing variables ---------------//
  FILE *StreamEditorCommands;
  // A maximum of 100 edit commands can be used, so an array to store the
  // structs:
  EditCommands arrayEditStructs[100];
  int noOfEdits = 0;

  // Read the command file given and give an error if its empty
  StreamEditorCommands = fopen(argv[1], "r");
  if (StreamEditorCommands == NULL) {
    printf("The file \"%s\" is empty or doesn't exist. \n", argv[1]);
    return 1;
  }

  //--------------- reading and storing the commands---------------//
  noOfEdits = readCommands(StreamEditorCommands, arrayEditStructs);

  //--------------- reading and editing the input file---------------//

  String inputText;
  int lineNo = 1;

  while (fgets(inputText, MAX_SIZE, stdin) != NULL) {
    inputText[strlen(inputText) - 1] = '\0';

    // Intializing all the possible edit operations
    String Atext;
    Atext[0] = '\0';
    String Itext;
    Itext[0] = '\0';
    String Otext;
    Otext[0] = '\0';
    int boolDeleted = 0;
    int *pointer;
    pointer = &boolDeleted;
    String Stext;
    Stext[0] = '\0';
    // Running through all the structs that we made on top and editing one by
    // one
    for (int i = 0; i < noOfEdits; i++) {
      if (isPossible(arrayEditStructs[i], inputText, lineNo) && !boolDeleted) {

        editInput(Atext, Itext, Otext, pointer, inputText, arrayEditStructs[i]);
      }
    }

    //-----//Print all the stuff line by line - part of the while loop

    String printLine;
    // printf("lineNo : %d, boolean: %d", lineNo, boolDeleted);
    if (!boolDeleted) {
      if (strlen(Otext) > 2) {
        sprintf(printLine, "%s%s%s%s\n", Otext, Itext, inputText, Atext);
      } else {
        sprintf(printLine, "%s%s%s\n", Itext, inputText, Atext);
      }
    } else {
      sprintf(printLine, "%s\n", Otext);
    }
    while (strstr(printLine, "\n\n") != NULL) {
      strcpy(printLine, replaceFunction(printLine, "\n\n", "\n"));
    }
    // it's weird because strcmp will return 0 = false if they're the same? so just flow with it
    if (strcmp(printLine, "\n")) {
      printf("%s", printLine);
    }

    lineNo++;
    //   printf("line number: %d\n",lineNo);
  }
  //  printf("\n");
  // printf("line number: %d\n",lineNo);
  return 0;
}


