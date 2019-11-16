#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class eCollidableTile;
class Server;
namespace Utilities
{
	float distance(sf::Vector2f source, sf::Vector2f target, sf::Vector2i tileSize);
	float distance(sf::Vector2i source, sf::Vector2i destination);

	bool isPositionNeighbouringBox(const Server& server, sf::Vector2f position);

	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);

	bool isPositionCollidable(const Server& server, sf::Vector2f position);

	int getRandomNumber(int min, int max);
}