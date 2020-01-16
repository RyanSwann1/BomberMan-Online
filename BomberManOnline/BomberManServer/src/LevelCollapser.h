#pragma once

#include "NonCopyable.h"
#include <SFML/Graphics.hpp>
#include "Direction.h"
#include "Timer.h"

class TileManager;
class Server;
class LevelCollapser : private NonCopyable
{
public:
	LevelCollapser();

	void activate(sf::Vector2f startingPosition);
	void update(Server& server, TileManager& tileManager, float frameTime);

private:		
	const float m_elaspedTimeUntilStart;
	sf::Vector2f m_startingPosition;
	int m_incrementAmount;
	int m_currentAmount;
	sf::Vector2f m_currentPlacementPosition;
	eDirection m_placementDirection;
	bool m_firstPass;
	bool m_disabled;
	Timer m_placementTimer;

	void placeNextCollidableTile(Server& server, TileManager& tileManager);
};