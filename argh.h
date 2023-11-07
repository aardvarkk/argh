#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

/// An abstract template class that represents an option.
///
/// \tparam CharT is the data type of the characters used for the
/// option name and description.
template<typename CharT = char>
class Option
{
  using string = std::basic_string<CharT>;
public:

  /// Constructs an empty option.
  Option()
    : m_parsed(false), m_required(false)
  {}

  virtual ~Option() = default;

  /// Get a text version of the option's default value.
  virtual string getDefault()  = 0;

  /// Get a message describing the option.
  virtual string getMessage()  = 0;

  /// Get the option's name.
  virtual string getName()     = 0;

  /// Set the option's value.
  ///
  /// \param val is the value to assign.
  virtual void setValue(string const& val) = 0;

  /// Was the option parsed, i.e., seen?
  ///
  /// \returns `true` if the option was parsed, `false` otherwise.
  bool
  getParsed()
  {
    return m_parsed;
  }

  /// Is the option required?
  ///
  /// \returns `true` if the option is required, `false` otherwise.
  bool
  getRequired()
  {
    return m_required;
  }

  /// Change the option's "was it parsed?"-status.
  ///
  /// \param parsed is the new status.
  virtual void
  setParsed(bool parsed)
  {
    m_parsed = parsed;
  }

protected:
  bool m_parsed;              //!< was the option parsed (i.e., seen)?
  bool m_required;            //!< is the option required?
};

/// A template class that represents an option with none or a single
/// value.
///
/// \tparam T is the data type of the option value.
///
/// \tparam CharT is the data type of the characters.
template<typename T,
         typename CharT = char>
class OptionImpl
  : public Option<CharT>
{
  using string = std::basic_string<CharT>;
  using stringstream = std::basic_stringstream<CharT>;
public:
  /// Constructor.
  ///
  /// \param var is a reference to the variable that receives the
  /// option's value.
  ///
  /// \param default_val is the option's default value.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is a description of the option.
  OptionImpl(T& var, T default_val, string const& name, bool required,
             string const& msg)
    : m_var(var)
  {
    // TODO: refactor into initializer list
    m_default_val = default_val;
    m_name = name;
    // MSVC 2022 requires m_required to be prefixed by this->
    this->m_required = required;
    m_msg = msg;
    m_var = default_val;
  }

  virtual string
  getDefault()
  {
    stringstream ss;
    ss << m_default_val;
    return ss.str();
  }

  string
  getName()
  {
    return m_name;
  }

  string
  getMessage()
  {
    return m_msg;
  }

  /// Converts the option's text value to the target type and writes
  /// it to the configured variable.
  ///
  /// \param val is the text value representing the option's value.
  virtual void
  setValue(string const& val)
  {
    if constexpr (std::is_same_v<T, string>)
    {
      // If the target is a string then we must not use the
      // stringstream (i.e., the "else" clause of this if-statement)
      // because that extracts only the text up to the first
      // whitespace.
      m_var = val;
    }
    else {
      stringstream ss(val);
      ss >> m_var;
    }
  }

protected:
  T m_default_val;              //!< the option's default value
  T& m_var; //!< a reference to the variable that receives the option's value
  string m_name;                //!< the option's name
  string m_msg;                 //!< the option's description
};

/// A template class that represents a text option.
///
/// \tparam CharT is the data type of the characters.
template<typename CharT = char>
class OptionStringImpl
  : public OptionImpl<std::basic_string<CharT>, CharT>
{
  using string = std::basic_string<CharT>;
  using stringstream = std::basic_stringstream<CharT>;
public:
  OptionStringImpl(string& var, string const& default_val, string const& name,
                   bool required, string const& msg)
    : OptionImpl(var, default_val, name, required, msg)
  {}

  string
  getDefault()
  {
    stringstream ss;
    // MSVC 2022 doesn't accept m_default_val without this-> before it
    // in the next statement
    ss << "\"" << this->m_default_val << "\"";
    return ss.str();
  }

  void
  setValue(string const& val)
  {
    // MSVC 2022 doesn't accept m_var without this-> before it in the
    // next statement
    this->m_var = val;
  }
};

/// A template class that represents an option with multiple values.
///
/// \tparam T is the data type of the option values.
///
/// \tparam CharT is the data type of the characters.
template<typename T,
         typename CharT = char>
