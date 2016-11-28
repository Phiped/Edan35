#pragma once

#include <string>

namespace config
{
	constexpr unsigned int msaa_rate = 1;
	constexpr unsigned int resolution_x = 1600;
	constexpr unsigned int resolution_y = 900;

	inline std::string shaders_path(std::string const& path)
	{
		return std::string("C:/Users/Tobbe/Desktop/edan35/CG_Labs/shaders/") + path;
	}
	inline std::string resources_path(std::string const& path)
	{
		return std::string("C:/Users/Tobbe/Desktop/edan35/CG_Labs/res/") + path;
	}
}
