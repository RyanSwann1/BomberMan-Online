#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class eCollidableTile;
struct Box;
namespace Utilities
{
	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);

	bool isPositionCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position);

	int getRandomNumber(int min, int max);
}