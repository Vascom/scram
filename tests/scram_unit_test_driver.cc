#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>

#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
  const char* target = "/home/olzhas/projects/scram/build/bin";
  assert(chdir(target) == 0);
  testing::InitGoogleTest ( &argc, argv );
  return RUN_ALL_TESTS();
}
