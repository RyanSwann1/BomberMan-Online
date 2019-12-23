#include "Utilities.h"
#include "CollidableTile.h"
#include "Box.h"
#include "Direction.h"
#include <algorithm>
#include <assert.h>
#include <random>

int Utilities::distance(sf::Vector2f source, sf::Vector2f target, sf::Vector2i tileSize)
{
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(source, tileSize));
	sf::Vector2i targetPositionOnGrid(Utilities::convertToGridPosition(target, tileSize));

	return std::abs(sourcePositionOnGrid.x - targetPositionOnGrid.x) + 
		std::abs(sourcePositionOnGrid.y - targetPositionOnGrid.y);
}

int Utilities::distance(sf::Vector2i source, sf::Vector2i target)
{
	return std::abs(source.x - target.x) + std::abs(source.y - target.y);
}

eDirection Utilities::getDirectionToAdjacentFromSourcePosition(sf::Vector2f sourcePosition, sf::Vector2f targetPosition)
{
	assert(sourcePosition != targetPosition);

	if (sourcePosition.x == targetPosition.x)
	{
		assert(sourcePosition.y != targetPosition.y);
		return (targetPosition.y < sourcePosition.y ? eDirection::eUp : eDirection::eDown);
	}
	else if (sourcePosition.y == targetPosition.y)
	{
		assert(sourcePosition.x != targetPosition.x);
		return (targetPosition.x < sourcePosition.x ? eDirection::eLeft : eDirection::eRight);
	}
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

bool Utilities::isPositionAdjacent(sf::Vector2f origin, sf::Vector2f neighbour, sf::Vector2i tileSize)
{
	bool neighbourPosition = false;

	if (origin.x - tileSize.x == neighbour.x && origin.y == neighbour.y)
	{
		neighbourPosition = true;
	}
	if (origin.x + tileSize.x == neighbour.x && origin.y == neighbour.y)
	{
		neighbourPosition = true;
	}
	if (origin.y - tileSize.y == neighbour.y && origin.x == neighbour.x)
	{
		neighbourPosition = true;
	}
	if (origin.y + tileSize.y == neighbour.y && origin.x == neighbour.x)
	{
		neighbourPosition = true;
	}
	
	return neighbourPosition;
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
	return sf::Vector2f(position.x * tileSize.x, position.y * tileSize.y);
}

bool Utilities::traverseDirection(sf::Vector2f& position, sf::Vector2f endPosition, sf::Vector2i tileSize, eDirection direction)
{
	switch (direction)
	{
	case eDirection::eLeft :
		position.x -= tileSize.x;
		break;
	case eDirection::eRight :
		position.x += tileSize.x;
		break;
	case eDirection::eUp :
		position.y -= tileSize.y;
		break;
	case eDirection::eDown :
		position.y += tileSize.y;
		break;
	}

	return position == endPosition;
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