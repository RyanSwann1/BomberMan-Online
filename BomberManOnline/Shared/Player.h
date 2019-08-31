#pragma once

#include <SFML/Graphics.hpp>
#include "PlayerControllerType.h"
#include "Timer.h"
#include "Direction.h"

struct Player
{
	Player(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
		: m_ID(ID),
		m_previousPosition(),
		m_position(startingPosition),
		m_newPosition(),
		m_controllerType(controllerType),
		m_moving(false),
		m_movementFactor(0),
		m_movementSpeed(2.5f),
		m_bombPlacementTimer(2.0f, true)
	{}
	virtual ~Player() {}

	int m_ID;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_position;
	sf::Vector2f m_newPosition;
	ePlayerControllerType m_controllerType;
	bool m_moving;
	float m_movementFactor;
	float m_movementSpeed;
	Timer m_bombPlacementTimer;
};