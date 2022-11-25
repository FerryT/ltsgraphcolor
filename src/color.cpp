// Author: Ferry Timmers

#include <cmath>
#include <unordered_map>

#include "color.h"

inline int parse_hex(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	else
		throw std::runtime_error("Incorrect hex value in color code.");
}

color_t parse_hexcode(const std::string &code)
{
	color_t color{0., 0., 0.};
	if (code.size() != 7 || code[0] != '#')
		throw std::runtime_error("Incorrect color code.");
	color.red   = (float) ((parse_hex(code[2]) << 4) | parse_hex(code[1])) / 255.;
	color.green = (float) ((parse_hex(code[4]) << 4) | parse_hex(code[3])) / 255.;
	color.blue  = (float) ((parse_hex(code[6]) << 4) | parse_hex(code[5])) / 255.;
	return color;
}

color_t hash2color(std::size_t hash)
{
	// create a cache for color hashes
	static std::unordered_map<std::size_t, color_t> colors;
	if (colors.count(hash) > 0)
		return colors.at(hash);

	// convert hash to hsl (a more agreeable colorspace than rgb)
	float h = (float) (hash & 0xffff) / (float) 0xffff;
	float s = (float) ((hash >> 16) & 0xff) / (float) 0xff;
	float l = (float) ((hash >> 24) & 0xff) / (float) 0xff;
	s = s * .5 + .25;
	l = l * .5 + .25;

	// convert to rgb
	float c = (1. - std::abs(2. * l - 1.)) * s;
	h *= 6.;
	float x = c * (1. - std::abs(std::fmod(h, 2.) - 1.));
	float m = l - c * .5;
	color_t color{m, m, m};

	if (h < 1.) { color.red   += c; color.green += x; }
	else if (h < 2.) { color.green += c; color.red   += x; }
	else if (h < 3.) { color.green += c; color.blue  += x; }
	else if (h < 4.) { color.blue  += c; color.green += x; }
	else if (h < 5.) { color.blue  += c; color.red   += x; }
	else if (h < 6.) { color.red   += c; color.blue  += x; }

	colors[hash] = color;
	return color;
}
