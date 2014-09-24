#ifndef ARGH_H
#define ARGH_H

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class Option {
public:
  virtual std::string getDefault() = 0;
  virtual std::string getName() = 0;
  virtual std::string getMessage() = 0;
  virtual void setValue(std::string const& val) = 0;
};

template<typename T>
class ArgumentImpl : public Option {
public:
  ArgumentImpl(T& var, std::string const& name, std::string const& msg) : var(var), name(name), msg(msg) {}
  std::string getDefault() { std::stringstream ss; ss << var; return ss.str(); }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) { std::stringstream ss(val); ss >> var; }

protected:
  T& var;
  std::string name;
  std::string msg;
};

class FlagImpl : public Option {
public:
  FlagImpl(std::string const& name, std::string const& msg) : name(name), msg(msg) {}
  std::string getDefault() { return ""; }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) {}

protected:
  std::string name;
  std::string msg;
};

class Argh {
public:

  //enum Style {
  //  kArgument,
  //  kFlag
  //};

  enum Source {
    kDefault,
    kCommandLine,
    kFile
  };

  ~Argh() { for (auto o : options) { delete o; } options.clear(); }

  void parse(int argc, char const* argv[]) {
    for (int i = 0; i < argc; ++i) {
      for (auto o : options) {
        if (std::string(argv[i]) == o->getName()) {
          o->setValue(argv[i + 1]);
        }
      }
    }
  }

  template<typename T>
  void addOption(T& var, T const& default_val, std::string const& name, std::string const& msg) {
    options.push_back(new ArgumentImpl<T>(var, name, msg));
    var = default_val;
  }

  void addFlag(std::string const& name, std::string const& msg) {
    options.push_back(new FlagImpl(name, msg));
  }

  std::string getUsage() {
    size_t name_space    = getLongestName()    + 1;
    size_t default_space = getLongestDefault() + 1;
    size_t msg_space     = getLongestMessage() + 1;

    std::stringstream ret;
    ret << std::left;
    for (auto o : options) {
      ret 
        << std::setw(name_space) << o->getName() 
        << std::setw(default_space) << o->getDefault() 
        <<std::setw(msg_space) << o->getMessage() 
        << std::endl;
    }
    return ret.str();
  }

  bool hasFlag(std::string const& flag, int argc, char const* argv[]) {
    for (int i = 0; i < argc; ++i) {
      if (flag == std::string(argv[i])) {
        return true;
      }
    }
    return false;
  }

protected:

  size_t getLongestName() {
    size_t ret = 0;
    for (auto o : options) {
      ret = std::max(ret, o->getName().length());
    }
    return ret;
  }

  size_t getLongestDefault() {
    size_t ret = 0;
    for (auto o : options) {
      ret = std::max(ret, o->getDefault().length());
    }
    return ret;
  }

  size_t getLongestMessage() {
    size_t ret = 0;
    for (auto o : options) {
      ret = std::max(ret, o->getMessage().length());
    }
    return ret;
  }

  std::vector<Option*> options;
};

#endif