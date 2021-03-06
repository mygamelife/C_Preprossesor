#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "Preprocessor.h"

/** Find character hashTag at the first position of the string
 * input :
 *			*string is String type pointer
 * output :
 *			return 1 if hashTag found
 *			return 0 if hashTag not found
 **/
int isHashTag(String *string) {

  stringTrim(string);

  if(string->string[string->startindex] == '#')
    return 1;

  else return 0;
}

/** Find directive inside the string
 * input :
 *			*string is String type pointer
 *      *directiveName is directive name
 * output :
 *			return 1 if directive name found
 *			throw ERR_INVALID_DIRECTIVE if directive name not found
 **/
int isDirective(String *string, char *directiveName) {
  char *tempString;
  String *directive;

  stringTrim(string);

  //search for directive
  directive = stringRemoveWordContaining(string, directiveName);

  tempString = stringSubStringInChars(directive, directive->length);

  if(strcmp(directiveName, tempString) == 0)  {
    subStringDel(tempString);
    stringDel(directive);
    return 1;
  }

  else  {
    subStringDel(tempString);
    stringDel(directive);
    Throw(ERR_INVALID_DIRECTIVE);
  }
}

/** Looking for identifier
 *
 **Identifier valid format  :
 * -nondigit
 * -identifier nondigit
 * -identifier digit
 *
 * input :
 *			*string is String type pointer
 * output :
 *			return 1 if identifier found
 *			throw ERR_INVALID_IDENTIFIER if identifier not found
 **/
int isIdentifier(String *str)  {
  String *identifier = str;
  int i = 0, j = 0;
  
  stringTrimLeft(identifier);  
  if(stringCharAtInSet(identifier, identifier->startindex, alphaSet)) {
    j = identifier->startindex;
    for(i; i < identifier->length ; i++) {
      if(!stringCharAtInSet(identifier, j, alphaNumericSet))  {
        if(identifier->string[j] == ' ' || identifier->string[j] == '\\' || identifier->string[j] == '\n' || identifier->string[j] == '(')
          break;
        else  Throw(ERR_INVALID_IDENTIFIER);
      }
      j++;
    }
    return 1;
  }
  else Throw(ERR_INVALID_IDENTIFIER);
}

/** getSizeOfArgu(String *str, char *containSet)
 *  This function is used to determine how many argument that macro content had
 * input :
 *			*str is the macro arugment list in string form
 *      *containSet only take identifier as arguments
 * output :
 *			return sizeOfArgu if found
 *      else return 0
 **/
int getSizeOfArgu(String *str, char *containSet) {
  int sizeOfMacro = 0;
  String *arguments;

  if(str->string[str->startindex] == ')')
    return sizeOfMacro;

  do  {
    arguments = stringRemoveWordContaining(str, containSet); /**need free**/
    // printf("argument start %d, length %d\n", arguments->startindex, arguments->length);
    if(arguments->length == 0)  {
      stringDel(arguments);
      break;
    }
    else if(isIdentifier(arguments)) {
      // printf("argument start %d, length %d\n", arguments->startindex, arguments->length);
      sizeOfMacro++;
    }
    stringTrim(str);
    stringDel(arguments);
  }while(str->string[str->startindex] == ',');

  // printf("sizeOfMacro %d\n" ,sizeOfMacro);
  return sizeOfMacro;
}

/** getSizeOfArguInString(String *str, char *containSet)
 *  This function is used to determine how many argument in string statement
 * input :
 *			*str is the macro arugment list in string form
 *      *containSet only take identifier as arguments
 * output :
 *			return sizeOfArgu if found
 *      else return 0
 **/
int getSizeOfArguInString(String *str, char *containSet) {
  int sizeOfMacro = 0;
  String *arguments;

  do  {
    stringTrim(str);
    arguments = stringRemoveWordContaining(str, containSet);
    if(arguments->length == 0)  {
      stringDel(arguments);
      break;
    }
    sizeOfMacro++;

    /* free before re-use */
    stringTrim(str);
    stringDel(arguments);
  }while(str->string[str->startindex] == ',');

  return sizeOfMacro;
}

/** *createMacroArguments(String *str)
 * input :
 *			*string is a given String
 * output :
 *			return macroArgument if bracket "()" is found
 *			else return NULL
 **/
