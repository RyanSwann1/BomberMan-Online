#pragma once

#include <SFML/Graphics.hpp>
#include "Timer.h"

enum class eTileID
{
	ePlayerMoveUpStart = 263,
	ePlayerMoveUpEnd = 264,
	ePlayerMoveDownStart = 256,
	ePlayerMoveDownEnd = 258,
	ePlayerMoveRightStart = 259,
	ePlayerMoveRightEnd = 262,
	ePlayerMoveLeftStart = 259,
	ePlayerMoveLeftEnd = 262,
	eBombEnd = 236,
	eBombStart = 266
};

enum class eAnimationType
{
	eHorizontal = 0,
	eVertical
};

class Sprite
{
public:
	Sprite(sf::Vector2f startingPosition, eTileID startTileID, eTileID endTileID, float frameExpirationTime);

	void update(float deltaTime);

private:
	sf::Sprite m_sprite;
	Timer m_animationTimer;
	int m_startTileID;
	int m_endTileID;
};