#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class Option
{
public:
  Option()
    : m_parsed(false), m_required(false)
  {}

  virtual ~Option()
  {}

  virtual std::string getDefault()  = 0;
  virtual std::string getMessage()  = 0;
  virtual std::string getName()     = 0;
  virtual void        setValue(std::string const& val) = 0;

  bool
  getParsed()
  {
    return m_parsed;
  }

  bool
  getRequired()
  {
    return m_required;
  }

  virtual void
  setParsed(bool parsed)
  {
    m_parsed = parsed;
  }

protected:
  bool m_parsed;
  bool m_required;
};

template<typename T>
class OptionImpl
  : public Option
{
public:
  OptionImpl(T& var, T default_val, std::string const& name,
             bool required, std::string const& msg)
    : m_var(var)
  {
    this->m_default_val = default_val;
    this->m_name = name;
    this->m_required = required;
    this->m_msg = msg;
    this->m_var = default_val;
  }

  virtual std::string
  getDefault()
  {
    std::stringstream ss;
    ss << m_default_val;
    return ss.str();
  }

  std::string
  getName()
  {
    return m_name;
  }

  std::string
  getMessage()
  {
    return m_msg;
  }

  virtual void
  setValue(std::string const& val)
  {
    std::stringstream ss(val);
    ss >> m_var;
  }

protected:
  T m_default_val;
  T& m_var;
  std::string m_name;
  std::string m_msg;
};

class OptionStringImpl
  : public OptionImpl<std::string>
{
public:
  OptionStringImpl(std::string& var, std::string const& default_val,
                   std::string const& name, bool required,
                   std::string const& msg)
    : OptionImpl(var, default_val, name, required, msg)
  {}

  std::string
  getDefault()
  {
    std::stringstream ss;
    ss << "\"" << m_default_val << "\"";
    return ss.str();
  }

  void
  setValue(std::string const& val)
  {
    m_var = val;
  }
};

template<typename T>
class MultiOptionImpl : public Option
{
public:
  MultiOptionImpl(std::vector<T>& var,
                  std::string const& default_vals,
                  std::string const& name, bool required,
                  std::string const& msg, char delim)
    : m_var(var)
  {
    this->m_default_vals = default_vals;
    this->m_name = name;
    this->m_required = required;
    this->m_msg = msg;
    this->m_delim = delim;
    setValue(default_vals);
  }

  std::string
  getDefault()
  {
    std::stringstream ss;
    ss << "\"";
    ss << m_default_vals;
    ss << "\"";
    return ss.str();
  }

  std::string
  getName()
  {
    return m_name;
  }

  std::string
  getMessage()
  {
    return m_msg;
  }

  virtual void
  setValue(std::string const& val)
  {
    m_var.clear();
    std::stringstream ss(val);
    T elem;
    for (std::string val_str; std::getline(ss, val_str, m_delim);)
    {
      std::stringstream st(val_str);
      st >> elem;
      m_var.push_back(elem);
    }
  }

protected:
  std::string m_default_vals;
  std::vector<T>& m_var;
  std::string m_name;
  std::string m_msg;
  char m_delim;
};

class MultiOptionStringImpl
  : public MultiOptionImpl<std::string>
{
public:
  MultiOptionStringImpl(std::vector<std::string>& var, std::string
                        const& default_vals, std::string const& name,
                        bool required, std::string const& msg,
                        char delim)
    : MultiOptionImpl(var, default_vals, name, required, msg, delim)
  {}

  void
  setValue(std::string const& val)
  {
    m_var.clear();
    std::stringstream ss(val);
    for (std::string val_str; std::getline(ss, val_str, m_delim);) {
      m_var.push_back(val_str);
    }
  }
};

class FlagImpl
  : public Option
{
public:
  FlagImpl(bool& flag, std::string const& name, std::string const& msg)
    : m_flag(flag), m_name(name), m_msg(msg)
  {
    m_flag = false;
  }

  std::string
  getDefault()
  {
    return "";
  }

  std::string
  getName()
  {
    return m_name;
  }

  std::string
  getMessage()
  {
    return m_msg;
  }

  void
  setParsed(bool parsed)
  {
    Option::setParsed(parsed);
    m_flag = parsed;
  }

  void
  setValue(std::string const& val)
  {}

protected:
  bool& m_flag;
  std::string m_name;
  std::string m_msg;
};

