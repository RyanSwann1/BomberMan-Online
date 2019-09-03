#include "Utilities.h"
#include "CollidableTile.h"
#include "Box.h"
#include <algorithm>
#include <random>

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

bool Utilities::isPositionCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position, sf::Vector2i tileSize)
{
	return collisionLayer[static_cast<int>(position.y / tileSize.y)][static_cast<int>(position.x / tileSize.x)] != eCollidableTile::eNonCollidable;
}

int Utilities::getRandomNumber(int min, int max)
{
	static std::random_device rd;  //Will be used to obtain a seed for the random number engine
	static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(min, max);

	return dis(gen);
}