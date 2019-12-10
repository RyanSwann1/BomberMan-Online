#include "Utilities.h"
#include "CollidableTile.h"
#include "Box.h"
#include <algorithm>
#include <random>

float Utilities::distance(sf::Vector2f source, sf::Vector2f target, sf::Vector2i tileSize)
{
	sf::Vector2i roundedSourcePosition(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));
	sf::Vector2i roundedTargetPosition(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y));

	return std::abs(static_cast<float>(roundedSourcePosition.x - roundedTargetPosition.x)) + 
		static_cast<float>(std::abs(roundedSourcePosition.y - roundedTargetPosition.y));
}

float Utilities::distance(sf::Vector2i source, sf::Vector2i destination)
{
	return std::abs(static_cast<float>(source.x - destination.x)) + static_cast<float>(std::abs(source.y - destination.y));
}

bool Utilities::isPositionNeighbouringBox(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position, sf::Vector2i levelSize, sf::Vector2i tileSize)
{
	sf::Vector2i roundedPosition(position.x / tileSize.x, position.y / tileSize.y);
	for (int x = roundedPosition.x - 1; x <= roundedPosition.x + 1; x += 2)
	{
		if (x >= 0 && x < levelSize.x && collisionLayer[roundedPosition.y][x] == eCollidableTile::eBox) 
		{
			return true;
		}
	}

	for (int y = roundedPosition.y - 1; y <= roundedPosition.y + 1; y += 2)
	{
		if (y >= 0 && y < levelSize.y && collisionLayer[y][roundedPosition.x] == eCollidableTile::eBox)
		{
			return true;
		}
	}

	return false;
}

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

sf::Vector2f Utilities::getClosestGridPosition(sf::Vector2f position, sf::Vector2i tileSize)
{
	sf::Vector2f centerPosition(position.x + (tileSize.x / 2.0f), position.y + (tileSize.y / 2.0f));

	sf::Vector2f pos((static_cast<int>(centerPosition.x / tileSize.x)) * tileSize.x, static_cast<int>((centerPosition.y / tileSize.y)) * tileSize.y);
	return pos;
}

sf::Vector2i Utilities::convertToGridPosition(sf::Vector2f position, sf::Vector2i tileSize)
{
	return sf::Vector2i(position.x / tileSize.x, position.y / tileSize.y);
}

sf::Vector2f Utilities::convertToWorldPosition(sf::Vector2i position, sf::Vector2i tileSize)
{
	return sf::Vector2f(position.x / tileSize.x, position.y / tileSize.y);
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