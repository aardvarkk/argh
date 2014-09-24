Argh!
====

Seriously? You made another command line parser? Yep. Deal with it.

## Features

* A single header file `argh.h`
* No dependencies
* Configuration file support
* Supports both flags and options that take an argument
* Supports variable-length argument lists

## Demos

### Easy

* Create a flag `addFlag()`
* Create an option with an argument `addOption<T>`
* Parse the command line options `parse()`
* Print the usage `getUsage()`

```cpp
int demo_easy(int argc, char const* argv[])
{
  std::cout << "::: Babytown Frolics! :::" << std::endl;

  bool help;
  int  mynumber;

  Argh argh;
  argh.addFlag(help, "--help", "Display this message");
  argh.addOption<int>(mynumber, 123, "--mynumber", "The default value here is 123");
  argh.parse(argc, argv);

  if (help) {
    std::cout << argh.getUsage() << std::endl;
  }

  std::cout << "mynumber = " << mynumber << std::endl;
  std::cout << std::endl;

  return EXIT_SUCCESS;
}
```

### Medium

* Use different data types `bool`, `float`
* Use variable-length options `std::vector<float>`
* Load from a config file `load()`

```cpp
int demo_medium(int argc, char const* argv[])
{
  std::cout << "::: I've been around the block! :::" << std::endl;

  bool                     boolValue;
  float                    floatValue;
  int                      intValue;
  std::string              stringValue;
  std::vector<float>       multiValue;
  std::vector<std::string> multiStringValue;

  Argh argh;
  argh.addOption<bool>(boolValue, false, "--boolvalue", "True? False?");
  argh.addOption<float>(floatValue, 3.14f, "--floatvalue", "Get real");
  argh.addOption<int>(intValue, 123, "--intvalue", "Optional description");
  argh.addOption<std::string>(stringValue, "It's a default", "--stringvalue");
  argh.addMultiOption<float>(multiValue, "1.f,2.f,3.f", "--multivalue", "See? Look up. Told you it was optional!");
  argh.addMultiOption<std::string>(multiStringValue, "one,two,three", "--multistringvalue", "It's so easy!");

  argh.load("argh.opts");
  argh.parse(argc, argv);

  std::cout << argh.getUsage() << std::endl;

  if (argh.isParsed("--multivalue")) {
    std::cout << "multiValue changed from the default to:" << std::endl;
    for (auto v : multiValue) {
      std::cout << v << std::endl;
    }
  }
  std::cout << std::endl;

  return EXIT_SUCCESS;
}
```

### Hard

* Use a different delimiter for variable-length options `Argh argh('|');`

```cpp
int demo_hard(int argc, char const* argv[])
{
  std::cout << "::: Futurejet fighter pilot! :::" << std::endl;

  Argh argh('|');
  std::vector<std::string> complex;
  argh.addMultiOption<std::string>(complex, "one|two", "--complex");
  argh.parse(argc, argv);

  std::cout << argh.getUsage() << std::endl;

  std::cout << "You so crazy. complex is:" << std::endl;
  for (auto c : complex) {
    std::cout << c << std::endl;
  }
  std::cout << std::endl;

  return EXIT_SUCCESS;
}
```
