
#pragma once

#include <algorithm>
#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "zen/config.hpp"
#include "zen/either.hpp"

ZEN_NAMESPACE_START

// TODO should be placed somewhere else
inline bool starts_with(const std::string_view& str, const std::string_view& needle) {
  auto it1 = str.begin();
  auto it2 = needle.begin();
  for (;;) {
    if (it2 == needle.end()) {
      return true;
    }
    if (it1 == str.end() || *it1 != *it2) {
      return false;
    }
    ++it1;
    ++it2;
  }
}

namespace po {

  enum class arg_action {
    set,
    set_true,
    set_false,
    append,
    prepend,
    count,
  };

  /**
   * @internal
   */
  struct _arg_info {

    friend class command;

    std::string _name;
    const std::type_info& _type;
    std::optional<std::string> _description;
    std::optional<arg_action> _action;

    std::vector<std::string> _flag_names;
    std::size_t _min_count = 0;
    std::size_t _max_count = 1;
    std::optional<std::any> _default_value;

    std::string name() const {
      return _name;
    }

    template<typename T2>
    bool is() {
      return typeid(T2) == _type;
    }

    bool is_repeat() const {
      return _max_count > 1;
    }

    bool is_positional() const {
      return _flag_names.empty();
    }

    _arg_info& min(std::size_t n) {
      _min_count = n;
      return *this;
    }

    _arg_info& max(std::size_t n) {
      _max_count = n;
      return *this;
    }

    _arg_info& some() {
      _min_count = 1;
      _max_count = ZEN_AUTO_SIZE;
      return *this;
    }

    _arg_info& many() {
      _min_count = 0;
      _max_count = ZEN_AUTO_SIZE;
      return *this;
    }

    _arg_info& optional() {
      _min_count = 0;
      return *this;
    }

    _arg_info& required() {
      _min_count = std::max(static_cast<std::size_t>(1), _min_count);
      return *this;
    }

    _arg_info& flag() {
      _flag_names.push_back(_name);
      return *this;
    }

    _arg_info& flag(char ch) {
      _flag_names.push_back(std::string { 1, ch });
      return *this;
    }

    _arg_info& flag(std::string str) {
      _flag_names.push_back(str);
      return *this;
    }

    _arg_info& default_value(std::any value) {
      _default_value = value;
      return *this;
    }

    _arg_info& action(arg_action action) {
      _action = action;
      return *this;
    }

  };

  template<typename T = std::string>
  _arg_info arg(std::string name, std::optional<std::string> description = {}) {
    return _arg_info {
      name,
      typeid(T),
      description,
    };
  }

  struct unrecognised_flag_error {

    std::string flag_name;

    void display(std::ostream& out) const {
      out << "the flag '" << flag_name << "' was not recognised by any of the (sub)commands.";
    }

  };

  struct unsupported_type_error {

    std::string flag_name;

    void display(std::ostream& out) const {
      out << "trying to store a flag value in a type that cannot be parsed."; 
    }

  };

  struct flag_value_missing_error {

    std::string flag_name;

    void display(std::ostream& out) const {
      out << "no value provided for flag '" << flag_name << "'.";
    }

  };

  struct excess_positional_arg_error {

    std::size_t i;
    std::string arg;

    void display(std::ostream& out) const {
      out << "excess positional argument '" << arg << "' found.";
    }

  };

  struct command_not_found_error {
 
    std::string actual;

    void display(std::ostream& out) const {
      out << "the command '" << actual << "' was not found.";
    }

  };

  struct invalid_argument_error {

    std::string_view actual;

    invalid_argument_error(std::string_view actual):
      actual(actual) {}

    void display(std::ostream& out) const {
      out << "the argument '" << actual << "' could not be parsed.";
    }

  };

  struct missing_pos_arg_error {

    _arg_info expected;

    missing_pos_arg_error(_arg_info expected):
      expected(expected) {}

    void display(std::ostream& out) const {
      out << "a positional argument for " << expected.name() << " was missing.";
    }

  };

  class error {

    using storage_t = std::variant<
      invalid_argument_error,
      missing_pos_arg_error,
      command_not_found_error,
      excess_positional_arg_error,
      flag_value_missing_error,
      unsupported_type_error,
      unrecognised_flag_error
    >;

