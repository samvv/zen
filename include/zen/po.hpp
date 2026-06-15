
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

namespace po {

  struct posarg {
    std::string _name;
    int _arity;
  };

  template<typename T>
  class flag {

    friend class command;

    std::string name;
    std::optional<std::string> description;
    std::size_t min_count = 1;
    std::size_t max_count = 1;

  public:

    flag(std::string name, std::optional<std::string> description = {}):
      name(name), description(description) {}

    flag& optional() {
      min_count = 0;
      return *this;
    }

    flag& required() {
      min_count = std::max(static_cast<std::size_t>(1), min_count);
      return *this;
    }

  };

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

    posarg expected;

    missing_pos_arg_error(posarg expected):
      expected(expected) {}

    void display(std::ostream& out) const {
      out << "a positional argument for " << expected._name << " was missing.";
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
    std::unordered_map<std::string, _flag_info> _flags;
    std::vector<command> _subcommands;
    std::vector<posarg> _pos_args;
    bool _is_fallback = false;
    std::optional<command_callback_t> _callback;

  public:

    inline command(std::string name, std::optional<std::string> description = {}):
      _name(name), _description(description) {}

    command& description(std::string description) {
      _description = description;
      return *this;
    }

    template<typename T>
    command& flag(flag<T> fl) {
      _flags.emplace(fl.name, _flag_info { fl.description, typeid(T), fl.min_count, fl.max_count });
      return *this;
    }

    command& action(command_callback_t callback) {
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

    command& pos_arg(std::string name, int arity = 1) {
      _pos_args.push_back(posarg(name, arity));
      return *this;
    }

  };

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

  class program {

    command _command;

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

    template<typename T>
    program& flag(flag<T> fl) {
      _command.flag(fl);
      return *this;
    }

    program& pos_arg(std::string name, int arity = 1) {
      _command.pos_arg(name, arity);
      return *this;
    }

    result<match> parse_args(std::vector<std::string_view> argv);

    result<match> parse_args(int argc, const char** argv);

  };

}

ZEN_NAMESPACE_END
