#include "Utilities.h"
#include <algorithm>

sf::Vector2f Utilities::Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor)
{;
	if (factor > 1.f)
	{
		factor = 1.f;
	}
	else if (factor < 0.f)
	{
		factor = 0.f;
	}

	return pointA + (pointB - pointA) * factor;
}

bool Utilities::isPositionCollidable(const std::vector<sf::Vector2f>& collisionLayer, sf::Vector2f position)
{
	auto cIter = std::find_if(collisionLayer.begin(), collisionLayer.end(), [position](const auto collidablePosition)
		{ return collidablePosition == position; });

	if (cIter == collisionLayer.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}