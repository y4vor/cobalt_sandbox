// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "starboard/string.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(SbStringCompareNoCaseNTest, SunnyDaySelf) {
  const char kString[] = "0123456789";
  EXPECT_EQ(0, SbStringCompareNoCaseN(kString, kString,
                                      SbStringGetLength(kString)));
  EXPECT_EQ(0, SbStringCompareNoCaseN("", "", 0));
}

TEST(SbStringCompareNoCaseNTest, SunnyDayEmptyLessThanNotEmpty) {
  const char kString[] = "0123456789";
  EXPECT_GT(0, SbStringCompareNoCaseN("", kString,
                                      SbStringGetLength(kString)));
}

TEST(SbStringCompareNoCaseNTest, SunnyDayEmptyZeroNEqual) {
  const char kString[] = "0123456789";
  EXPECT_EQ(0, SbStringCompareNoCaseN("", kString, 0));
}

TEST(SbStringCompareNoCaseNTest, SunnyDayBigN) {
  const char kString[] = "0123456789";
  EXPECT_EQ(0, SbStringCompareNoCaseN(kString, kString,
                                      SbStringGetLength(kString) * 2));
}

TEST(SbStringCompareNoCaseNTest, SunnyDayCase) {
  const char kString1[] = "aBcDeFgHiJkLmNoPqRsTuVwXyZ";
  const char kString2[] = "AbCdEfGhIjKlMnOpQrStUvWxYz";
  EXPECT_EQ(0, SbStringCompareNoCaseN(kString1, kString2,
                                      SbStringGetLength(kString1)));
  EXPECT_EQ(0, SbStringCompareNoCaseN(kString2, kString1,
                                      SbStringGetLength(kString2)));

  const char kString3[] = "aBcDeFgHiJkLmaBcDeFgHiJkLm";
  const char kString4[] = "AbCdEfGhIjKlMnOpQrStUvWxYz";
  EXPECT_GT(0, SbStringCompareNoCaseN(kString3, kString4,
                                      SbStringGetLength(kString3)));
  EXPECT_LT(0, SbStringCompareNoCaseN(kString4, kString3,
                                      SbStringGetLength(kString4)));
  EXPECT_EQ(0, SbStringCompareNoCaseN(kString3, kString4,
                                      SbStringGetLength(kString3) / 2));
  EXPECT_EQ(0, SbStringCompareNoCaseN(kString4, kString3,
                                      SbStringGetLength(kString4) / 2));
}

}  // namespace
