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

/// @file mocus.cc
/// Implementation of the MOCUS algorithm.
/// The algorithm assumes
/// that the graph is in negation normal form.
///
/// A ZBDD data structure is employed to store and extract
/// intermediate (containing gates)
/// and final (basic events only) cut sets
/// upon cut set generation.

#include "mocus.h"

#include "logger.h"

namespace scram {
namespace core {

Mocus::Mocus(const Pdag* fault_tree, const Settings& settings)
    : constant_graph_(false),
      graph_(fault_tree),
      kSettings_(settings) {
  const GatePtr& top_gate = fault_tree->root();
  if (top_gate->IsConstant() || top_gate->type() == kNull) {
    constant_graph_ = true;
    zbdd_ = std::make_unique<Zbdd>(fault_tree, settings);
    zbdd_->Analyze();
  }
}

void Mocus::Analyze() {
  BLOG(DEBUG2, constant_graph_) << "Graph is constant. No analysis!";
  if (constant_graph_)
    return;

  CLOCK(mcs_time);
  LOG(DEBUG2) << "Start minimal cut set generation.";
  zbdd_ = AnalyzeModule(*graph_->root(), kSettings_);
  LOG(DEBUG2) << "Delegating cut set extraction to ZBDD.";
  zbdd_->Analyze();
  LOG(DEBUG2) << "Minimal cut sets found in " << DUR(mcs_time);
}

const std::vector<std::vector<int>>& Mocus::products() const {
  assert(zbdd_ && "Analysis is not done.");
  return zbdd_->products();
}

std::unique_ptr<zbdd::CutSetContainer>
Mocus::AnalyzeModule(const Gate& gate, const Settings& settings) noexcept {
  assert(gate.module() && "Expected only module gates.");
  CLOCK(gen_time);
  LOG(DEBUG3) << "Finding cut sets from module: G" << gate.index();
  LOG(DEBUG4) << "Limit on product order: " << settings.limit_order();
  std::unordered_map<int, const Gate*> gates;
  auto add_gates = [&gates](const Gate::ArgMap<Gate>& args) {
    for (const Gate::Arg<Gate>& arg : args)
      gates.emplace(arg.first, arg.second.get());
  };
  add_gates(gate.args<Gate>());
  const int kMaxVariableIndex =
      Pdag::kVariableStartIndex + graph_->basic_events().size() - 1;
  auto container = std::make_unique<zbdd::CutSetContainer>(
      kSettings_, gate.index(), kMaxVariableIndex);
  container->Merge(container->ConvertGate(gate));
  while (int next_gate_index = container->GetNextGate()) {
    LOG(DEBUG5) << "Expanding gate G" << next_gate_index;
    const Gate* next_gate = gates.find(next_gate_index)->second;
    add_gates(next_gate->args<Gate>());

    container->Merge(container->ExpandGate(
        container->ConvertGate(*next_gate),
        container->ExtractIntermediateCutSets(next_gate_index)));
  }
  container->Minimize();
  container->Log();
  LOG(DEBUG3) << "G" << gate.index()
              << " cut set generation time: " << DUR(gen_time);
  if (!gate.coherent()) {
    container->EliminateComplements();
    container->Minimize();
  }
  for (const auto& entry : container->GatherModules()) {
    int index = entry.first;
    assert(index > 0 && "No complement modules are expected.");
    int limit = entry.second.second;
    assert(limit >= 0 && "Order cut-off is not strict.");
    bool coherent = entry.second.first;
    if (limit == 0 && coherent) {  // Unity is impossible.
      auto empty_zbdd = std::make_unique<zbdd::CutSetContainer>(
          kSettings_, index, kMaxVariableIndex);
      container->JoinModule(index, std::move(empty_zbdd));
      continue;
    }
    Settings adjusted(settings);
    adjusted.limit_order(limit);
    container->JoinModule(index,
                          AnalyzeModule(*gates.find(index)->second, adjusted));
  }
  container->EliminateConstantModules();
  container->Minimize();
  return container;
}

}  // namespace core
}  // namespace scram