    storage_t storage;

  public:

    template<typename T>
    error(T inner):
      storage(inner) {}

    void display(std::ostream& out) const {
      // TODO can probably do some metaprogramming to automate this
      switch (storage.index()) {
        case 0:
          std::get<0>(storage).display(out);
          break;
        case 1:
          std::get<1>(storage).display(out);
          break;
        case 2:
          std::get<2>(storage).display(out);
          break;
        case 3:
          std::get<3>(storage).display(out);
          break;
        case 4:
          std::get<4>(storage).display(out);
          break;
        case 5:
          std::get<5>(storage).display(out);
          break;
        case 6:
          std::get<6>(storage).display(out);
          break;
      }
    }

    template<typename T>
    bool is() const {
      return std::holds_alternative<T>(storage);
    }

    template<typename T>
    T& as() {
      return std::get<T>(storage);
    }

    template<typename T>
    const T& as() const {
      return std::get<T>(storage);
    }

  };

  template<typename T>
  using result = either<error, T>;

  class command;

  /**
   * @internal
   */
  using argmap = std::unordered_map<std::string, std::any>;

  struct match {

    friend class program;

    /**
     * @internal
     */
    command& cmd;

    /**
     * @internal
     */
    argmap values;

    /**
     * @internal
     */
    std::optional<std::pair<std::string, std::unique_ptr<match>>> submatch;

    /// Count how many different arguments are registered in this command.
    ///
    /// Repeated arguments are counted as one argument.
    ///
    /// This only counts arguemnts in the current command and NOT in any subcommands.
    ///
    bool count() const {
      return values.size();
    }

    /// Determine is the given argument received a value during invocation.
    bool has(const std::string& name) const {
      return values.count(name);
    }

    template<typename T>
    std::optional<T> get(const std::string& name) const {
      auto match = values.find(name);
      if  (match == values.end()) {
        return {};
      }
      return std::any_cast<T>(match->second);
    }

    bool has_subcommand() const noexcept {
      return submatch.has_value();
    }

    std::pair<std::string, match&> subcommand() {
      ZEN_ASSERT(submatch);
      return { submatch->first, *submatch->second };
    }

  };

  using command_callback_t = std::function<int(const match&)>;

  static constexpr const int opt = -3;
  static constexpr const int some = -2;
  static constexpr const int many = -1;

  struct _flag_info {
    std::optional<std::string> description;
    std::type_index type;
    std::size_t min_count;
    std::size_t max_count;
  };

  class command {

    friend class program;

  protected:

    std::string _name;
    std::optional<std::string> _description;
    std::vector<_arg_info> _args;
    std::unordered_map<std::string, _arg_info> _flags;
    std::vector<_arg_info> _pos_args;
    std::vector<command> _subcommands;
    bool _is_fallback = false;
    std::optional<command_callback_t> _callback;

  public:

    inline command(std::string name, std::optional<std::string> description = {}):
      _name(name), _description(description) {}

    command& description(std::string description) {
      _description = description;
      return *this;
    }

    command& arg(_arg_info x) {
      _args.push_back(x);
      if (x.is_positional()) {
        _pos_args.push_back(x);
      } else {
        for (auto& name: x._flag_names) {
          _flags.emplace(x._name, x);
        }
      }
      return *this;
    }

    command& callback(command_callback_t callback) {
      _callback = callback;
      return *this;
    }

    command& fallback() {
      _is_fallback = true;
      return *this;
    }

    command& subcommand(command cmd) {
      _subcommands.push_back(cmd);
      return *this;
    }

  };

  class program {

    command _command;

    argmap fresh_argmap(const command& cmd);

  public:

    inline program(std::string name, std::optional<std::string> description = {}):
      _command(name, description) {}

    program& description(std::string description) {
      _command.description(description);
      return *this;
    }

    program& subcommand(command cmd) {
      _command.subcommand(cmd);
      return *this;
    }

    program& arg(_arg_info fl) {
      _command.arg(fl);
      return *this;
    }

    result<match> parse_args(std::vector<std::string_view> argv);

    result<match> parse_args(int argc, const char** argv);

  };

}

ZEN_NAMESPACE_END
