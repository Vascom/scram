/*
 * Copyright (C) 2014-2017 Olzhas Rakhimov
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

#include <gtest/gtest.h>

#include "risk_analysis_tests.h"

namespace scram {
namespace core {
namespace test {

// Benchmark Tests for Chinese fault tree from XFTA.
TEST_P(RiskAnalysisTest, ChineseTree) {
  std::vector<std::string> input_files = {
      "./share/scram/input/Chinese/chinese.xml",
      "./share/scram/input/Chinese/chinese-basic-events.xml"};
  settings.probability_analysis(true).importance_analysis(true);
  ASSERT_NO_THROW(ProcessInputFiles(input_files));
  ASSERT_NO_THROW(analysis->Analyze());
  if (settings.approximation() == Approximation::kRareEvent) {
    EXPECT_NEAR(0.004804, p_total(), 1e-5);
    TestImportance(
        {{"e17", {88, 6.5536e-06, 2.72814e-05, 0.0200267, 1.00134, 1.00003}},
         {"e18", {88, 6.5536e-06, 2.72814e-05, 0.0200267, 1.00134, 1.00003}},
         {"e21", {136, 2.944e-06, 1.22553e-05, 0.020012, 1.0006, 1.00001}},
         {"e19", {92, 5.312e-06, 2.21128e-05, 0.0200217, 1.00108, 1.00002}},
         {"e20", {92, 5.312e-06, 2.21128e-05, 0.0200217, 1.00108, 1.00002}},
         {"e9", {48, 6.78912e-05, 0.000282618, 0.020277, 1.01385, 1.00028}},
         {"e10", {48, 6.78912e-05, 0.000282618, 0.020277, 1.01385, 1.00028}},
         {"e11", {48, 6.78912e-05, 0.000282618, 0.020277, 1.01385, 1.00028}},
         {"e12", {136, 0.000108314, 0.000450888, 0.0204419, 1.02209, 1.00045}},
         {"e13", {136, 0.000108314, 0.000450888, 0.0204419, 1.02209, 1.00045}},
         {"e1", {40, 0.080002, 0.333033, 0.346372, 17.3186, 1.49932}},
         {"e2", {40, 0.080002, 0.333033, 0.346372, 17.3186, 1.49932}},
         {"e3", {40, 0.080002, 0.333033, 0.346372, 17.3186, 1.49932}},
         {"e4", {21, 0.0600499, 0.249976, 0.264977, 13.2488, 1.33329}},
         {"e5", {21, 0.0600499, 0.249976, 0.264977, 13.2488, 1.33329}},
         {"e6", {21, 0.0600499, 0.249976, 0.264977, 13.2488, 1.33329}},
         {"e7", {21, 0.0600499, 0.249976, 0.264977, 13.2488, 1.33329}},
         {"e14", {52, 5.8112e-06, 2.41909e-05, 0.0200237, 1.00119, 1.00002}},
         {"e15", {52, 5.8112e-06, 2.41909e-05, 0.0200237, 1.00119, 1.00002}},
         {"e16", {52, 5.8112e-06, 2.41909e-05, 0.0200237, 1.00119, 1.00002}},
         {"e22", {154, 1.14688e-05, 4.77424e-05, 0.0200468, 1.00234, 1.00005}},
         {"e23", {154, 1.14688e-05, 4.77424e-05, 0.0200468, 1.00234, 1.00005}},
         {"e24", {154, 1.14688e-05, 4.77424e-05, 0.0200468, 1.00234, 1.00005}},
         {"e25", {154, 1.14688e-05, 4.77424e-05, 0.0200468, 1.00234, 1.00005}},
         {"e8", {180, 0.00020567, 0.000856166, 0.020839, 1.04195, 1.00086}}});
  } else {
    EXPECT_NEAR(0.0045691, p_total(), 1e-5);
    TestImportance(
        {{"e1", {40, 0.0745557, 0.326332, 0.339805, 16.9902, 1.48441}},
         {"e2", {40, 0.0745557, 0.326332, 0.339805, 16.9902, 1.48441}},
         {"e3", {40, 0.0745557, 0.326332, 0.339805, 16.9902, 1.48441}},
         {"e9", {48, 5.89643e-05, 0.000258088, 0.0202529, 1.01265, 1.00026}},
         {"e10", {48, 5.89643e-05, 0.000258088, 0.0202529, 1.01265, 1.00026}},
         {"e11", {48, 5.89643e-05, 0.000258088, 0.0202529, 1.01265, 1.00026}},
         {"e17", {88, 5.64596e-06, 2.47125e-05, 0.0200242, 1.00121, 1.00002}},
         {"e18", {88, 5.64596e-06, 2.47125e-05, 0.0200242, 1.00121, 1.00002}},
         {"e19", {92, 4.60982e-06, 2.01773e-05, 0.0200198, 1.00099, 1.00002}},
         {"e20", {92, 4.60982e-06, 2.01773e-05, 0.0200198, 1.00099, 1.00002}},
         {"e21", {136, 2.3919e-06, 1.04694e-05, 0.0200103, 1.00051, 1.00001}},
         {"e12", {136, 9.51481e-05, 0.000416465, 0.0204081, 1.02041, 1.00042}},
         {"e13", {136, 9.51481e-05, 0.000416465, 0.0204081, 1.02041, 1.00042}},
         {"e4", {21, 0.0553923, 0.242453, 0.257604, 12.8802, 1.32005}},
         {"e5", {21, 0.0553923, 0.242453, 0.257604, 12.8802, 1.32005}},
         {"e6", {21, 0.0553923, 0.242453, 0.257604, 12.8802, 1.32005}},
         {"e7", {21, 0.0553923, 0.242453, 0.257604, 12.8802, 1.32005}},
         {"e14", {52, 5.1467e-06, 2.25272e-05, 0.0200221, 1.0011, 1.00002}},
         {"e15", {52, 5.1467e-06, 2.25272e-05, 0.0200221, 1.0011, 1.00002}},
         {"e16", {52, 5.1467e-06, 2.25272e-05, 0.0200221, 1.0011, 1.00002}},
         {"e22", {154, 1.03582e-05, 4.5338e-05, 0.0200444, 1.00222, 1.00005}},
         {"e23", {154, 1.03582e-05, 4.5338e-05, 0.0200444, 1.00222, 1.00005}},
         {"e24", {154, 1.03582e-05, 4.5338e-05, 0.0200444, 1.00222, 1.00005}},
         {"e25", {154, 1.03582e-05, 4.5338e-05, 0.0200444, 1.00222, 1.00005}},
         {"e8", {180, 0.000181647, 0.000795073, 0.0207792, 1.03896, 1.0008}}});
  }
  // Minimal cut set check.
  EXPECT_EQ(392, products().size());
  std::vector<int> distr = {0, 12, 0, 24, 188, 168};
  EXPECT_EQ(distr, ProductDistribution());
}

}  // namespace test
}  // namespace core
}  // namespace scram
