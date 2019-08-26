#pragma once

#include <SFML/Graphics.hpp>
#include "Texture.h"
#include "Timer.h"

struct Bomb
{
	Bomb(const Texture& texture, sf::Vector2f startingPosition, float expirationTime)
		: m_position(startingPosition),
		m_sprite(texture.getTexture(), texture.getFrameRect(236)),
		m_lifeTimer(expirationTime, true)
	{
		m_sprite.setPosition(startingPosition);
	}

	sf::Vector2f m_position;
	sf::Sprite m_sprite;
	Timer m_lifeTimer;
};