Zen C++ Libraries
=================

<img src="https://raw.githubusercontent.com/ZenLibraries/ZenLibraries/master/zen-logo.png" height="250" />

The Zen C++ libraries are a set of C++ headers and sources that augment the C++
standard library. The libraries try to fill missing pieces in the existing C++
ecosystem, based on ideas taken from Haskell and Rust.

## Examples

### Parsing program arguments

```cpp
auto prog = zen::po::program("git", "A fake Git CLI tool")
  .arg(zen::po::arg<bool>("bare", "Treat the repository as a bare repository")
      .flag()
      .action(zen::po::arg_action::set_true))
  .subcommand(
    zen::po::command("remote", "Commands for remote management")
      .subcommand(
        zen::po::command("set-url", "Change the URL of an existing remote")
          .arg(zen::po::arg("name", "The name of the remote").required())
          .arg(zen::po::arg("url", "The new URL to use").required())
      )
      .subcommand(
        zen::po::command("get-url")
          .arg(zen::po::arg<bool>("push", "Query push URLs rather than fetch URLs")
            .flag()
            .action(zen::po::arg_action::set_true))
          .arg(zen::po::arg("name", "The name of the remote").required())
      )
      .subcommand(
        zen::po::command("remove")
          .arg(zen::po::arg("name", "The name of the remote").required())
      )
    )
  .subcommand(
    zen::po::command("commit", "Record changes to the repository")
      .arg(zen::po::arg("message")
        .flag("message")
        .flag('m'))
  )
  .subcommand(
    zen::po::command("add", "Add files to the stage")
      .arg(zen::po::arg("files").some())
  );

auto match = prog
  .parse_args({ "remote", "get-url", "foobar" })
  .unwrap();
```

## In The Wild

Zen is currently being used in the following projects:

 - [The Bolt compiler][bolt]

Want your project here? Open up a pull request and we'll gladly accept it!

## Installation and Usage 

> [!CAUTION]
>
> These libraries are experimental. They require a modern C++ compiler that
> supports at least C++20. The API may break regularly with the release of new
> versions. If you plan to make use of them right now, expect to refactor your
> code regularly.
>
>  - _Not all control flow paths have been tested._ We simply don't have the
>    scale to add test cases for all possible ways in which this library may be
>    used. You are encouraged, however, to use this library and report any issues
>    you find in the issue tracker. The more people use this library, the more
>    stable it will get.
>  - _New updates might need a small refactor of your code._ For example, a
>    better `zen::either<L, R>` might require you to use strucutred binding
>    declarations instead of using the dereferce operator. Only use this library
>    if your team has enough bandwidth to keep in sync.
>
> Use this library only if you feel like these drawbacks are justified for the
> project you're working on.

We support Meson and CMake. Currently, the preferred method for using these
libraries is by downloading a recent tarball of the repository's source and
checking in the sources into your project's version control system.

**If you're building a library that uses Zen++ internally, it is highly
recommended to define a custom Zen++ namespace.** Doing so will avoid conflicts
with dependencies that use a different version of these libraries. For
instance, if your project has a namespace `mylib`, you would define the Zen
namespace as `mylib::zen`.

Meson is the recommended choice for setting up your project. Download a tarball
of this repository and drop the contents in `subprojects/zen`. Next, adjust the
following example to match your project setup.

**meson.build**
```meson
project('myproject', 'cxx')

zen_proj = subproject('zen', default_options: ['namespace=mylib::zen'])
zen_dep = zen_proj.get_variable('zen_dep')

library('mylib', dependencies: zen_dep)
```

CMake is also supported, although we really recommend using Meson. CMake does
not play nice with in-source subprojects because there's no elegant way to
emulate Meson's `default_options`. Until we have figured out how to do it
properly, you might want to use the following.

**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.10)

project(MyProject CXX)

# This will overwrite any value that might previously have been set.
set(ZEN_NAMESPACE mylib::zen CACHE INTERNAL "The Zen++ namespace this project will use" FORCE)

add_subdirectory(third_party/zen EXCLUDE_FROM_ALL)

add_library(mylib src/main.cc)

target_link_libraries(mylib zen)
```

## Documentation

Documentation will soon be available on [the official website][1]. For now, you
will have to consult the header files in `zen/` to learn more.
Alternatively, you can try to build the documentation locally using [Doxygen][2].

## License

This library is licensed under Apache 2.0 license. Briefly put, you are allowed
to use this library commercially as long as you give due credit to the authors
that put their time and energy in building this.

See [the LICENSE file][3] for more information.

[1]: https://samvv.github.io/zen/
[2]: http://www.doxygen.nl/
[3]: https://github.com/samvv/zen/blob/master/LICENSE
[bolt]: https://github.com/boltlang/bolt
