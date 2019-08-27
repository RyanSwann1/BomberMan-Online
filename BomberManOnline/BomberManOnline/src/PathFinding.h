#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class PathFinding
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}


private:

};