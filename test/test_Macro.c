#include "unity.h"
#include "Macro.h"
#include "StringObject.h"

void setUp(void)  {}

void tearDown(void) {}

/** test newMacro given name "A" and content "123"
 *
 *    *macro  --> String *name    = "A"
 *            --> String *content = "123"
 */
void test_newMacro_given_name_A_and_content_123(void)
{
  Macro *macro = newMacro("A", "123");

  //Name
  TEST_ASSERT_EQUAL("A", macro->name->string);
  TEST_ASSERT_EQUAL(0, macro->name->startindex);
  TEST_ASSERT_EQUAL(1, macro->name->length);

  //Content
  TEST_ASSERT_EQUAL("123", macro->content->string);
  TEST_ASSERT_EQUAL(0, macro->content->startindex);
  TEST_ASSERT_EQUAL(3, macro->content->length);

  //free malloc
  delMacro(macro);
}

/* test newMacroArgument given arguments size
 */
void Xtest_newMacroArgument_given_arguments_size_3(void)
{
  Argument *argu = newMacroArgument(3);

  TEST_ASSERT_EQUAL(3, argu->size);
  TEST_ASSERT_NULL(argu->entries[0]);

  //free malloc
  delMacroArgument(argu);
}