
#include "zen/config.hpp"
#include "zen/either.hpp"
#include "zen/po.hpp"

ZEN_NAMESPACE_START

namespace po {

  result<match> program::parse_args(int argc, const char** argv) {
    std::vector<std::string_view> args;
    auto argv_end = argv+argc;
    ZEN_ASSERT(argv_end != argv);
    for (auto ptr = argv+1; ptr != argv_end; ++ptr) {
      args.push_back(*ptr);
    }
    return parse_args(args);
  }

  using parse_fn_t = std::function<result<std::any>(const std::string_view&)>;

  template<typename T>
  std::pair<std::type_index, parse_fn_t> make_handler(parse_fn_t parse) {
    return {
    std::type_index(typeid(T)),
    parse,
    };
  }

  static result<std::string> parse_string(const std::string_view& text) {
    return right(text);
  }

  static result<bool> parse_bool(const std::string_view& text) {
    // FIXME only allow '0' and '1'?
    return right(text.empty() || text == "0" ? false : true);
  }

  static result<int> parse_int(const std::string_view& text) {
    if (text.empty()) {
      return left(invalid_number_format_error { std::string(text), text.data() });
    }
    char* endptr = nullptr;
    errno = 0;
    long l = std::strtol(text.data(), &endptr, 10);
    if (*endptr != '\0') {
      return left(invalid_number_format_error { std::string(text), endptr });
    }
    if ((errno == ERANGE && l == std::numeric_limits<long>::max()) || l > std::numeric_limits<int>::max()) {
      return left(number_too_large_error { l, std::numeric_limits<int>::max() });
    }
    if ((errno == ERANGE && l == std::numeric_limits<long>::min()) || l < std::numeric_limits<int>::min()) {
        return left(number_too_small_error { l, std::numeric_limits<int>::min() });
    }
    return right(l);
  }

  std::unordered_map<std::type_index, parse_fn_t> type_descs {
    make_handler<std::string>(parse_string),
    make_handler<bool>(parse_bool),
    make_handler<int>(parse_int),
  };

  argmap program::fresh_argmap(const command& cmd) {
    argmap out;
    for (auto& arg: cmd._args) {
      switch (*arg._action) {
        case arg_action::append:
        case arg_action::prepend:
          out.emplace(arg._name, std::vector<std::any> {});
          break;
        default:
          break;
      }
    }
    return out;
  }

  result<std::any> program::parse_value(const std::string_view& text, const _arg_info& arg) {
    auto parser = type_descs.find(arg._type);
    if (parser == type_descs.end()) {
      return left(unsupported_type_error(arg._name));
    }
    return parser->second(std::string(text));
  }

