// Color functions
// Author: Ferry Timmers

#ifndef _COLOR_H
#define _COLOR_H

#include <string>
#include <cstddef>

struct color_t
{
	float red, green, blue;
};

color_t parse_hexcode(const std::string &code);

color_t hash2color(std::size_t hash);

#endif /*_COLOR_H*/
