// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/basictypes.h"
#include "base/file_path.h"
#include "base/string_util.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CommandLineTest, CommandLineConstructor) {
#if defined(OS_WIN)
  CommandLine cl = CommandLine::FromString(
                     L"program --foo= -bAr  /Spaetzel=pierogi /Baz flim "
                     L"--other-switches=\"--dog=canine --cat=feline\" "
                     L"-spaetzle=Crepe   -=loosevalue  flan "
                     L"--input-translation=\"45\"--output-rotation "
                     L"-- -- --not-a-switch "
                     L"\"in the time of submarines...\"");
  EXPECT_FALSE(cl.command_line_string().empty());
#elif defined(OS_POSIX)
  const char* argv[] = {"program", "--foo=", "-bar",
                        "-spaetzel=pierogi", "-baz", "flim",
                        "--other-switches=--dog=canine --cat=feline",
                        "-spaetzle=Crepe", "-=loosevalue", "flan",
                        "--input-translation=45--output-rotation",
                        "--", "--", "--not-a-switch",
                        "in the time of submarines..."};
  CommandLine cl(arraysize(argv), argv);
#endif
  EXPECT_FALSE(cl.HasSwitch("cruller"));
  EXPECT_FALSE(cl.HasSwitch("flim"));
  EXPECT_FALSE(cl.HasSwitch("program"));
  EXPECT_FALSE(cl.HasSwitch("dog"));
  EXPECT_FALSE(cl.HasSwitch("cat"));
  EXPECT_FALSE(cl.HasSwitch("output-rotation"));
  EXPECT_FALSE(cl.HasSwitch("not-a-switch"));
  EXPECT_FALSE(cl.HasSwitch("--"));

  EXPECT_EQ(L"program", cl.program());

  EXPECT_TRUE(cl.HasSwitch("foo"));
  EXPECT_TRUE(cl.HasSwitch("bar"));
  EXPECT_TRUE(cl.HasSwitch("baz"));
  EXPECT_TRUE(cl.HasSwitch("spaetzle"));
#if defined(OS_WIN)
  EXPECT_TRUE(cl.HasSwitch("SPAETZLE"));
#endif
  EXPECT_TRUE(cl.HasSwitch("other-switches"));
  EXPECT_TRUE(cl.HasSwitch("input-translation"));

  EXPECT_EQ("Crepe", cl.GetSwitchValueASCII("spaetzle"));
  EXPECT_EQ("", cl.GetSwitchValueASCII("Foo"));
  EXPECT_EQ("", cl.GetSwitchValueASCII("bar"));
  EXPECT_EQ("", cl.GetSwitchValueASCII("cruller"));
  EXPECT_EQ("--dog=canine --cat=feline", cl.GetSwitchValueASCII(
      "other-switches"));
  EXPECT_EQ("45--output-rotation", cl.GetSwitchValueASCII("input-translation"));

  const std::vector<CommandLine::StringType>& args = cl.args();
  ASSERT_EQ(5U, args.size());

  std::vector<CommandLine::StringType>::const_iterator iter = args.begin();
  EXPECT_EQ(FILE_PATH_LITERAL("flim"), *iter);
  ++iter;
  EXPECT_EQ(FILE_PATH_LITERAL("flan"), *iter);
  ++iter;
  EXPECT_EQ(FILE_PATH_LITERAL("--"), *iter);
  ++iter;
  EXPECT_EQ(FILE_PATH_LITERAL("--not-a-switch"), *iter);
  ++iter;
  EXPECT_EQ(FILE_PATH_LITERAL("in the time of submarines..."), *iter);
  ++iter;
  EXPECT_TRUE(iter == args.end());
#if defined(OS_POSIX)
  const std::vector<std::string>& argvec = cl.argv();

  for (size_t i = 0; i < argvec.size(); i++) {
    EXPECT_EQ(0, argvec[i].compare(argv[i]));
  }
#endif
}

// Tests behavior with an empty input string.
TEST(CommandLineTest, EmptyString) {
#if defined(OS_WIN)
  CommandLine cl = CommandLine::FromString(L"");
  EXPECT_TRUE(cl.command_line_string().empty());
  EXPECT_TRUE(cl.program().empty());
#elif defined(OS_POSIX)
  CommandLine cl(0, NULL);
  EXPECT_TRUE(cl.argv().size() == 0);
#endif
  EXPECT_EQ(0U, cl.args().size());
}

// Test methods for appending switches to a command line.
TEST(CommandLineTest, AppendSwitches) {
  std::string switch1 = "switch1";
  std::string switch2 = "switch2";
  std::string value = "value";
  std::string switch3 = "switch3";
  std::string value3 = "a value with spaces";
  std::string switch4 = "switch4";
  std::string value4 = "\"a value with quotes\"";

  CommandLine cl(FilePath(FILE_PATH_LITERAL("Program")));

  cl.AppendSwitch(switch1);
  cl.AppendSwitchASCII(switch2, value);
  cl.AppendSwitchASCII(switch3, value3);
  cl.AppendSwitchASCII(switch4, value4);

  EXPECT_TRUE(cl.HasSwitch(switch1));
  EXPECT_TRUE(cl.HasSwitch(switch2));
  EXPECT_EQ(value, cl.GetSwitchValueASCII(switch2));
  EXPECT_TRUE(cl.HasSwitch(switch3));
  EXPECT_EQ(value3, cl.GetSwitchValueASCII(switch3));
  EXPECT_TRUE(cl.HasSwitch(switch4));
  EXPECT_EQ(value4, cl.GetSwitchValueASCII(switch4));
}
