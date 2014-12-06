#include <malloc.h>
#include "unity.h"
#include "Preprocessor.h"
#include "StringObject.h"
#include "ErrorCode.h"
#include "CException.h"
#include "Macro.h"
#include "RedBlackTree.h"
#include "Node.h"
#include "Rotations.h"
#include "InitNode.h"
#include "CustomAssertions.h"
#include "LinkedList.h"

#define Apple _AB)  389+_AB

void setUp(void)  {}

void tearDown(void) {}
/***************************************************************/
/**                                                           **/
/**  This TESTCODE is used to test define function as macro   **/
/**                                                           **/
/***************************************************************/

/** test createMacroInfo() given macroName with bracket symbol
 **/
void Xtest_createMacroInfo_given_macroName_with_bracket_symbol(void)
{
	String *str = stringNew("Add(X) X + 3");
  Macro *macro;
  Argument *argu;

  printf("Start test_createMacroInfo_given_macroName_with_bracket_symbol\n");
  argu = createMacroArguments(str);
  printf("------------------------------------------------------------\n");

  TEST_ASSERT_NOT_NULL(argu);
  // TEST_ASSERT_EQUAL_STRING("Add", macro->name->string);
  // TEST_ASSERT_EQUAL_STRING("X + 3", macro->content->string); //empty content

  delMacroNameAndContent(macro);
  stringDel(str);
}

/** test createMacroInfo() given macroName with bracket symbol and comma
 **/
void Xtest_createMacroInfo_given_macroName_with_several_argument(void)
{
	String *str = stringNew("Sam(a,b) a + b");
  Macro *macro;

  printf("Start test_createMacroInfo_given_macroName_with_several_argument\n");
  macro = createMacroInfo(str);
  printf("------------------------------------------------------------\n");

  TEST_ASSERT_NOT_NULL(macro);
  TEST_ASSERT_EQUAL_STRING("Sam", macro->name->string);
  TEST_ASSERT_EQUAL_STRING("a + b", macro->content->string); //empty content

  delMacroNameAndContent(macro);
  stringDel(str);
}

/** test macroPositionInString():
 ** macro :
 *          #define Add(X) 10 + X
 *          Sum = Add(5)
 ** String :
 *          Search Something
 **/
void Xtest_macroPositionInString_given_Add_5_should_get_the_Add_5_position(void) // <----- Problem
{
	String *str = stringNew("#define Add(X) 10 + X\n"
                           "Sum = Add(5)");
  String *subString;
  LinkedList *head = NULL;

  printf("Start test_macroPositionInString_given_string_X_trying_to_search_Macro_should_get_search_macroInfo\n");
  Node *root = addAllMacroIntoTree(str, "define");
  subString = macroPositionInString(str, root);
  printf("------------------------------------------------------------\n");

  TEST_ASSERT_NOT_NULL(subString);
  TEST_ASSERT_EQUAL(0, subString->startindex);
  TEST_ASSERT_EQUAL(0, subString->length);

  destroyAllMacroInTree(root);
  stringDel(str);
}