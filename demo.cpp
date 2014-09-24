#include <iostream>

#include "argh.h"

int main(int argc, char const* argv[])
{
  int intValue;

  Argh argh;
  argh.addFlag("--help", "Display this message");
  argh.addOption<int>(intValue, 1234, "--int_value", "An integer value");

  std::cout << argh.getUsage() << std::endl;

  return EXIT_SUCCESS;
}