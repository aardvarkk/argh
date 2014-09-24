#include <iostream>

#include "argh.h"

int main(int argc, char const* argv[])
{
  bool        boolValue;
  float       floatValue;
  int         intValue;
  std::string stringValue;

  Argh argh;
  argh.addFlag("--help", "Display this message");
  argh.addOption<bool>(boolValue, false, "--boolvalue", "True? False?");
  argh.addOption<float>(floatValue, 3.14f, "--floatvalue", "Get real");
  argh.addOption<int>(intValue, 123, "--intvalue", "Making numbers whole");
  argh.addOption<std::string>(stringValue, "It's a default", "--stringvalue", "Tell me a story");

  argh.load("argh.opts");
  argh.parse(argc, argv);

  std::cout << argh.getUsage() << std::endl;

  if (argh.hasFlag("--help")) {
    return EXIT_SUCCESS;
  }

  std::cout << "We have liftoff..." << std::endl;

  return EXIT_SUCCESS;
}