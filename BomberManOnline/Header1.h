#pragma once

class Player
{
public:


protected:
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