  result<match> program::parse_args(std::vector<std::string_view> argv) {

    std::size_t i = 0; // index into argv
    std::vector<command*> command_stack { &_command };
    std::vector<argmap> mapping_stack { fresh_argmap(_command) };
    auto pos_arg_iter = _command._pos_args.begin();
    std::size_t pos_arg_count = 0; // Counts the amount of positional arguments.

#define BOLT_PUSH_CMD(cmd) \
  if (pos_arg_iter != command_stack.back()->_pos_args.end()) { \
    return left(missing_pos_arg_error(*pos_arg_iter)); \
  } \
  command_stack.push_back(&cmd); \
  pos_arg_iter = cmd._pos_args.begin(); \
  mapping_stack.push_back(fresh_argmap(cmd)); \

    for (; i < argv.size(); ) {

      auto arg = argv[i++];

      if (arg.size() == 0) {

        return left(invalid_argument_error(arg));

      } else if (arg[0] == '-') {

        // Process argument as a command-line flag

        std::size_t k = 1;
        if (arg.size() >= 2 && arg[1] == '-') {
          ++k;
        }
        std::string name;
        auto l = arg.find('=', k);
        if (l != arg.npos) {
          name = std::string(arg.substr(k, l-k));
        } else {
          name = std::string(arg.substr(k));
        }

        bool found = false;
        for (std::size_t j = command_stack.size(); j-- > 0; ) {

          auto& cmd = *command_stack[j];
          auto& map = mapping_stack[j];

          auto found_arg_desc = cmd._flags.find(name);
          if (found_arg_desc == cmd._flags.end()) {
            continue;
          }
          auto& flag = found_arg_desc->second;
          found = true;

          switch (*flag._action) {

          case arg_action::set_true:
            map.emplace(flag._name, true);
            goto next;

          case arg_action::set_false:
            map.emplace(flag._name, false);
            goto next;

          default:

            // Get the text that was given to the flag
            std::string value_str;
            if (l != arg.npos) {
              value_str = arg.substr(l+1);
            } else {
              if (i == argv.size()) {
                return left(flag_value_missing_error(name));
              }
              value_str = arg[i++];
            }

            // Parse the text into a value of the type the flag expects
            auto result = parse_value(value_str, flag);
            ZEN_TRY(result);

            // Attempt to assign the value to this flag
            if (map.count(name)) {
              return left(argument_already_specified_error {
                  name,
              });
            }
            map.emplace(name, result.right());
          }
          break;
        }
        if (!found) {
          return left(unrecognised_flag_error(name));
        }

      } else {

        // Match a command from the list of subcommands
        for (auto& cmd: command_stack.back()->_subcommands) {
          if (cmd._name == arg) {
            BOLT_PUSH_CMD(cmd);
            goto next;
          }
        }

        // Add a fallback command if no command matched
        bool added_fallback = false;
        for (;;) {
          bool changed = false;
          for (auto cmd: command_stack.back()->_subcommands) {
            if (cmd._is_fallback) {
              BOLT_PUSH_CMD(cmd);
              changed = true;
              added_fallback = true;
              break;
            }
          }
          if (!changed) {
            break;
          }
        }

        const auto& cmd = *command_stack.back();
        auto& map = mapping_stack.back();

        // Ensure that positional arguments can still be accepted
        if (pos_arg_iter == cmd._pos_args.end()) {
          if (added_fallback || cmd._subcommands.empty()) {
            return left(excess_positional_arg_error(i, std::string(arg)));
          } else {
            return left(command_not_found_error(std::string(arg)));
          }
        }

        // Process as positional argument
        auto result = parse_value(arg, *pos_arg_iter);
        ZEN_TRY(result);
        switch (*pos_arg_iter->_action) {
          case arg_action::append:
            {
              auto& v = std::any_cast<std::vector<std::any>&>(map.find(pos_arg_iter->_name)->second);
              v.push_back(*result);
              break;
            }
          case arg_action::prepend:
            {
              auto& v = std::any_cast<std::vector<std::any>&>(map.find(pos_arg_iter->_name)->second);
              v.insert(v.begin(), *result);
              break;
            }
          case arg_action::set:
            if (map.count(pos_arg_iter->_name)) {
              return left(argument_already_specified_error {
                  pos_arg_iter->_name,
              });
            }
            map.emplace(pos_arg_iter->_name, *result);
            break;
          default:
            ZEN_UNREACHABLE
        }

        // Go to the next positional argument descriptor
        ++pos_arg_count;
        if (pos_arg_count == pos_arg_iter->_max_count) {
          ++pos_arg_iter;
          pos_arg_count = 0;
        }

      }

next:;
    }

    // Assign defaults and check for missing arguments
    for (std::size_t i = 0; i < command_stack.size(); ++i) {
      const auto& cmd = *command_stack[i];
      auto& map = mapping_stack[i];

      for (const auto& arg: cmd._args) {
        switch (*arg._action) {
          case arg_action::set:
            if (arg.is_required() && !map.count(arg._name)) {
              return left(argument_missing_error {
                  cmd._name,
                  arg._name,
              });
            }
            break;
          case arg_action::set_true:
            map.emplace(arg._name, false);
            break;
          case arg_action::set_false:
            map.emplace(arg._name, true);
            break;
          default:
            break;
        }
      }
    }

    // Construct the nested structure of the outgoing match object
    std::optional<std::pair<std::string, std::unique_ptr<match>>> sub;
    for (std::size_t k = command_stack.size(); k-- > 1; ) {
      sub = std::make_pair(
        command_stack[k]->_name,
        std::make_unique<match>(
          *command_stack[k],
          mapping_stack[k],
          std::move(sub)
        )
      );
    }

    match out = match {
      *command_stack[0],
      mapping_stack[0],
      std::move(sub)
    };

    if (command_stack.back()->_callback) {
      std::exit((*command_stack.back()->_callback)(out));
    }

    return right(out);
  }

}

ZEN_NAMESPACE_END
