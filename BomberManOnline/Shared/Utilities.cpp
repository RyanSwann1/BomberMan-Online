#include "Utilities.h"
#include "Box.h"
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

bool Utilities::isPositionCollidable(const std::vector<sf::Vector2f>& collisionLayer, const std::vector<sf::Vector2f>& boxes, sf::Vector2f position)
{
	auto cIter = std::find_if(collisionLayer.cbegin(), collisionLayer.cend(), [position](const auto collidablePosition)
		{ return collidablePosition == position; });

	if (cIter == collisionLayer.end())
	{
		auto boxColision = std::find_if(boxes.cbegin(), boxes.cend(), [position](const auto box) { return box == position; });
		if (boxColision == boxes.cend())
		{
			return false;
		}
	}

	return true;
}

bool Utilities::isPositionCollidable(const std::vector<sf::Vector2f>& collisionLayer, const std::vector<Box>& boxes, sf::Vector2f position)
{
	auto cIter = std::find_if(collisionLayer.cbegin(), collisionLayer.cend(), [position](const auto collidablePosition)
	{ return collidablePosition == position; });

	if (cIter == collisionLayer.end())
	{
		auto boxColision = std::find_if(boxes.cbegin(), boxes.cend(), [position](const auto& box) { return box.position == position; });
		if (boxColision == boxes.cend())
		{
			return false;
		}
	}

	return true;
}