Argument *createMacroArguments(String *str, char *containSet) {
  int sizeOfArgu = 0, i = 0, start = 0, end = 0;
  char *macroArgument;
  String *arguments;
  Argument *argu;

  if(str->string[str->startindex] == '(') {
    str->startindex++;
    str->length--;
    start = str->startindex;
    // printf("str->startindex %d\n", str->startindex);
    sizeOfArgu = getSizeOfArgu(str, alphaNumericSetWithSymbolWithoutBracket);

    if(str->string[str->startindex] == ')') {
      /* reset string position to initial */
      end = str->startindex;
      str->startindex = start;
      str->length = strlen(str->string);

      argu = newMacroArgument(sizeOfArgu);
      argu->withArgument = 1;

      if(sizeOfArgu == 0) {
        if(str->string[end] == ')')
          str->startindex++;
        return argu;
      }

      for(i ; i < sizeOfArgu ; i++) {
        arguments = stringRemoveWordContaining(str, containSet); /**need free**/
        macroArgument = stringSubStringInChars(arguments , arguments->length);
        argu->entries[i]->name = stringNew(macroArgument);
        argu->entries[i]->value = NULL;
        stringDel(arguments);
      }

      /* string position after done with the argument */
      str->startindex = end + 1;
      str->length = strlen(str->string) - str->startindex;
    }

    else if(str->string[str->startindex] == ',')
      Throw(ERR_EXPECT_IDENTIFIER);

    else Throw(ERR_EXPECT_CLOSED_BRACKET);

    return argu;
  }
  else  {
      argu = newMacroArgument(0);
      argu->withArgument = 0;
      return argu;
  }
}

/** Extract the MacroInfo out from the given string
 * input :
 *			*string is String type pointer
 * output :
 *			return macroInfo contain all the macro information
 *			return NULL if macro information is not found
 **/
Macro *createMacroInfo(String *str) {
  char *macroName, *macroContent, *macroContentPart, EOL = '\n';
  String *name, *content;
  Macro *macroInfo;
  Argument *macroArguments = NULL;

  name = stringRemoveWordContaining(str, alphaNumericSet);

  if(name->length == 0) //Throw error name is empty
    Throw(ERR_EMPTY_MACRO_NAME);

  macroName = stringSubStringInChars(name , name->length);
  // printf("str after removed name, start %d, length %d\n", str->startindex, str->length);
  macroArguments = createMacroArguments(str, alphaNumericSet);

  /*------------------------------------------------------------------------------*/
  stringTrimUntilEOL(str);

  if(str->string[str->startindex] == EOL || str->string[str->startindex] == '\0') {
    macroContent = malloc(sizeof(char) * 1);
    macroContent[0] = '\0';
    return macroInfo = newMacro(macroName, macroContent, macroArguments); //" " this space indicate it is empty content
  }
  content = stringRemoveWordContaining(str, alphaNumericSetWithSymbol);
  macroContent = stringSubStringInChars(content , content->length);

  if(str->string[str->startindex] == '\\')  {
    content = stringRemoveWordContaining(str, alphaNumericSetWithSymbol);
    macroContentPart = stringSubStringInChars(content , content->length);
    strcat(macroContent, macroContentPart);
  }
  return macroInfo = newMacro(macroName, macroContent, macroArguments); //need to free malloc
}

/** verifyRedifineArguments(Macro *macro)
 * This function is to verify redifined arguments in macro
 * input :
 *			*macro contain macroName, macroContent, macroArguments
 * output :
 *      return 1 if redifined arguments is found
 *      return 0 if redifined arguments is not found
 **/
int verifyRedifineArguments(Macro *macro) {
  int i = 0, j = 0;

  if(macro->argument != NULL) {
    if(macro->argument->size != 0)  {
      for(i ; i < macro->argument->size ; i++)  {
        for(j ; j < (macro->argument->size - i - 1); j++)  {
          if(!strcmp(macro->argument->entries[i]->name->string, macro->argument->entries[j+i+1]->name->string))  {
            return 1;
          }
        }
        j = 0;
      }
    }
  }
  return 0;
}

/** Replace all the words if it is a macro
 * input :
 *			*string is String type pointer
 *      *directiveName is C directive name
 * output :
 *      return the address of root pointer
 **/
