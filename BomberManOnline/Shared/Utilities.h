#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Direction.h"
#include <array>

enum class eDirection;
enum class eCollidableTile;
namespace Utilities
{
	int distance(sf::Vector2f source, sf::Vector2f target, sf::Vector2i tileSize);
	int distance(sf::Vector2i source, sf::Vector2i target);
	eDirection getDirectionToAdjacentFromPosition(sf::Vector2f sourcePosition, sf::Vector2f targetPosition);
	std::array<sf::Vector2i, static_cast<size_t>(eDirection::eTotal)> getAllDirections();
	sf::Vector2f scale(sf::Vector2i tileSize, sf::Vector2i v, int i);

	bool isPositionInLevelBounds(sf::Vector2i position, sf::Vector2i levelSize);
	bool isPositionInLevelBounds(sf::Vector2f position, sf::Vector2i tileSize, sf::Vector2i levelSize);
	bool isPositionAdjacent(sf::Vector2f origin, sf::Vector2f neighbour, sf::Vector2i tileSize);
	bool isTargetInDirectSight(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, eDirection facingDirection);

	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);
	sf::Vector2f getClosestGridPosition(sf::Vector2f position, sf::Vector2i tileSize);

	sf::Vector2i convertToGridPosition(sf::Vector2f position, sf::Vector2i tileSize);
	sf::Vector2f convertToWorldPosition(sf::Vector2i position, sf::Vector2i tileSize);

	bool traverseDirection(sf::Vector2f& position, sf::Vector2f endPosition, sf::Vector2i tileSize, eDirection direction);
	int getRandomNumber(int min, int max);
}