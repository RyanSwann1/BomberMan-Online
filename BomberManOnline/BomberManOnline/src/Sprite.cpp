#include "Sprite.h"
#include "Resources.h"

Sprite::Sprite(sf::Vector2f startingPosition, eTileID minTileID, eTileID maxTileID, float frameExpirationTime)
	: m_sprite(),
	m_animationTimer()
{
}

void Sprite::update(float deltaTime)
{
}
