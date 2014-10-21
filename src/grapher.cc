/// @file grapher.cc
/// Implements Grapher.
#include "grapher.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

#include "error.h"

namespace fs = boost::filesystem;

namespace scram {

Grapher::Grapher() {
  gate_colors_.insert(std::make_pair("or", "blue"));
  gate_colors_.insert(std::make_pair("and", "green"));
  gate_colors_.insert(std::make_pair("not", "red"));
  gate_colors_.insert(std::make_pair("xor", "brown"));
  gate_colors_.insert(std::make_pair("inhibit", "yellow"));
  gate_colors_.insert(std::make_pair("atleast", "cyan"));
  gate_colors_.insert(std::make_pair("null", "gray"));
  gate_colors_.insert(std::make_pair("nor", "magenta"));
  gate_colors_.insert(std::make_pair("nand", "orange"));

  event_colors_.insert(std::make_pair("basic", "black"));
  event_colors_.insert(std::make_pair("undeveloped", "blue"));
  event_colors_.insert(std::make_pair("house", "green"));
  event_colors_.insert(std::make_pair("conditional", "red"));
}

void Grapher::GraphFaultTree(const FaultTreePtr& fault_tree,
                             bool prob_requested,
                             std::string output) {
  // The structure of the output:
  // List gates with their children following the tree structure.
  // List reused intermediate events as transfer gates.
  // List gates and primary events' descriptions.
  assert(output != "");
  std::string graph_name = output;
  graph_name.erase(graph_name.find_last_of("."), std::string::npos);

  std::string output_path = graph_name + "_" + fault_tree->name() + ".dot";

  graph_name = graph_name.substr(graph_name.find_last_of("/") +
                                 1, std::string::npos);
  std::ofstream out(output_path.c_str());
  if (!out.good()) {
    throw IOError(output_path +  " : Cannot write the graphing file.");
  }

  boost::to_upper(graph_name);
  out << "digraph " << graph_name << " {\n";

  // Write top event.
  // Keep track of number of repetitions of the primary events.
  std::map<std::string, int> pr_repeat;
  // Keep track of number of repetitions of the intermediate events.
  std::map<std::string, int> in_repeat;
  // Populate intermediate and primary events of the top.
  Grapher::GraphNode(fault_tree->top_event(), fault_tree->primary_events(),
                     &pr_repeat, &in_repeat, out);
  // Do the same for all intermediate events.
  boost::unordered_map<std::string, GatePtr>::const_iterator it_inter;
  for (it_inter = fault_tree->inter_events().begin();
       it_inter != fault_tree->inter_events().end();
       ++it_inter) {
    Grapher::GraphNode(it_inter->second, fault_tree->primary_events(),
                       &pr_repeat, &in_repeat, out);
  }

  // Format events.
  Grapher::FormatTopEvent(fault_tree->top_event(), out);
  Grapher::FormatIntermediateEvents(fault_tree->inter_events(),
                                    in_repeat, out);
  Grapher::FormatPrimaryEvents(fault_tree->primary_events(), pr_repeat,
                               prob_requested, out);

  out << "}";
  out.flush();
}

void Grapher::GraphNode(
    const GatePtr& t,
    const boost::unordered_map<std::string, PrimaryEventPtr>& primary_events,
    std::map<std::string, int>* pr_repeat,
    std::map<std::string, int>* in_repeat,
    std::ofstream& out) {
  // Populate intermediate and primary events of the input inter event.
  std::map<std::string, EventPtr> events_children = t->children();
  std::map<std::string, EventPtr>::iterator it_child;
  for (it_child = events_children.begin(); it_child != events_children.end();
       ++it_child) {
    // Deal with repeated primary events.
    if (primary_events.count(it_child->first)) {
      if (pr_repeat->count(it_child->first)) {
        int rep = pr_repeat->find(it_child->first)->second;
        rep++;
        pr_repeat->erase(it_child->first);
        pr_repeat->insert(std::make_pair(it_child->first, rep));
      } else {
        pr_repeat->insert(std::make_pair(it_child->first, 0));
      }
      out << "\"" << t->orig_id() << "_R0\" -> "
          << "\"" << it_child->second->orig_id() <<"_R"
          << pr_repeat->find(it_child->first)->second << "\";\n";
    } else {  // This must be an intermediate event.
      if (in_repeat->count(it_child->first)) {
        int rep = in_repeat->find(it_child->first)->second;
        rep++;
        in_repeat->erase(it_child->first);
        in_repeat->insert(std::make_pair(it_child->first, rep));
      } else {
        in_repeat->insert(std::make_pair(it_child->first, 0));
      }
      out << "\"" << t->orig_id() << "_R0\" -> "
          << "\"" << it_child->second->orig_id() <<"_R"
          << in_repeat->find(it_child->first)->second << "\";\n";
    }
  }
}

void Grapher::FormatTopEvent(const GatePtr& top_event, std::ofstream& out) {
  std::string gate = top_event->type();

  // Special case for inhibit gate.
  if (gate == "and" && top_event->HasAttribute("flavor"))
    gate = top_event->GetAttribute("flavor").value;

  std::string gate_color = "black";
  if (gate_colors_.count(gate)) {
    gate_color = gate_colors_.find(gate)->second;
  }

  boost::to_upper(gate);
  out << "\"" <<  top_event->orig_id()
      << "_R0\" [shape=ellipse, "
      << "fontsize=12, fontcolor=black, fontname=\"times-bold\", "
      << "color=" << gate_color << ", "
      << "label=\"" << top_event->orig_id() << "\\n"
      << "{ " << gate;
  if (gate == "ATLEAST") {
    out << " " << top_event->vote_number() << "/"
        << top_event->children().size();
  }
  out << " }\"]\n";
}

void Grapher::FormatIntermediateEvents(
    const boost::unordered_map<std::string, GatePtr>& inter_events,
    const std::map<std::string, int>& in_repeat,
    std::ofstream& out) {
  std::map<std::string, int>::const_iterator it;
  for (it = in_repeat.begin(); it != in_repeat.end(); ++it) {
    std::string gate = inter_events.find(it->first)->second->type();

    if (inter_events.find(it->first)->second->HasAttribute("flavor") &&
        gate == "and")
      gate =
          inter_events.find(it->first)->second->GetAttribute("flavor").value;

    std::string gate_color = "black";
    if (gate_colors_.count(gate)) {
      gate_color = gate_colors_.find(gate)->second;
    }
    boost::to_upper(gate);  // This is for graphing.
    std::string type = inter_events.find(it->first)->second->type();
    std::string orig_name = inter_events.find(it->first)->second->orig_id();
    for (int i = 0; i <= it->second; ++i) {
      if (i == 0) {
        out << "\"" <<  orig_name << "_R" << i
            << "\" [shape=box, ";
      } else {
        // Repetition is a transfer symbol.
        out << "\"" <<  orig_name << "_R" << i
            << "\" [shape=triangle, ";
      }
      out << "fontsize=10, fontcolor=black, "
          << "color=" << gate_color << ", "
          << "label=\"" << orig_name << "\\n"
          << "{ " << gate;
      if (gate == "ATLEAST") {
        out << " " << inter_events.find(it->first)->second->vote_number()
            << "/" << inter_events.find(it->first)->second->children().size();
      }
      out << " }\"]\n";
    }
  }
}

void Grapher::FormatPrimaryEvents(
    const boost::unordered_map<std::string, PrimaryEventPtr>& primary_events,
    const std::map<std::string, int>& pr_repeat,
    bool prob_requested,
    std::ofstream& out) {
  std::map<std::string, int>::const_iterator it;
  for (it = pr_repeat.begin(); it != pr_repeat.end(); ++it) {
    for (int i = 0; i < it->second + 1; ++i) {
      std::string id = it->first;
      std::string type = primary_events.find(id)->second->type();
      std::string orig_name = primary_events.find(id)->second->orig_id();
      // Detect undeveloped or conditional event.
      if (type == "basic" &&
          primary_events.find(id)->second->HasAttribute("flavor")) {
        type = primary_events.find(id)->second->GetAttribute("flavor").value;
      }

      out << "\"" << orig_name << "_R" << i
          << "\" [shape=circle, "
          << "height=1, fontsize=10, fixedsize=true, "
          << "fontcolor=" << event_colors_.find(type)->second
          << ", " << "label=\"" << orig_name << "\\n["
          << type << "]";
      if (prob_requested) { out << "\\n"
                                << primary_events.find(id)->second->p(); }
      out << "\"]\n";
    }
  }
}

}  // namespace scram
