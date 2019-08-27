#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class eCollidableTile;
struct Box;
namespace Utilities
{
	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);

	bool isPositionCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position);

	bool isPositionCollidable(const std::vector<sf::Vector2f>& collisionLayer, 
		const std::vector<sf::Vector2f>& boxes, sf::Vector2f position);

	bool isPositionCollidable(const std::vector<sf::Vector2f>& collisionLayer,
		const std::vector<Box>& boxes, sf::Vector2f position);
}