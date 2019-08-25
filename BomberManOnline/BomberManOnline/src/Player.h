#pragma once

#include <SFML/Graphics.hpp>
#include "Direction.h"
#include "Timer.h"

struct Player
{
	Player(int tileSize)
		: m_position(),
		m_newPosition(),
		m_movementSpeed(2.5f),
		m_shape(sf::Vector2f(tileSize, tileSize)),
		m_AABB(m_position, sf::Vector2f(tileSize, tileSize)),
		m_moving(false),
		m_bombPlacementTimer(2.0f, true)
	{
		m_shape.setPosition(m_position);
		m_shape.setFillColor(sf::Color::Red);
	}

	sf::Vector2f m_position;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_newPosition;
	eDirection m_movementDirection;
	float m_movementSpeed;
	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
	bool m_moving;
	Timer m_bombPlacementTimer;
};

struct Bomb
{

};