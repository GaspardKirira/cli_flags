#include <cli_flags/cli_flags.hpp>

#include <iostream>

int main(int argc, char **argv)
{
  cli_flags::app app;
  app.name("vix")
      .about("Subcommands example")
      .add_command("build", "Build the project")
      .add_command("run", "Run the project")
      .add_command("test", "Run tests")
      .add_flag({"release", 'r', cli_flags::value_kind::boolean, "Release mode", "false"})
      .add_flag({"jobs", 'j', cli_flags::value_kind::string, "Parallel jobs", "8"});

  auto out = app.parse(argc, argv);

  if (out.has("help_text"))
  {
    std::cout << out.get("help_text") << "\n";
    return 0;
  }

  if (out.commands.empty())
  {
    std::cout << "No command provided. Use --help.\n";
    return 1;
  }

  const std::string cmd = out.commands[0];

  if (cmd == "build")
  {
    std::cout << "build: release=" << (out.get_bool("release") ? "true" : "false")
              << " jobs=" << out.get("jobs") << "\n";
    return 0;
  }

  if (cmd == "run")
  {
    std::cout << "run: args=";
    for (const auto &a : out.positionals)
      std::cout << a << " ";
    std::cout << "\n";
    return 0;
  }

  std::cout << "Unknown command: " << cmd << "\n";
  return 1;
}
