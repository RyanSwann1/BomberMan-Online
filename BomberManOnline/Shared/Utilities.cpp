#include "Utilities.h"
#include "CollidableTile.h"
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

bool Utilities::isPositionCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position)
{
	return collisionLayer[position.y / 16][position.x / 16] != eCollidableTile::eNonCollidable;
}