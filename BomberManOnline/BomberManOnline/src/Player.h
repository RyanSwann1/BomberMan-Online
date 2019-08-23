#pragma once

#include <SFML/Graphics.hpp>
#include "Direction.h"

struct Player
{
	Player()
		: m_position(2, 2),
		m_movementSpeed(5.0f),
		m_sprite()
	{}

	sf::Vector2f m_position;
	eDirection m_movementDirection;
	float m_movementSpeed;
	sf::Sprite m_sprite;
};