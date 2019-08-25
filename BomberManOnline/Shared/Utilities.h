#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

namespace Utilities
{
	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);

	bool isPositionCollidable(const std::vector<sf::Vector2f>& collisionLayer, sf::Vector2f position);
}