class Argh {
public:
  Argh(char delim = ',')
    : m_delim(delim)
  {}

  ~Argh()
  {
    for (auto o : m_options)
    {
      delete o;
    }
    m_options.clear();
  }

  void
  parse(int argc, char const* argv[])
  {
    for (int i = 0; i < argc; ++i)
    {
      for (auto o : m_options)
      {
        if (std::string(argv[i]) == o->getName())
        {
          o->setParsed(true);
          if (i + 1 < argc) {
            o->setValue(argv[i + 1]);
          }
        }
      }
    }
  }

  void
  parseEnv()
  {
    for (auto o : m_options)
    {
      auto str = getenv(o->getName().c_str());
      if (str) {
        o->setParsed(true);
        o->setValue(str);
      }
    }
  }

  template<typename T>
  void
  addOption(T& var, T const& default_val, std::string const& name,
            bool required = false, std::string const& msg = "")
  {
    m_options.push_back(new OptionImpl<T>(var, default_val, name,
                                          required, msg));
  }

  void
  addOption(std::string& var, std::string const& default_val,
            std::string const& name, bool required = false,
            std::string const& msg = "")
  {
    m_options.push_back(new OptionStringImpl(var, default_val, name,
                                             required, msg));
  }

  template<typename T>
  void
  addMultiOption(std::vector<T>& var, std::string const& default_vals,
                 std::string const& name, bool required = false,
                 std::string const& msg = "")
  {
    m_options.push_back(new MultiOptionImpl<T>(var, default_vals, name,
                                               required, msg, m_delim));
  }

  void
  addMultiOption(std::vector<std::string>& var,
                 std::string const& default_vals,
                 std::string const& name, bool required = false,
                 std::string const& msg = "")
  {
    m_options.push_back(new MultiOptionStringImpl(var, default_vals,
                                                  name, required, msg,
                                                  m_delim));
  }

  void
  addFlag(bool& flag, std::string const& name, std::string const& msg = "")
  {
    m_options.push_back(new FlagImpl(flag, name, msg));
  }

  std::string
  getUsage() {
    size_t name_space    = getLongestName()    + 1;
    size_t default_space = getLongestDefault() + 1;
    size_t msg_space     = getLongestMessage() + 1;

    std::stringstream ret;
    ret << std::left;
    for (auto o : m_options)
    {
      ret
        << std::setw(static_cast<int>(name_space))    << o->getName()
        << std::setw(static_cast<int>(default_space)) << o->getDefault()
        << std::setw(static_cast<int>(msg_space))     << o->getMessage()
        << (o->getRequired() ? "REQUIRED" : "NOT REQUIRED")
        << std::endl;
    }
    return ret.str();
  }

  bool
  isParsed(std::string const& name)
  {
    for (auto o : m_options)
    {
      if (name == o->getName() && o->getParsed())
      {
        return true;
      }
    }
    return false;
  }

  std::vector<std::string>
  missingRequired()
  {
    std::vector<std::string> missing;
    for (auto o : m_options)
    {
      if (o->getRequired() && !o->getParsed())
        missing.push_back(o->getName());
    }
    return missing;
  }

  bool
  load(std::string const& filename)
  {
    std::ifstream ifs(filename);
    if (!ifs.good())
    {
      return false;
    }
    int argc = 0;
    std::vector<std::string> argv_str;
    std::vector<char const*> argv;
    std::string arg;
    while (std::getline(ifs, arg))
    {
      argv_str.push_back(arg);
      ++argc;
    }
    for (int i = 0; i < argc; ++i)
    {
      argv.push_back(argv_str[i].c_str());
    }
    parse(argc, &*argv.begin());
    return true;
  }

protected:

  size_t
  getLongestName()
  {
    size_t ret = 0;
    for (auto o : m_options)
    {
      ret = std::max(ret, o->getName().length());
    }
    return ret;
  }

  size_t
  getLongestDefault()
  {
    size_t ret = 0;
    for (auto o : m_options)
    {
      ret = std::max(ret, o->getDefault().length());
    }
    return ret;
  }

  size_t
  getLongestMessage()
  {
    size_t ret = 0;
    for (auto o : m_options)
    {
      ret = std::max(ret, o->getMessage().length());
    }
    return ret;
  }

  std::vector<Option*> m_options;
  char m_delim;
};
