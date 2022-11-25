// Author: Ferry Timmers
// License: MIT
// Description: A tool that colors states of a ltsgraph export file.

#include <iostream>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <regex>

#include "tinyxml2.h"
#include "color.h"

using namespace tinyxml2;
using std::string;
using std::size_t;
using std::uint64_t;
using indices_t = std::unordered_map<uint64_t, size_t>;
// A set of state indices, where each index is mapped to a hash of the machted label

template <typename T> T *check(T *value)
{
	if (value == nullptr)
		throw std::runtime_error("Unable to parse XML file.");
	return value;
}

inline size_t hash(const string &str)
{
	return std::hash<string>()(str);
}

indices_t match_statelabels(XMLElement *graph, const std::regex &pattern)
{
	indices_t states;
	XMLElement *statelabel = graph->FirstChildElement("StateLabel");
	while (statelabel != nullptr)
	{
		const string label = statelabel->Attribute("label");
		if (std::regex_match(label, pattern))
			states[statelabel->Unsigned64Attribute("value")] = hash(label);
		statelabel = statelabel->NextSiblingElement("StateLabel");
	}
	return states;
}

indices_t match_transitionlabels(XMLElement *graph, const std::regex &pattern)
{
	indices_t labels;
	XMLElement *trlabel = graph->FirstChildElement("TransitionLabel");
	while (trlabel != nullptr)
	{
		const string label = trlabel->Attribute("label");
		if (std::regex_match(label, pattern))
			labels[trlabel->Unsigned64Attribute("value")] = hash(label);
		trlabel = trlabel->NextSiblingElement("TransitionLabel");
	}
	return labels;
}

indices_t match_selfloops(XMLElement *graph, const indices_t &labels)
{
	indices_t states;
	XMLElement *transition = graph->FirstChildElement("Transition");
	while (transition != nullptr)
	{
		if (transition->Unsigned64Attribute("from") == transition->Unsigned64Attribute("to"))
		{
			XMLElement *node = transition->NextSiblingElement("TransitionLabelNode");
			if (node != nullptr)
			{
				uint64_t index = node->Unsigned64Attribute("labelindex");
				if (labels.count(index) > 0)
					states[transition->Unsigned64Attribute("from")] = labels.at(index);
			}
		}
		transition = transition->NextSiblingElement("Transition");
	}
	return states;
}

void color_states(XMLElement *graph, const indices_t &states, const color_t &color)
{
	XMLElement *state = graph->FirstChildElement("State");
	while (state != nullptr)
	{
		if (states.count(state->Unsigned64Attribute("value")) > 0)
		{
			state->SetAttribute("red", color.red);
			state->SetAttribute("green", color.green);
			state->SetAttribute("blue", color.blue);
		}
		state = state->NextSiblingElement("State");
	}
}

void color_states(XMLElement *graph, const indices_t &states)
{
	XMLElement *state = graph->FirstChildElement("State");
	while (state != nullptr)
	{
		const uint64_t index = state->Unsigned64Attribute("value");
		if (states.count(index) > 0)
		{
			color_t color = hash2color(states.at(index));
			state->SetAttribute("red", color.red);
			state->SetAttribute("green", color.green);
			state->SetAttribute("blue", color.blue);
		}
		state = state->NextSiblingElement("State");
	}
}

int main(int argc, char const *argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <input.xml> [@]<regex pattern> [color] [output.xml]" << std::endl
			<< std::endl
			<< "This tool colors the states of an exported ltsgraph graph according to a regular expression." << std::endl
			<< "If the regex pattern is prefixed by @ it matches on the transition labels of self-loops," << std::endl
			<< "otherwise it matches on state labels." << std::endl
			<< "A hexadecimal color code can be specified (#RRGGBB), otherwise a color hash is used." << std::endl;
		return 0;
	}

	try {
		XMLDocument xml;

		xml.LoadFile(argv[1]);

		bool useLoops = argv[2][0] == '@';
		std::regex pattern(&argv[2][useLoops ? 1 : 0]);

		XMLElement *graph = check(xml.RootElement());
		if (string(graph->Name()) != "Graph")
			throw std::runtime_error("XML file does not have the correct root element.");

		indices_t states = useLoops ?
			match_selfloops(graph, match_transitionlabels(graph, pattern)) :
			match_statelabels(graph, pattern);
		
		if (argc < 4)
			color_states(graph, states);
		else
			color_states(graph, states, parse_hexcode(argv[3]));

		if (argc < 5)
		{
			string outfile = argv[1];
			string::size_type pos = outfile.find_last_of(".");
			if (pos != string::npos)
				outfile.erase(pos);
			outfile += ".colored.xml";
			xml.SaveFile(outfile.c_str());
		}
		else
			xml.SaveFile(argv[4]);

	} catch (const std::exception &e) {
		std::cerr << "[Error] " << e.what() << std::endl;
		return -1;
	}
	return 0;
}