Node *addAllMacroIntoTree(String *string, char *directiveName) {
  int i = 0, j = 0;
  Node *macroNode, *root = NULL;
  Macro *macro, *foundMacro;

  while(isHashTag(string)) {
    if(isDirective(string, directiveName))  {
      if(isIdentifier(string))  {
        macro = createMacroInfo(string);
        foundMacro = findMacroInTree(root, macro->name->string);

        if(verifyRedifineArguments(macro))
          Throw(ERR_MACRO_ARGUMENTS_REDEFINED);

        else if(foundMacro != NULL)  { //check redefined macro
          Throw(ERR_MACRO_REDEFINED);
        }
        macroNode = macroNodeNew(macro); //need to free malloc
        addMacro(&root, macroNode);
      }
    }
  }
  return root;
}

/** findMacroInTree(Node *root, char *targetMacro)
 * This function is use for finding the targetMacro name inside the tree
 *
 * input :
 *          *root is the pointer pointing to the tree root
 *          *targetMacro is a macro name that we are trying to search in tree
 * output :
 *          return the macro pointer contain macro information if found
 *          return NULL if cant find the macro name inside the tree
 **/
Macro *findMacroInTree(Node *root, char *targetMacro)  {
  Macro *macro;
  char *macroString;

  if(root != NULL)  {
    macro = (Macro*)root->dataPtr;
    macroString = macro->name->string;

    if(strcmp(macroString, targetMacro) == 0) {
      return macro;
    }

    else if(macro->name->string[0] >= targetMacro[0])
      findMacroInTree(root->left, targetMacro);

    else findMacroInTree(root->right, targetMacro);
  }
  else  return NULL;
}

/** *getMacroPositionInString(String *str, Node *root)
 * This function is use to search pre-defined macro in string
 *
 ** input :
 *          *str is a pointer pointing to the given string
 *          *root is a pointer pointing to the macro tree
 ** output :
 *          If found return foundMacro pointer contain macro information
 *          Else return NULL
 **/
String *getMacroPositionInString(String *str, Node *root) {
  char *token;
  String *subString = NULL, *arguList = NULL;
  Macro *foundMacro = NULL;

  subString = stringRemoveWordContaining(str, alphaNumericSet);
  while(subString->length != 0)  {
    token = stringSubStringInChars(subString, subString->length);
    foundMacro = findMacroInTree(root, token);
    free(token);
    if(foundMacro != NULL)  {
      break;
    }
    free(subString);
    subString = stringRemoveWordContaining(str, alphaNumericSet);
  }
  return subString;
}

String *getArgumentPositionInString(String *str, char *argumentName) {
  char *token;
  
  String *subString = stringRemoveWordContaining(str, alphaNumericSet);
  while(subString->length != 0)  {
    token = stringSubStringInChars(subString, subString->length);
    if(!strcmp(token, argumentName))  {
      free(token);
      return subString;
    }
    free(token);
    free(subString);
    subString = stringRemoveWordContaining(str, alphaNumericSet);
  }
  return subString;
}

/** *modifyMacroPositionWithArguments(String *str, Macro *macro)
 * This function is use to modify macro position to macro position with arguments
 *
 ** input :
 *          *macroSubString is a pointer pointing to the current macro position
 *          *macro contain macro information (name, content, arguments)
 ** output :
 *          modify if withArgument is 1
 *          return if withArgument is 0
 **/
void modifyMacroPositionWithArguments(String *macroSubString, Macro *foundMacro) {
  int start = 0, arguLength = 0;

  start = macroSubString->startindex;
  macroSubString->startindex = macroSubString->startindex + foundMacro->name->length;
  if(foundMacro->argument->withArgument == 1 && macroSubString->string[macroSubString->startindex] != '(')  {
    Throw(ERR_EXPECT_ARGUMENT);
  }
  else if(foundMacro->argument->size != 0 && macroSubString->string[macroSubString->startindex] == '(') {
    do  {
      arguLength++;
    }while(macroSubString->string[macroSubString->startindex++] != ')');
    macroSubString->startindex = start;
    macroSubString->length = arguLength + foundMacro->name->length;
  }
}

