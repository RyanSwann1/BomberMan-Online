#pragma once

#include <SFML/Graphics.hpp>

struct Box
{
	Box(sf::Vector2f startingPosition);

	sf::Vector2f position;
	sf::Sprite sprite;
};