#include <cli_flags/cli_flags.hpp>

#include <iostream>
#include <vector>

int main()
{
  cli_flags::app app;
  app.name("mini")
      .about("minimist style parsing")
      .add_flag({"count", 'n', cli_flags::value_kind::string, "Count", "0"})
      .add_flag({"verbose", 'v', cli_flags::value_kind::boolean, "Verbose", "false"})
      .add_flag({"color", 'c', cli_flags::value_kind::boolean, "Color", "true"});

  std::vector<std::string> argv = {
      "mini",
      "-vc",
      "--count=12",
      "--no-color",
      "--",
      "--not-a-flag",
      "file.txt"};

  auto out = app.parse(argv);

  std::cout << "verbose=" << (out.get_bool("verbose") ? "true" : "false") << "\n";
  std::cout << "color=" << (out.get_bool("color") ? "true" : "false") << "\n";
  std::cout << "count=" << out.get("count") << "\n";

  std::cout << "positionals:";
  for (const auto &p : out.positionals)
    std::cout << " " << p;
  std::cout << "\n";

  return 0;
}
