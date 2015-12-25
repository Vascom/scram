/*
 * Copyright (C) 2014-2015 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Performance testing is done only if requested
// by activating disabled tests.
//
// To run the performance tests,
// supply "--gtest_also_run_disabled_tests" flag to GTest.
// The GTest filter may be applied to filter only performance tests.
// Different tests are compiled depending on the build type.
// Generally, debug or non-debug types are recognized.
//
// Performance testing values are taken
// from a computer with the following specs:
//
//   Proc         Core i7-2820QM
//   Ubuntu       14.04 64bit
//   GCC          4.8.4
//   Boost        1.55
//   TCMalloc     2.1
//
// The values for performance are expected to have some random variation.
// Better as well as worse performance are reported
// as test failures to indicate the change.
//
// NOTE: Running all the tests may take considerable time.
// NOTE: Running tests several times is recommended
//       to take into account the random variation of time results.

#include "performance_tests.h"

#include <string>

namespace scram {
namespace test {

// Tests the performance of probability calculations
// with cut-off approximations tests are done.
TEST_F(PerformanceTest, DISABLED_ThreeMotor) {
  double p_time_std = 0.01;
  std::string input = "./share/scram/input/ThreeMotor/three_motor.xml";
  settings.probability_analysis(true);
  ASSERT_NO_THROW(Analyze(input));
  EXPECT_LT(ProbCalcTime(), p_time_std);
}

TEST_F(PerformanceTest, DISABLED_ChineseTree) {
  double mcs_time = 0.1;
  std::vector<std::string> input_files;
  input_files.push_back("./share/scram/input/Chinese/chinese.xml");
  input_files.push_back("./share/scram/input/Chinese/chinese-basic-events.xml");
  settings.probability_analysis(false);
  ASSERT_NO_THROW(Analyze(input_files));
  EXPECT_LT(McsGenerationTime(), mcs_time);
  // EXPECT_LT(ProbCalcTime(), p_time);
}

TEST_F(PerformanceTest, DISABLED_200Event) {
  double mcs_time = 0.5;
  std::string input = "./share/scram/input/Autogenerated/200_event.xml";
  ASSERT_NO_THROW(Analyze(input));
  EXPECT_EQ(15347, NumOfMcs());
  EXPECT_LT(McsGenerationTime(), mcs_time);
}

TEST_F(PerformanceTest, DISABLED_Baobab1_L7) {
  double mcs_time = 4.7;
#ifdef NDEBUG
  mcs_time = 0.55;
#endif
  std::vector<std::string> input_files;
  input_files.push_back("./share/scram/input/Baobab/baobab1.xml");
  input_files.push_back("./share/scram/input/Baobab/baobab1-basic-events.xml");
  settings.limit_order(7);
  ASSERT_NO_THROW(Analyze(input_files));
  EXPECT_EQ(17432, NumOfMcs());
  EXPECT_NEAR(mcs_time, McsGenerationTime(), mcs_time * delta);
}

TEST_F(PerformanceTest, DISABLED_CEA9601_BDD) {
  double mcs_time = 20;
#ifdef NDEBUG
  mcs_time = 6;
#endif
  std::vector<std::string> input_files;
  input_files.push_back("./share/scram/input/CEA9601/CEA9601.xml");
  input_files.push_back("./share/scram/input/CEA9601/CEA9601-basic-events.xml");
  settings.limit_order(4).algorithm("bdd");
  ASSERT_NO_THROW(Analyze(input_files));
  EXPECT_EQ(54436, NumOfMcs());
  EXPECT_NEAR(mcs_time, McsGenerationTime(), mcs_time * delta);
}

TEST_F(PerformanceTest, DISABLED_Baobab2) {
  double mcs_time = 1.5;
#ifdef NDEBUG
  mcs_time = 0.20;
#endif
  std::vector<std::string> input_files;
  input_files.push_back("./share/scram/input/Baobab/baobab2.xml");
  input_files.push_back("./share/scram/input/Baobab/baobab2-basic-events.xml");
  ASSERT_NO_THROW(Analyze(input_files));
  EXPECT_EQ(4805, NumOfMcs());
  EXPECT_NEAR(mcs_time, McsGenerationTime(), mcs_time * delta);
}

TEST_F(PerformanceTest, DISABLED_Baobab1) {
  double mcs_time = 6.5;
#ifdef NDEBUG
  mcs_time = 1.0;
#endif
  std::vector<std::string> input_files;
  input_files.push_back("./share/scram/input/Baobab/baobab1.xml");
  input_files.push_back("./share/scram/input/Baobab/baobab1-basic-events.xml");
  settings.limit_order(8);
  ASSERT_NO_THROW(Analyze(input_files));
  EXPECT_EQ(25892, NumOfMcs());
  EXPECT_NEAR(mcs_time, McsGenerationTime(), mcs_time * delta);
}

}  // namespace test
}  // namespace scram
