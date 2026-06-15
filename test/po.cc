
#include "gtest/gtest.h"

#include "zen/po.hpp"

auto prog = zen::po::program("git", "A fake Git CLI tool")
  .arg(zen::po::arg<bool>("bare")
      .flag("bare"))
  .subcommand(
    zen::po::command("remote", "Commands for remote management")
      .subcommand(
        zen::po::command("set-url")
          .arg(zen::po::arg("name").required())
          .arg(zen::po::arg("url").required()))
      .subcommand(
        zen::po::command("get-url")
          .arg(
            zen::po::arg<bool>("push", "Query push URLs rather than fetch URLs")
              .flag("push")
          )
          .arg(zen::po::arg("name").required()))
      .subcommand(
        zen::po::command("remove")
          .arg(zen::po::arg("name")))
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

TEST(PoTest, StoresEmptyVectorWhenNoPosArgs) {
  auto match = zen::po::program("test")
    .arg(zen::po::arg("foo").many())
    .parse_args({})
    .unwrap();
  ASSERT_TRUE(match.has("foo"));
  auto files = *match.get<std::vector<std::string>>("foo");
  ASSERT_EQ(files.size(), 0);
}

TEST(POTest, ReportsErrorWhenCommandNotFound) {
  auto res = prog.parse_args({ "foobar" });
  ASSERT_TRUE(res.is_left());
  auto& err = res.left();
  ASSERT_TRUE(err.is<zen::po::command_not_found_error>());
  auto real_err = err.as<zen::po::command_not_found_error>();
  ASSERT_EQ(real_err.actual, "foobar");
}

TEST(POTest, ConvertsFlagToBool) {
  auto match = prog
    .parse_args({ "--bare" })
    .unwrap();
  ASSERT_EQ(match.count(), 1);
  ASSERT_TRUE(match.has("bare"));
  auto bare = match.get<bool>("bare");
  ASSERT_TRUE(bare.has_value());
  ASSERT_EQ(*bare, true);
}

TEST(POTest, CanParseSubcommandsNoPositional) {
  auto match = prog
    .parse_args({ "remote", "get-url" })
    .unwrap();
  ASSERT_TRUE(match.has_subcommand());
  auto [name, remote] = match.subcommand();
  ASSERT_EQ(name, "remote");
  ASSERT_TRUE(remote.has_subcommand());
  auto [name2, geturl] = remote.subcommand();
  ASSERT_FALSE(geturl.has_subcommand());
  ASSERT_EQ(geturl.count(), 0);
}

TEST(POTest, FailsOnExcessPositional) {
  auto res = prog.parse_args({ "remote", "get-url", "foobar", "baz" });
  ASSERT_TRUE(res.is_left());
  ASSERT_TRUE(res.left().is<zen::po::excess_positional_arg_error>());
}

TEST(POTest, ParsesToplevelFlagBeforeSubcommand) {

  auto match = prog
    .parse_args({ "remote", "--bare", "set-url", "foobar" })
    .unwrap();

  ASSERT_TRUE(match.has_subcommand());
  auto [name, remote] = match.subcommand();
  ASSERT_EQ(name, "remote");

  ASSERT_TRUE(match.get<bool>("bare"));
}

TEST(POTest, ParsesToplevelFlagAfterSubcommand) {

  auto match = prog
    .parse_args({ "remote", "set-url", "foobar", "--bare" })
    .unwrap();

  ASSERT_TRUE(match.has_subcommand());
  auto [name, remote] = match.subcommand();
  ASSERT_EQ(name, "remote");

  ASSERT_TRUE(match.get<bool>("bare"));
}

TEST(POTest, CanParseSubcommandsEndingPositional)  {

  auto match = prog
    .parse_args({ "remote", "get-url", "foobar" })
    .unwrap();

  ASSERT_TRUE(match.has_subcommand());
  auto [name, remote] = match.subcommand();
  ASSERT_EQ(name, "remote");
  ASSERT_TRUE(remote.has_subcommand());
  auto [name2, geturl] = remote.subcommand();
  ASSERT_EQ(name2, "get-url");
  ASSERT_FALSE(geturl.has_subcommand());
  ASSERT_EQ(geturl.count(), 1);
  ASSERT_EQ(geturl.get<std::string>("name"), "foobar");
}

