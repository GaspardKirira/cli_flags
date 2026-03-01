#include <cli_flags/cli_flags.hpp>

#include <cassert>
#include <vector>

static void test_long_and_short()
{
  cli_flags::app app;
  app.name("tool")
      .add_flag({"port", 'p', cli_flags::value_kind::string, "Server port", "8080"})
      .add_flag({"verbose", 'v', cli_flags::value_kind::boolean, "Verbose logs", "false"});

  std::vector<std::string> argv = {"tool", "--port", "9000", "-v", "file.txt"};
  auto out = app.parse(argv);

  assert(out.get("port") == "9000");
  assert(out.get_bool("verbose") == true);
  assert(out.positionals.size() == 1);
  assert(out.positionals[0] == "file.txt");
}

static void test_equals_and_negation()
{
  cli_flags::app app;
  app.add_flag({"color", 'c', cli_flags::value_kind::boolean, "Color output", "true"});

  std::vector<std::string> argv = {"tool", "--no-color"};
  auto out = app.parse(argv);

  assert(out.get_bool("color", true) == false);

  std::vector<std::string> argv2 = {"tool", "--color=false"};
  auto out2 = app.parse(argv2);

  assert(out2.get_bool("color", true) == false);
}

static void test_short_bundle()
{
  cli_flags::app app;
  app.add_flag({"alpha", 'a', cli_flags::value_kind::boolean, "", "false"})
      .add_flag({"beta", 'b', cli_flags::value_kind::boolean, "", "false"})
      .add_flag({"gamma", 'c', cli_flags::value_kind::boolean, "", "false"});

  std::vector<std::string> argv = {"tool", "-abc"};
  auto out = app.parse(argv);

  assert(out.get_bool("alpha") == true);
  assert(out.get_bool("beta") == true);
  assert(out.get_bool("gamma") == true);
}

static void test_subcommands_and_double_dash()
{
  cli_flags::app app;
  app.add_flag({"release", 'r', cli_flags::value_kind::boolean, "", "false"});

  std::vector<std::string> argv = {"tool", "build", "--release", "--", "--not-a-flag", "x"};
  auto out = app.parse(argv);

  assert(out.commands.size() == 1);
  assert(out.commands[0] == "build");
  assert(out.get_bool("release") == true);

  assert(out.positionals.size() == 2);
  assert(out.positionals[0] == "--not-a-flag");
  assert(out.positionals[1] == "x");
}

static void test_help_generation()
{
  cli_flags::app app;
  app.name("tool").about("Example tool").add_flag({"port", 'p', cli_flags::value_kind::string, "Server port", "8080"});

  std::vector<std::string> argv = {"tool", "--help"};
  auto out = app.parse(argv);

  assert(out.has("help_text"));
  assert(!out.get("help_text").empty());
}

int main()
{
  test_long_and_short();
  test_equals_and_negation();
  test_short_bundle();
  test_subcommands_and_double_dash();
  test_help_generation();
  return 0;
}
