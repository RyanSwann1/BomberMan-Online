#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class eDirection;
enum class eCollidableTile;
class TileLayer;
enum class eTileID;
namespace Utilities
{
	int distance(sf::Vector2f source, sf::Vector2f target, sf::Vector2i tileSize);
	int distance(sf::Vector2i source, sf::Vector2i target);
	int getRandomNumber(int min, int max);

	bool isPositionAdjacent(sf::Vector2f origin, sf::Vector2f neighbour, sf::Vector2i tileSize);
	bool isTargetInDirectSight(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, eDirection facingDirection);
	bool traverseDirection(sf::Vector2f& position, sf::Vector2f endPosition, sf::Vector2i tileSize, eDirection direction);

	eDirection getDirectionToAdjacentFromPosition(sf::Vector2f sourcePosition, sf::Vector2f targetPosition);

	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);
	sf::Vector2f getClosestGridPosition(sf::Vector2f position, sf::Vector2i tileSize);
	sf::Vector2i convertToGridPosition(sf::Vector2f position, sf::Vector2i tileSize);
	sf::Vector2f convertToWorldPosition(sf::Vector2i position, sf::Vector2i tileSize);

	void getClosestGridPosition(sf::Vector2f position, sf::Vector2i tileSize, sf::Vector2i& newPosition);
}