class MultiOptionImpl
  : public Option<CharT>
{
  using string = std::basic_string<CharT>;
  using stringstream = std::basic_stringstream<CharT>;
public:

  /// Constructor.
  ///
  /// \param var is a reference to the vector where the converted
  /// option values get written.
  ///
  /// \param default_vals is the default value for the option values.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is the option's description.
  ///
  /// \param delim is the delimiter that separates the option values
  /// in the text.
  MultiOptionImpl(std::vector<T>& var, string const& default_vals,
                  string const& name, bool required,
                  string const& msg, CharT delim)
    : m_var(var)
  {
    // TODO: refactor into initializer list
    m_default_vals = default_vals;
    m_name = name;
    // MSVC 2022 requires m_required to be prefixed by this->
    this->m_required = required;
    m_msg = msg;
    m_delim = delim;
    setValue(default_vals);
  }

  string
  getDefault()
  {
    stringstream ss;
    ss << "\"";
    ss << m_default_vals;
    ss << "\"";
    return ss.str();
  }

  string
  getName()
  {
    return m_name;
  }

  string
  getMessage()
  {
    return m_msg;
  }

  /// Converts the option's text value to zero or more values of the
  /// target type and writes them to the configured vector.
  ///
  /// \param val is the text value representing the option's value or
  /// values.
  virtual void
  setValue(string const& val)
  {
    m_var.clear();
    stringstream ss(val);
    if constexpr (std::is_same_v<T, string>)
    {
      // If the target is a vector of strings then we must not use
      // stringstream st (i.e., the "else" clause of this
      // if-statement) because that extracts only the text up to the
      // first whitespace.
      for (string val_str; std::getline(ss, val_str, m_delim);)
      {
        m_var.push_back(val_str);
      }
    }
    else
    {
      T elem;
      for (string val_str; std::getline(ss, val_str, m_delim);)
      {
        stringstream st(val_str);
        st >> elem;
        m_var.push_back(elem);
      }
    }
  }

protected:
  string m_default_vals;  //!< the option's default value or values
  std::vector<T>& m_var;  //!< a reference to the vector that receives
                          //!< the values
  string m_name;          //!< the option's name
  string m_msg;           //!< the option's description
  CharT m_delim;          //!< the delimiter for the option values
};

/// A template class that represents an option with multiple text
/// values.
///
/// \tparam CharT is the data type of the characters.
template<typename CharT = char>
class MultiOptionStringImpl
  : public MultiOptionImpl<std::basic_string<CharT>, CharT>
{
  using string = std::basic_string<CharT>;
  using stringstream = std::basic_stringstream<CharT>;
public:
  /// Constructor.
  ///
  /// \param var is a reference to the vector where the converted
  /// option values get written.
  ///
  /// \param default_vals is the default value for the option values.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is the option's description.
  ///
  /// \param delim is the delimiter that separates the option values
  /// in the text.
  MultiOptionStringImpl(std::vector<string>& var,
                        string const& default_vals,
                        string const& name, bool required,
                        string const& msg, CharT delim)
    : MultiOptionImpl(var, default_vals, name, required, msg, delim)
  {}

  /// Splits the option's text value into zero or more individual text
  /// values and writes them to the configured vector.
  ///
  /// \param val is the text value representing the option's value or
  /// values.
  void
  setValue(string const& val)
  {
    // MSVC 2022 does not accept m_var and m_delim without this->
    // before them.
    this->m_var.clear();
    stringstream ss(val);
    for (string val_str; std::getline(ss, val_str, this->m_delim);)
    {
      this->m_var.push_back(val_str);
    }
  }
};

/// A template class that represents a flag, an option without a
/// value.
template<typename CharT = char>
class FlagImpl
  : public Option<CharT>
{
  using string = std::basic_string<CharT>;
public:
  /// Constructor
  ///
  /// \param flag is a reference to the boolean variable that gets set
  /// to `true` when the option is seen.
  ///
  /// \param name is the option's name.
  ///
  /// \param msg is the option's description.
  FlagImpl(bool& flag, string const& name, string const& msg)
    : m_flag(flag), m_name(name), m_msg(msg)
  {
    m_flag = false; // false by default, changes to true when the option is seen
  }

  string
  getDefault()
  {
    return {};
  }

  string
  getName()
  {
    return m_name;
  }

  string
  getMessage()
  {
    return m_msg;
  }

  /// Changes the "was it seen?" status of the option.
  ///
  /// \param parsed says whether the option was seen.
  void
  setParsed(bool parsed)
  {
    Option<CharT>::setParsed(parsed);
    m_flag = parsed;
  }

  /// Implementation required to placate the compiler.  It does
  /// nothing and is never called.
  void
  setValue(string const& val)
  {}

protected:
  bool& m_flag;     //!< a reference to the variable that receives the
                    //!< flag's status
  string m_name;                //!< the option's name
  string m_msg;                 //!< the option's description
};

/// A template class to parse command line options.
///
/// \tparam CharT is the data type of the characters.
template<typename CharT = char>
class Argh
{
  using string = std::basic_string<CharT>;
  using stringstream = std::basic_stringstream<CharT>;
public:
  /// Constructor.
  ///
  /// \param delim is the delimiter that separates the text
  /// representing multiple values.
  Argh(CharT delim = ',')
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

  /// Parse the command line arguments.
  ///
  /// \param argc is the count of command line arguments.
  ///
  /// \param argv points at the first of the command line arguments.
  void
  parse(int argc, CharT const* const argv[])
  {
    std::vector<bool> is_option(argc);
    for (int i = 0; i < argc; ++i)
    {
      for (auto o : m_options)
      {
        if (string(argv[i]) == o->getName())
        {
          o->setParsed(true);
          is_option[i] = true;
          if (i + 1 < argc)
          {
            o->setValue(argv[i + 1]);
            is_option[i + 1] = true;
          }
        }
      }
    }
    for (int i = 0; i < argc; ++i)
    {
      if (!is_option[i])
        m_remaining_args.push_back(argv[i]);
    }
  }

