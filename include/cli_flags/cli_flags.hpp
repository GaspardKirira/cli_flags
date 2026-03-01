/**
 * @file cli_flags.hpp
 * @brief Minimal CLI flags parser with subcommands and help (minimist/yargs style).
 *
 * Features:
 * - long flags: --port 8080, --port=8080, --no-color
 * - short flags: -p 8080, -v, -abc (bundled bool flags)
 * - positional args
 * - "--" stops parsing
 * - subcommands: app build --release
 * - auto help text generation
 *
 * Header-only. Zero external dependencies.
 *
 * Requirements: C++17+
 */

#ifndef CLI_FLAGS_CLI_FLAGS_HPP
#define CLI_FLAGS_CLI_FLAGS_HPP

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cli_flags
{
  enum class value_kind
  {
    boolean,
    string
  };

  struct flag_spec
  {
    std::string long_name;  // "port"
    char short_name = '\0'; // 'p'
    value_kind kind = value_kind::string;

    std::string description;
    std::string default_value; // for kind=string, or "true/false" for boolean
  };

  struct parsed
  {
    // For boolean: "true"/"false"
    // For string: actual value
    std::unordered_map<std::string, std::string> flags;

    std::vector<std::string> positionals;

    // subcommands in order, ex: ["build", "run"]
    std::vector<std::string> commands;

    bool has(const std::string &key) const
    {
      return flags.find(key) != flags.end();
    }

    std::string get(const std::string &key, const std::string &fallback = "") const
    {
      const auto it = flags.find(key);
      return (it == flags.end()) ? fallback : it->second;
    }

    bool get_bool(const std::string &key, bool fallback = false) const
    {
      const auto it = flags.find(key);
      if (it == flags.end())
        return fallback;

      const std::string &v = it->second;
      return v == "1" || v == "true" || v == "TRUE" || v == "yes" || v == "on";
    }
  };

  namespace detail
  {
    inline bool starts_with(const std::string &s, const char *pfx)
    {
      const std::size_t n = std::char_traits<char>::length(pfx);
      return s.size() >= n && s.compare(0, n, pfx) == 0;
    }

    inline bool is_flag_token(const std::string &s)
    {
      return starts_with(s, "-") && s != "-";
    }

    inline std::string strip_quotes(std::string s)
    {
      if (s.size() >= 2)
      {
        const char a = s.front();
        const char b = s.back();
        if ((a == '"' && b == '"') || (a == '\'' && b == '\''))
        {
          s = s.substr(1, s.size() - 2);
        }
      }
      return s;
    }

    inline std::string to_lower(std::string s)
    {
      for (char &c : s)
      {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      }
      return s;
    }

    inline std::string normalize_bool(std::string v)
    {
      v = to_lower(strip_quotes(std::move(v)));
      if (v == "1" || v == "true" || v == "yes" || v == "on")
        return "true";
      if (v == "0" || v == "false" || v == "no" || v == "off")
        return "false";
      // fallback: anything non-empty => true (minimist-like)
      return v.empty() ? "false" : "true";
    }

    struct spec_index
    {
      std::unordered_map<std::string, flag_spec> by_long;
      std::unordered_map<char, std::string> short_to_long;
    };

    inline spec_index build_index(const std::vector<flag_spec> &specs)
    {
      spec_index idx;
      for (const auto &s : specs)
      {
        if (!s.long_name.empty())
        {
          idx.by_long.emplace(s.long_name, s);
        }
        if (s.short_name != '\0')
        {
          idx.short_to_long.emplace(s.short_name, s.long_name);
        }
      }
      return idx;
    }

    inline value_kind kind_for(const spec_index &idx, const std::string &long_name)
    {
      const auto it = idx.by_long.find(long_name);
      if (it == idx.by_long.end())
        return value_kind::string;
      return it->second.kind;
    }

    inline std::string default_for(const spec_index &idx, const std::string &long_name)
    {
      const auto it = idx.by_long.find(long_name);
      if (it == idx.by_long.end())
        return "";
      return it->second.default_value;
    }

    inline bool is_known_long(const spec_index &idx, const std::string &long_name)
    {
      return idx.by_long.find(long_name) != idx.by_long.end();
    }

    inline std::string resolve_short(const spec_index &idx, char c)
    {
      const auto it = idx.short_to_long.find(c);
      if (it == idx.short_to_long.end())
      {
        // unknown short flag becomes itself as long name (single-letter key)
        return std::string(1, c);
      }
      return it->second;
    }
  } // namespace detail

  class app
  {
  public:
    app() = default;

    app &name(std::string n)
    {
      name_ = std::move(n);
      return *this;
    }

    app &about(std::string a)
    {
      about_ = std::move(a);
      return *this;
    }

    app &add_flag(flag_spec s)
    {
      specs_.push_back(std::move(s));
      return *this;
    }

    app &add_command(std::string cmd, std::string description = "")
    {
      commands_desc_.emplace_back(std::move(cmd), std::move(description));
      return *this;
    }

    parsed parse(int argc, char **argv) const
    {
      std::vector<std::string> args;
      args.reserve(static_cast<std::size_t>(argc));
      for (int i = 0; i < argc; ++i)
      {
        args.emplace_back(argv[i] ? argv[i] : "");
      }
      return parse(args);
    }

    parsed parse(const std::vector<std::string> &argv) const
    {
      if (argv.empty())
      {
        return parsed{};
      }

      const auto idx = detail::build_index(specs_);

      parsed out;
      apply_defaults(idx, out);

      // argv[0] is program name
      std::size_t i = 1;
      bool stop_flags = false;
      bool parsing_commands = true;

      while (i < argv.size())
      {
        const std::string tok = argv[i];

        if (!stop_flags && tok == "--")
        {
          stop_flags = true;
          ++i;
          continue;
        }

        // subcommand detection: first non-flag token becomes a command
        // and we keep consuming consecutive non-flag tokens as commands
        // until we see a flag or "--".
        if (parsing_commands && !stop_flags && !detail::is_flag_token(tok))
        {
          out.commands.push_back(tok);
          ++i;
          continue;
        }

        parsing_commands = false;

        if (!stop_flags && detail::starts_with(tok, "--"))
        {
          parse_long(idx, tok, argv, i, out);
          continue;
        }

        if (!stop_flags && detail::starts_with(tok, "-") && tok.size() >= 2 && tok != "-")
        {
          parse_short(idx, tok, argv, i, out);
          continue;
        }

        out.positionals.push_back(tok);
        ++i;
      }

      // Auto help flags
      if (out.get_bool("help", false))
      {
        out.flags["help_text"] = help_text();
      }

      return out;
    }

    std::string help_text() const
    {
      std::ostringstream os;

      const std::string prog = name_.empty() ? std::string("app") : name_;
      os << prog << "\n";
      if (!about_.empty())
        os << about_ << "\n";

      os << "\nUsage\n";
      os << "  " << prog << " [command...] [flags] [--] [args]\n";

      if (!commands_desc_.empty())
      {
        os << "\nCommands\n";
        for (const auto &c : commands_desc_)
        {
          os << "  " << c.first;
          if (!c.second.empty())
            os << "  " << c.second;
          os << "\n";
        }
      }

      os << "\nFlags\n";
      os << "  -h, --help  Show help\n";

      for (const auto &s : specs_)
      {
        os << "  ";
        bool wrote = false;

        if (s.short_name != '\0')
        {
          os << "-" << s.short_name;
          wrote = true;
        }

        if (!s.long_name.empty())
        {
          if (wrote)
            os << ", ";
          os << "--" << s.long_name;
        }

        if (s.kind == value_kind::string)
          os << " <value>";

        if (!s.description.empty())
          os << "  " << s.description;

        if (!s.default_value.empty())
          os << " (default: " << s.default_value << ")";

        os << "\n";
      }

      return os.str();
    }

  private:
    std::string name_{};
    std::string about_{};

    std::vector<flag_spec> specs_{};
    std::vector<std::pair<std::string, std::string>> commands_desc_{};

  private:
    static void apply_defaults(const detail::spec_index &idx, parsed &out)
    {
      // Apply default_value for known specs.
      for (const auto &kv : idx.by_long)
      {
        const auto &spec = kv.second;
        if (spec.default_value.empty())
          continue;

        if (spec.kind == value_kind::boolean)
        {
          out.flags[spec.long_name] = detail::normalize_bool(spec.default_value);
        }
        else
        {
          out.flags[spec.long_name] = spec.default_value;
        }
      }
    }

    static void set_bool(parsed &out, const std::string &key, bool v)
    {
      out.flags[key] = v ? "true" : "false";
    }

    static void set_value(parsed &out, const std::string &key, std::string v, value_kind kind)
    {
      if (kind == value_kind::boolean)
      {
        out.flags[key] = detail::normalize_bool(std::move(v));
      }
      else
      {
        out.flags[key] = detail::strip_quotes(std::move(v));
      }
    }

    static void parse_long(
        const detail::spec_index &idx,
        const std::string &tok,
        const std::vector<std::string> &argv,
        std::size_t &i,
        parsed &out)
    {
      // tok: --name or --name=value or --no-name
      std::string s = tok.substr(2);

      bool negated = false;
      if (detail::starts_with(s, "no-"))
      {
        negated = true;
        s = s.substr(3);
      }

      std::string name;
      std::string value;
      bool has_eq = false;

      const std::size_t eq = s.find('=');
      if (eq != std::string::npos)
      {
        name = s.substr(0, eq);
        value = s.substr(eq + 1);
        has_eq = true;
      }
      else
      {
        name = s;
      }

      // Built-in help: treat as boolean even if not declared in specs.
      if (name == "help")
      {
        if (negated)
        {
          set_bool(out, "help", false);
          ++i;
          return;
        }

        if (has_eq)
        {
          set_value(out, "help", value, value_kind::boolean);
          ++i;
          return;
        }

        set_bool(out, "help", true);
        ++i;
        return;
      }

      const value_kind kind = detail::kind_for(idx, name);

      if (kind == value_kind::boolean)
      {
        if (negated)
        {
          set_bool(out, name, false);
          ++i;
          return;
        }

        if (has_eq)
        {
          set_value(out, name, value, kind);
          ++i;
          return;
        }

        // no explicit value => true
        set_bool(out, name, true);
        ++i;
        return;
      }

      // string kind
      if (negated)
      {
        // --no-name on a string flag is treated as empty
        out.flags[name] = "";
        ++i;
        return;
      }

      if (has_eq)
      {
        set_value(out, name, value, kind);
        ++i;
        return;
      }

      // --name value
      if (i + 1 < argv.size() && !detail::is_flag_token(argv[i + 1]))
      {
        set_value(out, name, argv[i + 1], kind);
        i += 2;
        return;
      }

      // missing value => empty
      out.flags[name] = "";
      ++i;
    }

    static void parse_short(
        const detail::spec_index &idx,
        const std::string &tok,
        const std::vector<std::string> &argv,
        std::size_t &i,
        parsed &out)
    {
      // tok: -p or -abc
      const std::string bundle = tok.substr(1);

      // single short flag may consume a value: -p 8080 or -p=8080
      // bundled flags are bool only.
      if (bundle.size() == 1)
      {
        const char c = bundle[0];

        // built-in help alias: -h => help (even if not in specs)
        const std::string name = (c == 'h') ? std::string("help") : detail::resolve_short(idx, c);
        const value_kind kind = detail::kind_for(idx, name);

        // support -p=8080
        {
          const std::size_t eq = tok.find('=');
          if (eq != std::string::npos)
          {
            const std::string v = tok.substr(eq + 1);
            set_value(out, name, v, kind);
            ++i;
            return;
          }
        }

        if (kind == value_kind::boolean)
        {
          set_bool(out, name, true);
          ++i;
          return;
        }

        if (i + 1 < argv.size() && !detail::is_flag_token(argv[i + 1]))
        {
          set_value(out, name, argv[i + 1], kind);
          i += 2;
          return;
        }

        out.flags[name] = "";
        ++i;
        return;
      }

      // bundle: -abc => set a,b,c to true
      for (char c : bundle)
      {
        // built-in help alias inside bundles too: -vh => verbose + help
        const std::string name = (c == 'h') ? std::string("help") : detail::resolve_short(idx, c);
        set_bool(out, name, true);
      }
      ++i;
    }
  };

} // namespace cli_flags

#endif // CLI_FLAGS_CLI_FLAGS_HPP
