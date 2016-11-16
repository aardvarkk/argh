#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class Option {
public:
  Option() : parsed(false), required(false) {}
	virtual ~Option() {};

  virtual std::string getDefault()  = 0;
	virtual std::string getMessage()  = 0;
  virtual std::string getName()     = 0;
  virtual void        setValue(std::string const& val) = 0;

  bool         getParsed()            { return parsed;   }
	bool         getRequired()          { return required; }
	
  virtual void setParsed(bool parsed) { this->parsed = parsed; }

protected:
  bool parsed, required;
};

template<typename T>
class OptionImpl : public Option {
public:
  OptionImpl(T& var, T default_val, std::string const& name, bool required, std::string const& msg) : var(var)
  {
		this->default_val = default_val;
		this->name = name;
		this->required = required;
		this->msg = msg;
		this->var = default_val;
	}

  virtual std::string getDefault() { std::stringstream ss; ss << default_val; return ss.str(); }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  virtual void setValue(std::string const& val) { std::stringstream ss(val); ss >> var; }

protected:
  T default_val;
  T& var;
  std::string name;
  std::string msg;
};

class OptionStringImpl : public OptionImpl<std::string>
{
public:
  OptionStringImpl(std::string& var, std::string const& default_val, std::string const& name, bool required, std::string const& msg) :
    OptionImpl(var, default_val, name, required, msg)
  {}

  std::string getDefault() { std::stringstream ss; ss << "\"" << default_val << "\""; return ss.str(); }
  void setValue(std::string const& val) { var = val; }
};

template<typename T>
class MultiOptionImpl : public Option
{
public:
  MultiOptionImpl(std::vector<T>& var, std::string const& default_vals, std::string const& name, bool required, std::string const& msg, char delim) : var(var)
  {
		this->default_vals = default_vals;
		this->name = name;
		this->required = required;
		this->msg = msg;
		this->delim = delim;
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
  virtual void setValue(std::string const& val) {
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

class MultiOptionStringImpl : public MultiOptionImpl<std::string>
{
public:
  MultiOptionStringImpl(std::vector<std::string>& var, std::string const& default_vals, std::string const& name, bool required, std::string const& msg, char delim) :
    MultiOptionImpl(var, default_vals, name, required, msg, delim)
  {}

  void setValue(std::string const& val) {
    var.clear();
    std::stringstream ss(val);
    for (std::string val_str; std::getline(ss, val_str, delim);) {
      var.push_back(val_str);
    }
  }
};

class FlagImpl : public Option {
public:
  FlagImpl(bool& flag, std::string const& name, std::string const& msg) :
    flag(flag),
    name(name),
    msg(msg)
  {
    flag = false;
  }

  std::string getDefault() { return ""; }
  std::string getName() { return name; }
  std::string getMessage() { return msg; }
  void setParsed(bool parsed) { Option::setParsed(parsed); flag = parsed; }
  void setValue(std::string const& val) {}

protected:
  bool& flag;
  std::string name;
  std::string msg;
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
	
	void parseEnv() {
		for (auto o : options) {
			auto str = getenv(o->getName().c_str());
			if (str) {
				o->setParsed(true);
				o->setValue(str);
			}
		}
	}

  template<typename T>
  void addOption(T& var, T const& default_val, std::string const& name, bool required = false, std::string const& msg = "") {
    options.push_back(new OptionImpl<T>(var, default_val, name, required, msg));
  }

  void addOption(std::string& var, std::string const& default_val, std::string const& name, bool required = false, std::string const& msg = "") {
    options.push_back(new OptionStringImpl(var, default_val, name, required, msg));
  }

  template<typename T>
  void addMultiOption(std::vector<T>& var, std::string const& default_vals, std::string const& name, bool required = false, std::string const& msg = "") {
    options.push_back(new MultiOptionImpl<T>(var, default_vals, name, required, msg, delim));
  }

  void addMultiOption(std::vector<std::string>& var, std::string const& default_vals, std::string const& name, bool required = false, std::string const& msg = "") {
    options.push_back(new MultiOptionStringImpl(var, default_vals, name, required, msg, delim));
  }

  void addFlag(bool& flag, std::string const& name, std::string const& msg = "") {
    options.push_back(new FlagImpl(flag, name, msg));
  }

  std::string getUsage() {
    size_t name_space    = getLongestName()    + 1;
    size_t default_space = getLongestDefault() + 1;
    size_t msg_space     = getLongestMessage() + 1;

    std::stringstream ret;
    ret << std::left;
    for (auto o : options) {
      ret
        << std::setw(static_cast<int>(name_space))    << o->getName()
        << std::setw(static_cast<int>(default_space)) << o->getDefault()
        << std::setw(static_cast<int>(msg_space))     << o->getMessage()
				<< (o->getRequired() ? "REQUIRED" : "NOT REQUIRED")
        << std::endl;
    }
    return ret.str();
  }

  bool isParsed(std::string const& name) {
    for (auto o : options) {
      if (name == o->getName() && o->getParsed()) {
        return true;
      }
    }
    return false;
  }
	
	bool allRequiredParsed() {
    for (auto o : options) {
      if (o->getRequired() && !o->getParsed()) return false;
    }
		return true;
	}

  bool load(std::string const& filename) {
    std::ifstream ifs(filename);
    if (!ifs.good()) { return false; }
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
		return true;
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