/** *replaceMacroInString(String *oriString, String *subString, Macro *macro, int size)
 * This function is use to replace macro found inside the string
 *
 ** input :
 *          *latestString is a pointer pointing to latest update string statement
 *          *macroSubString is the position of macro inside string
 *          *macro is a pointer contain macro information
 *          size is the size of string after replaced with macro content
 ** output :
 *          return the character string after replaced macro name with macro content
 **/
char *replaceMacroInString(String *latestString, String *macroSubString, Macro *foundMacro, int size) {
  int i = 0, j = 0, start = 0;
  char *replacedMacroInString = malloc(sizeof(char) * size + 1);

  if(foundMacro != NULL)   {
    for(i; i< size ; i++) {
      if(start == macroSubString->startindex) {
        for(j; j< strlen(foundMacro->content->string) ; j++)
          replacedMacroInString[i++] = foundMacro->content->string[j];
        start += macroSubString->length;
      }
      if(i == size) break;
      replacedMacroInString[i] = latestString->string[start++];
    }
    replacedMacroInString[i] = '\0';
  }

  else  {
    for(i; i< size ; i++)
      replacedMacroInString[i] = latestString->string[start++];
    replacedMacroInString[i] = '\0';
  }
  // printf("replacedMacroInString %s\n", replacedMacroInString);
  return replacedMacroInString;
}

/** storeArgumentsInString(Macro *macro)
 * This function is to store arguments in string statement into macro structure
 ** input :
 *          *str is a pointer pointing to the string statement
 *          *macro is a pointer contain macro information
 ** output :
 *          store arguments into macro structure
 **/
void storeArgumentsInString(String *str, Macro *macro) {
  int sizeOfArgu = 0, start = 0, length = 0, i = 0;
  char *macroArgument;
  String *arguments;

  if(macro->argument != NULL) {
    if(macro->argument->withArgument == 1)  {
      if(str->string[str->startindex] == '(') {
        start = str->startindex++;
        length = str->length--;
        sizeOfArgu = getSizeOfArguInString(str, alphaNumericSetWithSymbolWithoutBracket);
        if(sizeOfArgu == macro->argument->size) {
          if(str->string[str->startindex] == ')') {
            str->startindex = start;
            str->length = length;
            for(i; i < sizeOfArgu; i++) {
              arguments = stringRemoveWordContaining(str, alphaNumericSetWithSymbolWithoutBracket);
              macroArgument = stringSubStringInChars(arguments , arguments->length);
              macro->argument->entries[i]->value = stringNew(macroArgument);
              stringDel(arguments);
            }
          }
          else Throw(ERR_EXPECT_CLOSED_BRACKET);
        }
        else Throw(ERR_MISMATCH_ARGUMENT_SIZE);
      }
      else Throw(ERR_EXPECT_ARGUMENT);
    }
    else  {
      if(str->string[str->startindex] == '(')
        Throw(ERR_EXPECT_NO_ARGUMENT);
    }
  }
}

/** char *replaceArgumentsInString(String *latestString, String *macroSubString, Macro *foundMacro, int size)
 * This function is use to replace all the predefined arguments in string
 *
 ** input :
 *          *latestString is a pointer pointing to latest update string statement
 *          *argumentSubString is the position of arguments inside string
 *          *foundMacro is a pointer contain macro information
 *          size is the size of string after replaced with arguments
 ** output :
 **/
char *replaceArgumentsInString(String *str, String *argumentSubString, char *argumentValue, int size) {
  int i = 0, j = 0, valueLength, start = 0;
  char *replacedArgumentsString = malloc(sizeof(char) * size + 1);

  valueLength = strlen(argumentValue);

  for(i; i< size ; i++) {
    if(start == argumentSubString->startindex) {
      for(j; j< valueLength ; j++)
        replacedArgumentsString[i++] = argumentValue[j];
        start += argumentSubString->length;
      }
    if(i == size)
      break;
    replacedArgumentsString[i] = str->string[start++];
  }

  replacedArgumentsString[i] = '\0';
  return replacedArgumentsString;
}

/** char *searchAndReplaceArgumentsInString(char *replacedMacroString, Macro *foundMacro)
 * This function is use to replace all the predefined arguments inside the string
 *
 ** input :
 *          replacedMacroString is the string placed with the macro content
 *          foundMacro contain the macro information
 ** output :
 *          return replaced string with arguments if withArgument is 1
 *          return if withArgument is 0
 **/
