#include <cli_flags/cli_flags.hpp>

#include <iostream>

int main(int argc, char **argv)
{
  cli_flags::app app;
  app.name("tool")
      .about("Help generation example")
      .add_command("init", "Initialize something")
      .add_command("serve", "Start server")
      .add_flag({"port", 'p', cli_flags::value_kind::string, "Server port", "8080"})
      .add_flag({"color", 'c', cli_flags::value_kind::boolean, "Enable colors", "true"});

  auto out = app.parse(argc, argv);

  if (out.has("help_text"))
  {
    std::cout << out.get("help_text") << "\n";
    return 0;
  }

  std::cout << "Try: --help\n";
  return 0;
}
