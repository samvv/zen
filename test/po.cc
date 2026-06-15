
#include "gtest/gtest.h"

#include "zen/po.hpp"

auto prog = zen::po::program("git", "A fake Git CLI tool")
  .arg(zen::po::arg<bool>("bare")
      .flag("bare")
      .action(zen::po::arg_action::set_true))
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
              .action(zen::po::arg_action::set_true)
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
  auto files = *match.get<std::vector<std::any>>("foo");
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

TEST(POTest, SetTrueFlagMissing) {
  auto prog = zen::po::program("test")
    .arg(zen::po::arg<bool>("foobar").flag().action(zen::po::arg_action::set_true));
  auto match = prog
    .parse_args({})
    .unwrap();
  ASSERT_EQ(match.count(), 1);
  ASSERT_TRUE(match.has("foobar"));
  auto foobar = match.get<bool>("foobar");
  ASSERT_TRUE(foobar.has_value());
  ASSERT_EQ(*foobar, false);
}

TEST(POTest, SetTrueFlagPresent) {
  auto prog = zen::po::program("test")
    .arg(zen::po::arg<bool>("foobar").flag().action(zen::po::arg_action::set_true));
  auto match = prog
    .parse_args({ "--foobar" })
    .unwrap();
  ASSERT_EQ(match.count(), 1);
  ASSERT_TRUE(match.has("foobar"));
  auto foobar = match.get<bool>("foobar");
  ASSERT_TRUE(foobar.has_value());
  ASSERT_EQ(*foobar, true);
}

TEST(POTest, SetFalseFlagMissing) {
  auto prog = zen::po::program("test")
    .arg(zen::po::arg<bool>("foobar").flag().action(zen::po::arg_action::set_false));
  auto match = prog
    .parse_args({ })
    .unwrap();
  ASSERT_EQ(match.count(), 1);
  ASSERT_TRUE(match.has("foobar"));
  auto foobar = match.get<bool>("foobar");
  ASSERT_TRUE(foobar.has_value());
  ASSERT_EQ(*foobar, true);
}

TEST(POTest, SetFalseFlagPresent) {
  auto prog = zen::po::program("test")
    .arg(zen::po::arg<bool>("foobar").flag().action(zen::po::arg_action::set_false));
  auto match = prog
    .parse_args({ "--foobar" })
    .unwrap();
  ASSERT_EQ(match.count(), 1);
  ASSERT_TRUE(match.has("foobar"));
  auto foobar = match.get<bool>("foobar");
  ASSERT_TRUE(foobar.has_value());
  ASSERT_EQ(*foobar, false);
}

TEST(POTest, AssignsToRightmostArgOnOverlap) {
  auto prog = zen::po::program("test")
    .arg(zen::po::arg("bla").flag())
    .subcommand(zen::po::command("foo")
      .arg(zen::po::arg("bla").flag())
      .subcommand(zen::po::command("bar")
        .arg(zen::po::arg("bla").flag())
      )
    );
  auto match = prog.parse_args({ "foo", "bar", "--bla=foobar" }).unwrap();
  ASSERT_TRUE(match.has_subcommand());
  auto [foo_name, foo_match] = match.subcommand();
  ASSERT_TRUE(foo_match.has_subcommand());
  auto [bar_name, bar_match] = foo_match.subcommand();
  ASSERT_EQ(bar_match.count(), 1);
  auto test = bar_match.get<std::string>("bla");
  ASSERT_EQ(*test, "foobar");
}

TEST(POTest, CanParseSubcommandsNoPositional) {
  auto prog = zen::po::program("test")
    .subcommand(zen::po::command("foo")
      .subcommand(zen::po::command("bar"))
    );
  auto match = prog.parse_args({ "foo", "bar" }).unwrap();
  ASSERT_TRUE(match.has_subcommand());
  auto [name, sub1] = match.subcommand();
  ASSERT_EQ(name, "foo");
  ASSERT_TRUE(sub1.has_subcommand());
  auto [name2, sub2] = sub1.subcommand();
  ASSERT_FALSE(sub2.has_subcommand());
  ASSERT_EQ(sub2.count(), 0);
}

TEST(POTest, CanParseSubcommandsPositionalMixed) {
  auto prog = zen::po::program("test")
    .subcommand(zen::po::command("foo")
      .arg(zen::po::arg("pos").required())
      .subcommand(zen::po::command("bar"))
    );
  auto match = prog.parse_args({ "foo", "blabla", "bar" }).unwrap();
  ASSERT_TRUE(match.has_subcommand());
  auto [name, sub1] = match.subcommand();
  ASSERT_EQ(name, "foo");
  ASSERT_EQ(sub1.count(), 1);
  ASSERT_TRUE(sub1.has("pos"));
  ASSERT_EQ(*sub1.get<std::string>("pos"), "blabla");
  ASSERT_TRUE(sub1.has_subcommand());
  auto [name2, sub2] = sub1.subcommand();
  ASSERT_EQ(sub2.count(), 0);
  ASSERT_FALSE(sub2.has_subcommand());
}

TEST(POTest, FailsOnExcessPositional) {
  auto res = prog.parse_args({ "remote", "get-url", "foobar", "baz" });
  ASSERT_TRUE(res.is_left());
  ASSERT_TRUE(res.left().is<zen::po::excess_positional_arg_error>());
}

TEST(POTest, ParsesToplevelFlagBeforeSubcommand) {

  auto match = prog
    .parse_args({ "remote", "--bare", "get-url", "foobar" })
    .unwrap();

  ASSERT_TRUE(match.has_subcommand());
  auto [name, remote] = match.subcommand();
  ASSERT_EQ(name, "remote");

  auto bare = match.get<bool>("bare");
  ASSERT_TRUE(bare.has_value());
  ASSERT_EQ(*bare, true);
}

TEST(POTest, ParsesToplevelFlagAfterSubcommand) {

  auto match = prog
    .parse_args({ "remote", "get-url", "foobar", "--bare" })
    .unwrap();

  ASSERT_TRUE(match.has_subcommand());
  auto [name, remote] = match.subcommand();
  ASSERT_EQ(name, "remote");

  auto bare = match.get<bool>("bare");
  ASSERT_TRUE(bare.has_value());
  ASSERT_EQ(*bare, true);
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

