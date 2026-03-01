#include <cli_flags/cli_flags.hpp>

#include <iostream>

int main(int argc, char **argv)
{
  cli_flags::app app;
  app.name("tool")
      .about("Minimal CLI flags parser example")
      .add_flag({"port", 'p', cli_flags::value_kind::string, "Server port", "8080"})
      .add_flag({"host", 'H', cli_flags::value_kind::string, "Server host", "127.0.0.1"})
      .add_flag({"verbose", 'v', cli_flags::value_kind::boolean, "Verbose logs", "false"});

  auto out = app.parse(argc, argv);

  if (out.has("help_text"))
  {
    std::cout << out.get("help_text") << "\n";
    return 0;
  }

  std::cout << "host=" << out.get("host") << "\n";
  std::cout << "port=" << out.get("port") << "\n";
  std::cout << "verbose=" << (out.get_bool("verbose") ? "true" : "false") << "\n";

  std::cout << "commands:";
  for (const auto &c : out.commands)
    std::cout << " " << c;
  std::cout << "\n";

  std::cout << "positionals:";
  for (const auto &p : out.positionals)
    std::cout << " " << p;
  std::cout << "\n";

  return 0;
}
