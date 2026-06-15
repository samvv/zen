
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

  using parser_t = std::function<result<std::any>(const std::string&)>;

  std::unordered_map<std::type_index, parser_t> parsers {
    { std::type_index(typeid(std::string)), [](auto x) { return right(x); }  },
    { std::type_index(typeid(std::string)), [](auto x) { return right(x.empty() || x == "0" ? false : true); }  },
  };

  using value_list = std::vector<std::string>;

  static arg_action get_action(const _arg_info& arg) {
    if (arg._action) {
      return *arg._action;
    }
    if (arg._max_count > 1) {
      return arg_action::append;
    }
    return arg_action::set;
  }

  argmap program::fresh_argmap(const command& cmd) {
    argmap out;
    for (auto& arg: cmd._args) {
      switch (get_action(arg)) {
        case arg_action::set_true:
          out.emplace(arg._name, false);
          break;
        case arg_action::set_false:
          out.emplace(arg._name, true);
          break;
        case arg_action::append:
        case arg_action::prepend:
          out.emplace(arg._name, value_list {});
          break;
        default:
          break;
      }
    }
    return out;
  }

  result<match> program::parse_args(std::vector<std::string_view> argv) {

    std::size_t i = 0; // index into argv
    std::vector<command*> command_stack { &_command };
    std::vector<argmap> mapping_stack { fresh_argmap(_command) };
    auto pos_arg_iter = command_stack.back()->_pos_args.begin();
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
          name = std::string(arg.substr(k, l));
        } else {
          name = std::string(arg.substr(k));
        }

        bool found = false;
        for (std::size_t i = 0; i < command_stack.size(); ++i) {
        // for (auto iter = command_stack.rbegin(); iter != command_stack.rend(); ++iter) {
          // auto& cmd = **iter;
          auto& cmd = *command_stack[i];
          auto iter = cmd._flags.find(name);
          if (iter == cmd._flags.end()) {
            continue;
          }
          auto& flag = iter->second;
          found = true;
          bool needs_value = !flag.is<bool>();
          auto& map = mapping_stack[i];
          if (!needs_value) {
            map.emplace(name, true);
            break;
          }
          std::string value_str;
          if (l != arg.npos) {
            value_str = arg.substr(l);
          } else {
            if (i == argv.size()) {
              return left(flag_value_missing_error(name));
            }
            value_str = arg[i++];
          }
          auto parser = parsers.find(flag._type);
          if (parser == parsers.end()) {
            return left(unsupported_type_error(name));
          }
          auto value = parser->second(value_str);
          map.emplace(name, value);
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

        // Ensure that positional arguments can still be accepted
        if (pos_arg_iter == command_stack.back()->_pos_args.end()) {
          if (added_fallback || command_stack.back()->_subcommands.empty()) {
            return left(excess_positional_arg_error(i, std::string(arg)));
          } else {
            return left(command_not_found_error(std::string(arg)));
          }
        }

        // Process as positional argument
        // TODO support types other than std::string
        auto value = std::string(arg);
        if (pos_arg_iter->is_repeat()) {
          auto vec= mapping_stack.back().find(pos_arg_iter->_name)->second;
          std::any_cast<value_list&>(vec).push_back(value);
        } else {
          mapping_stack.back().emplace(pos_arg_iter->_name, value);
        }
        ++pos_arg_count;
        if (pos_arg_count == pos_arg_iter->_max_count) {
          ++pos_arg_iter;
          pos_arg_count = 0;
        }

      }

next:;
    }

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

    if (!command_stack.empty() && command_stack.back()->_callback) {
      std::exit((*command_stack.back()->_callback)(out));
    }

    return right(out);
  }

}

ZEN_NAMESPACE_END