  /// Get a reference to a vector containing the command line
  /// arguments (from the argv passed to parse()) that remain when the
  /// recognized options are removed.  That vector is filled by
  /// parse() so remains empty until parse() is called.
  ///
  /// \returns the reference
  const std::vector<string>&
  getRemainingArguments() const
  {
    return m_remaining_args;
  }

  /// Seek the configured options in the process environment.  Each
  /// option's name is sought in the process environment.  If an
  /// environment variable has the name of an option then that
  /// option's value is set on the environment variable's value.
  void
  parseEnv()
  {
    for (auto o : m_options)
    {
      auto str = getenv(o->getName().c_str());
      if (str)
      {
        o->setParsed(true);
        o->setValue(str);
      }
    }
  }

  /// Define an option.
  ///
  /// \param var is a reference to the variable that receives the
  /// option's value.
  ///
  /// \param default_val is the text representation of the option's
  /// default value.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is the option's description.
  template<typename T>
  void
  addOption(T& var, T const& default_val, string const& name,
            bool required = false, string const& msg = {})
  {
    m_options.push_back(new OptionImpl<T, CharT>(var, default_val, name,
                                                 required, msg));
  }

  /// Define a text option.
  ///
  /// \param var is a reference to the variable that receives the
  /// option's value.
  ///
  /// \param default_val is the option's default value.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is the option's description.
  void
  addOption(string& var, string const& default_val,
            string const& name, bool required = false,
            string const& msg = {})
  {
    m_options.push_back(new OptionStringImpl(var, default_val, name,
                                             required, msg));
  }

  /// Define a multi-value option.
  ///
  /// \param var is a reference to the vector that receives the
  /// option's values.
  ///
  /// \param default_vals is the text representation of the option's
  /// default values.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is the option's description.
  template<typename T>
  void
  addMultiOption(std::vector<T>& var, string const& default_vals,
                 string const& name, bool required = false,
                 string const& msg = {})
  {
    m_options.push_back(new MultiOptionImpl<T>(var, default_vals, name,
                                               required, msg, m_delim));
  }

  /// Define a multi-value text option.
  ///
  /// \param var is a reference to the vector that receives the
  /// option's values.
  ///
  /// \param default_vals is the text representation of the option's
  /// default values.
  ///
  /// \param name is the option's name.
  ///
  /// \param required says whether the option is required.
  ///
  /// \param msg is the option's description.
  void
  addMultiOption(std::vector<string>& var,
                 string const& default_vals,
                 string const& name, bool required = false,
                 string const& msg = "")
  {
    m_options.push_back(new MultiOptionStringImpl(var, default_vals,
                                                  name, required, msg,
                                                  m_delim));
  }

  /// Define a flag, an option without a value.
  ///
  /// \param flag is a reference to the variable that receives the
  /// option's status.
  ///
  /// \param name is the option's name.
  ///
  /// \param msg is the option's description.
  void
  addFlag(bool& flag, string const& name, string const& msg = {})
  {
    m_options.push_back(new FlagImpl<CharT>(flag, name, msg));
  }

  /// Get text that describes the options.
  ///
  /// \returns the text.
  string
  getUsage()
  {
    size_t name_space    = getLongestName()    + 1;
    size_t default_space = getLongestDefault() + 1;
    size_t msg_space     = getLongestMessage() + 1;

    stringstream ret;
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

  /// Was the specified option seen?
  ///
  /// \param name is the name of the option to query.
  ///
  /// \returns `true` if the option was seen, `false` otherwise.
  bool
  isParsed(string const& name)
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

  /// Which required options were not seen?
  ///
  /// \returns a vector containing the names of the options that
  /// weren't seen.
  std::vector<string>
  missingRequired()
  {
    std::vector<string> missing;
    for (auto o : m_options)
    {
      if (o->getRequired() && !o->getParsed())
        missing.push_back(o->getName());
    }
    return missing;
  }

  /// Load the options from a file.
  ///
  /// \param filename is the name of the file to read.
  ///
  /// \returns `true` for success, `false` for failure.
  bool
  load(string const& filename)
  {
    std::ifstream ifs(filename);
    if (!ifs.good())
    {
      return false;
    }
    int argc = 0;
    std::vector<string> argv_str;
    std::vector<CharT const*> argv;
    string arg;
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

  /// Get the greatest length of any of the option names.
  ///
  /// \returns the greatest length, or 0 if there are no options.
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

  /// Get the greatest length of any of the options' default values
  /// (converted to text).
  ///
  /// \returns the greatest length, or 0 if there are no options.
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

  /// Get the greatest length of any of the options' descriptions.
  ///
  /// \returns the greatest length, or 0 if there are no options.
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

  std::vector<Option<CharT>*> m_options; //!< the options
  CharT m_delim;                         //!< the delimiter
  std::vector<string> m_remaining_args;  //!< non-option arguments
};
