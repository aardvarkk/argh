#ifndef ARGH_H
#define ARGH_H

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class Option {
public:
  Option() : parsed(false) {}

  virtual std::string getDefault() = 0;
  virtual std::string getName() = 0;
  virtual std::string getMessage() = 0;
  virtual void setValue(std::string const& val) = 0;
  
  bool getParsed() { return parsed; }
  void setParsed(bool parsed) { this->parsed = parsed; }

protected:
  bool parsed;
};

template<typename T>
class OptionImpl : public Option {
public:
  OptionImpl(T& var, T default_val, std::string const& name, std::string const& msg) :
    var(var), 
    default_val(default_val), 
    name(name), 
    msg(msg) 
  { var = default_val; }

  std::string getDefault() { std::stringstream ss; ss << default_val; return ss.str(); }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) { std::stringstream ss(val); ss >> var; }

protected:
  T default_val;
  T& var;
  std::string name;
  std::string msg;
};

template<>
class OptionImpl<std::string> : public Option
{
public:
  OptionImpl(std::string& var, std::string const& default_val, std::string const& name, std::string const& msg) :
    var(var), 
    default_val(default_val), 
    name(name), 
    msg(msg) 
  { var = default_val; }

  std::string getDefault() { std::stringstream ss; ss << "\"" << default_val << "\""; return ss.str(); }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) { var = val; }

protected:
  std::string default_val;
  std::string& var;
  std::string name;
  std::string msg;
};

class FlagImpl : public Option {
public:
  FlagImpl(std::string const& name, std::string const& msg) :
    name(name),
    msg(msg) 
  {}

  std::string getDefault() { return ""; }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) {}

protected:
  std::string name;
  std::string msg;
};

template<typename T>
class MultiOptionImpl : public Option
{
public:
  MultiOptionImpl(std::vector<T>& var, std::string const& default_vals, std::string const& name, std::string const& msg, char delim) :
    var(var), 
    default_vals(default_vals),
    name(name), 
    msg(msg),
    delim(delim)
  {
    setValue(default_vals);
  }

  std::string getDefault() 
  {  
    std::stringstream ss; 
    ss << "\"";
    ss << default_vals;
    ss << "\"";
    return ss.str(); 
  }

  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) {
    var.clear();
    std::stringstream ss(val);
    T elem;
    for (std::string val_str; std::getline(ss, val_str, delim);) {
      std::stringstream st(val_str);
      st >> elem;
      var.push_back(elem);
    }
  }

protected:
  std::string default_vals;
  std::vector<T>& var;
  std::string name;
  std::string msg;
  char delim;
};

template<>
class MultiOptionImpl<std::string> : public Option
{
public:
  MultiOptionImpl(std::vector<std::string>& var, std::string const& default_vals, std::string const& name, std::string const& msg, char delim) :
    var(var), 
    default_vals(default_vals),
    name(name), 
    msg(msg),
    delim(delim)
  {
    setValue(default_vals);
  }

  std::string getDefault() 
  {  
    std::stringstream ss; 
    ss << "\"";
    ss << default_vals;
    ss << "\"";
    return ss.str(); 
  }

  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setValue(std::string const& val) {
    var.clear();
    std::stringstream ss(val);
    for (std::string val_str; std::getline(ss, val_str, delim);) {
      var.push_back(val_str);
    }
  }

protected:
  std::string default_vals;
  std::vector<std::string>& var;
  std::string name;
  std::string msg;
  char delim;
};

class Argh {
public:
  Argh(char delim = ',') : delim(delim) {}
  ~Argh() { for (auto o : options) { delete o; } options.clear(); }

  void parse(int argc, char const* argv[]) {
    for (int i = 0; i < argc; ++i) {
      for (auto o : options) {
        if (std::string(argv[i]) == o->getName()) {
          o->setParsed(true);
          if (i + 1 < argc) {
            o->setValue(argv[i + 1]);
          }
        }
      }
    }
  }

  template<typename T>
  void addOption(T& var, T const& default_val, std::string const& name, std::string const& msg = "") {
    options.push_back(new OptionImpl<T>(var, default_val, name, msg));
  }

  template<typename T>
  void addMultiOption(std::vector<T>& var, std::string const& default_vals, std::string const& name, std::string const& msg = "") {
    options.push_back(new MultiOptionImpl<T>(var, default_vals, name, msg, delim));
  }

  void addFlag(std::string const& name, std::string const& msg = "") {
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
        << std::setw(name_space)    << o->getName() 
        << std::setw(default_space) << o->getDefault() 
        << std::setw(msg_space)     << o->getMessage() 
        << std::endl;
    }
    return ret.str();
  }

  bool hasFlag(std::string const& flag) {
    for (auto o : options) {
      if (flag == o->getName() && o->getParsed()) {
        return true;
      }
    }
    return false;
  }

  void load(std::string const& filename) {
    std::ifstream ifs(filename);
    if (!ifs.good()) { return; }
    int argc = 0;
    std::vector<std::string> argv_str;
    std::vector<char const*> argv;
    std::string arg;
    while (std::getline(ifs, arg)) {
      argv_str.push_back(arg);
      ++argc;
    }
    for (int i = 0; i < argc; ++i) {
      argv.push_back(argv_str[i].c_str());
    }
    parse(argc, &*argv.begin());
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
  char delim;
};

#endif