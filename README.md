# cli_flags

Minimal CLI flags parser for modern C++.

`cli_flags` provides a small, deterministic command-line parser inspired by
tools like minimist and yargs.

It supports long flags, short flags, bundled flags, subcommands, and automatic help generation.

Header-only. Zero external dependencies.

## Download

https://vixcpp.com/registry/pkg/gaspardkirira/cli_flags

## Why cli_flags?

Command-line parsing appears everywhere:

- CLI tools
- Build systems
- Dev servers
- Package managers
- Compilers
- Deployment utilities

Manual parsing with raw `argv` often leads to:

- Repeated boilerplate
- Inconsistent flag semantics
- Hard-to-maintain code
- Missing help text
- Poor subcommand handling

This library provides:

- Long flags: `--port 8080`, `--port=8080`
- Boolean flags: `--verbose`, `--no-color`
- Short flags: `-p 8080`, `-v`
- Bundled short flags: `-abc`
- Subcommands: `tool build --release`
- `--` stop-parsing semantics
- Automatic help text generation
- Deterministic parsing behavior

No dynamic dispatch.
No macro system.
No external libraries.

Just explicit CLI primitives.

## Installation

### Using Vix Registry

```bash
vix add gaspardkirira/cli_flags
vix deps
```

### Manual

```bash
git clone https://github.com/GaspardKirira/cli_flags.git
```

Add the `include/` directory to your project.

## Dependency

Requires C++17 or newer.

No external dependencies.

## Quick Examples

### Basic Parsing

```cpp
#include <cli_flags/cli_flags.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    cli_flags::app app;
    app.name("tool")
       .add_flag({"port", 'p', cli_flags::value_kind::string, "Server port", "8080"})
       .add_flag({"verbose", 'v', cli_flags::value_kind::boolean, "Verbose logs", "false"});

    auto out = app.parse(argc, argv);

    std::cout << "port=" << out.get("port") << "\n";
    std::cout << "verbose=" << (out.get_bool("verbose") ? "true" : "false") << "\n";
}
```

### Subcommands

```cpp
#include <cli_flags/cli_flags.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    cli_flags::app app;
    app.add_command("build", "Build project")
       .add_flag({"release", 'r', cli_flags::value_kind::boolean, "Release mode", "false"});

    auto out = app.parse(argc, argv);

    if (!out.commands.empty() && out.commands[0] == "build")
    {
        std::cout << "Build: release="
                  << (out.get_bool("release") ? "true" : "false") << "\n";
    }
}
```

### Boolean Negation

```bash
tool --no-color
```

Automatically sets `color=false`.

### Stop Parsing

```bash
tool --flag value -- --not-a-flag file.txt
```

Everything after `--` becomes positional.

## API Overview

```cpp
cli_flags::app app;

app.name("tool");
app.about("Description");

app.add_flag({
    "port",     // long name
    'p',        // short name
    cli_flags::value_kind::string,
    "Server port",
    "8080"
});

app.add_command("build", "Build project");

auto parsed = app.parse(argc, argv);
```

## Parsed Result

```cpp
parsed.flags          // map<string,string>
parsed.positionals    // vector<string>
parsed.commands       // vector<string>

parsed.get("port");
parsed.get_bool("verbose");
parsed.has("help_text");
```

## Supported Syntax

| Syntax         | Behavior                 |
|--------------|--------------------------|
| `--name value` | string flag              |
| `--name=value` | string flag              |
| `--flag`       | boolean true             |
| `--no-flag`    | boolean false            |
| `-p value`     | short string flag        |
| `-v`           | short boolean flag       |
| `-abc`         | bundled boolean flags    |
| `--`           | stop parsing             |

Subcommands are consecutive non-flag tokens before flags.

Example:

```bash
tool build --release file.txt
```

- Commands: `["build"]`
- Flags: `release=true`
- Positionals: `["file.txt"]`

## Complexity

Let:

- N = number of arguments

| Operation | Time Complexity |
|----------|------------------|
| Parsing  | O(N)             |
| Lookup   | O(1) average     |

Memory usage is proportional to number of flags and positionals.

## Semantics

- Defaults applied before parsing.
- Later flags override earlier ones.
- Unknown flags are accepted as string flags.
- Boolean flags normalize values (`true/false`, `1/0`, `yes/no`).
- Help text auto-generated via `--help` or `-h`.

## Design Principles

- Minimal abstraction
- Deterministic parsing
- Explicit flag specification
- No reflection
- No global state
- Header-only simplicity

This library provides primitives only.

If you need:

- Nested command trees
- Typed conversion helpers
- Environment variable integration
- Advanced validation
- Interactive CLI prompts

Build them on top of this layer.

## Tests

```bash
vix build
vix tests
```

Tests verify:

- Long and short flags
- Boolean negation
- Bundled short flags
- Subcommand parsing
- `--` stop behavior
- Help generation

## License

MIT License
Copyright (c) Gaspard Kirira