char *searchAndReplaceArgumentsInString(char *replacedMacroString, Macro *foundMacro) {
  String *argumentSubString, *str;
  int size, valueLength, start = 0, i = 0, j = 0, k = 0;
  char *replacedArgumentsString, *argumentValue = NULL;

  if(foundMacro != NULL)  {
    if(foundMacro->argument->withArgument == 0)
      return replacedMacroString;  
    else if(foundMacro->argument->size != 0 && foundMacro->argument->withArgument != 0)  {
      str = stringNew(replacedMacroString);
      argumentSubString = getArgumentPositionInString(str , foundMacro->argument->entries[k]->name->string);
      while(argumentSubString->length != 0) {
        size = strlen(str->string) - argumentSubString->length + foundMacro->argument->entries[k]->value->length;
        argumentValue = foundMacro->argument->entries[k]->value->string;
        replacedArgumentsString = replaceArgumentsInString(str, argumentSubString, argumentValue, size);
        if(k == foundMacro->argument->size - 1) break;
        
        subStringDel(str->string);
        stringDel(str);
        str = stringNew(replacedArgumentsString);
        stringDel(argumentSubString);
        argumentSubString = NULL;
        k++;
      
        argumentSubString = getArgumentPositionInString(str ,foundMacro->argument->entries[k]->name->string);
      }
      stringDel(str);
      stringDel(argumentSubString);
      return  replacedArgumentsString;
    }
  }
}

/** String *directiveDefine(String *str, char *directiveName)
 * This function is use to replace all the predefined macro and macro argument
 ** input :
 *          str is the given string
 *          directiveName is the cpp valid directive name
 ** output :
 *          return string with completed replace all the macros and arguments
 **/
String *directiveDefine(String *str, char *directiveName) {
  int size, count = 0, cyclic = 0, nextToCyclic = 0;
  char *macroToken, *stringStatement, *replacedMacroString = NULL, *finalString = NULL;
  String *macroSubString, *latestString;
  Macro *foundMacro;
  LinkedList *head = NULL, *newList = NULL;

  Node *root = addAllMacroIntoTree(str, directiveName); //add all predefined macro into tree
  stringStatement = stringSubStringInChars(str, str->length);
  latestString = stringNew(stringStatement);
  macroSubString = getMacroPositionInString(latestString, root);
  
  while(macroSubString->length != 0) {
    macroToken = stringSubStringInChars(macroSubString, macroSubString->length);
    foundMacro = findMacroInTree(root, macroToken);
    if(foundMacro != NULL)  {//get size of replace macro string
      size = strlen(latestString->string) - (foundMacro->name->length) + (foundMacro->content->length);
      if(foundMacro->argument->withArgument)  {
        modifyMacroPositionWithArguments(macroSubString, foundMacro);
        size = strlen(latestString->string) - macroSubString->length + (foundMacro->content->length);
        storeArgumentsInString(latestString, foundMacro);
      }
        
      cyclic = findList(&head, foundMacro->name->string);
      if(cyclic)  {
        nextToCyclic = latestString->startindex;
        destroyAllLinkedLists(head);
        head = NULL;
      }
      else  {
        newList = linkListNew(foundMacro->name->string); //Create new list
        addLinkedList(&head, newList);
      }
    }
    else  size = strlen(latestString->string);

   if(!cyclic)  {
      replacedMacroString = replaceMacroInString(latestString, macroSubString, foundMacro, size);
      finalString = searchAndReplaceArgumentsInString(replacedMacroString, foundMacro);
      subStringDel(latestString->string);
      stringDel(latestString);
      latestString = stringNew(finalString);
   }
   if(nextToCyclic)  //always start at next to cyclic happen
    latestString->startindex = nextToCyclic;

    stringDel(macroSubString);
    macroSubString = getMacroPositionInString(latestString, root);
    subStringDel(macroToken);
  }
  stringDel(macroSubString);
  destroyAllMacroInTree(root);
  destroyAllLinkedLists(head);

  latestString->startindex = 0;
  latestString->length = strlen(latestString->string);
  return latestString;
}
