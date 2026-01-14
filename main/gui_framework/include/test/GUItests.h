#ifndef GUITESTS_H
#define GUITESTS_H
#include "unity.h"  // The core Unity testing header

void test_VBox_is_properly_initiated();
void test_HBox_is_properly_initiated();
void test_Label_is_properly_initiated();
void test_Container_add_child_recursive_delete();
void test_Nested_Container_Layout();
void test_Safety_Null_Inputs();
void test_VBox_Full_Height_Distribution();
void test_HBox_Full_Width_Distribution();
#endif  // GUITESTS_H