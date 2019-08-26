#pragma once

#include <SFML/Graphics.hpp>
#include "Texture.h"

struct Bomb
{
	Bomb(const Texture& texture, int ID, sf::Vector2f startingPosition)
		: m_ID(ID),
		m_position(startingPosition),
		m_sprite(texture.getTexture(), texture.getFrameRect(236))
	{
		m_sprite.setPosition(startingPosition);
	}

	int m_ID;
	sf::Vector2f m_position;
	sf::Sprite m_sprite;
	int m_owningPlayerID;
};