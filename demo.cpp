#include <iostream>

#include "argh.h"

int main(int argc, char const* argv[])
{
  bool                     boolValue;
  float                    floatValue;
  int                      intValue;
  std::string              stringValue;
  std::vector<float>       multiValue;
  std::vector<std::string> multiStringValue;

  Argh argh;
  argh.addFlag("--help", "Display this message");
  argh.addOption<bool>(boolValue, false, "--boolvalue", "True? False?");
  argh.addOption<float>(floatValue, 3.14f, "--floatvalue", "Get real");
  argh.addOption<int>(intValue, 123, "--intvalue", "Making numbers whole");
  argh.addOption<std::string>(stringValue, "It's a default", "--stringvalue", "Tell me a story");
  argh.addMultiOption<float>(multiValue, "1.f,2.f,3.f", "--multivalue", "The more the merrier");
  argh.addMultiOption<std::string>(multiStringValue, "one,two,three", "--multistringvalue", "It's so easy!");

  argh.load("argh.opts");
  argh.parse(argc, argv);

  std::cout << argh.getUsage() << std::endl;

  if (argh.isParsed("--help")) {
    return EXIT_SUCCESS;
  }

  std::cout << "We have liftoff..." << std::endl;

  Argh argh_adv('|');
  std::vector<std::string> complex;
  argh_adv.addMultiOption<std::string>(complex, "one|two", "--complex");
  const int argc_adv = 2;
  char const* argv_adv[argc_adv] = { "--complex", "o n e|t w o|t h r e e" };
  argh_adv.parse(argc_adv, argv_adv);

  return EXIT_SUCCESS;
}