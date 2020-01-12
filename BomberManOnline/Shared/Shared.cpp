#include "Shared.h"
#include "TileID.h"

bool Shared::isTileOnPosition(const std::vector<TileLayer>& tileLayers, sf::Vector2f position, eTileID tileID)
{
	switch (tileID)
	{
	case eTileID::eBox:
		//see if box at position is in "Box Layer";
		break;

	case eTileID::eWall:
		//See if Wall is at position in "Wall Layer";

		break;
	}

	return false;
}

bool Shared::isTileOnPosition(const std::vector<TileLayer>& tileLayers, sf::Vector2i position, eTileID tileID)
{
	switch (tileID)
	{
	case eTileID::eBox:
		//see if box at position is in "Box Layer";
		break;

	case eTileID::eWall:
		//See if Wall is at position in "Wall Layer";

		break;
	}

	return false;
}

bool Shared::isTileCollidable(const std::vector<std::vector<eCollidableTile>>& collidableLayer, sf::Vector2f position)
{
	assert(position.x >= 0 && position.x < (m_levelSize.x * m_tileSize.x) && position.y >= 0 && position.y < (m_levelSize.y * m_tileSize.x));
	if (position.x >= 0 && position.x < m_levelSize.x * m_tileSize.y && position.y >= 0 && position.y < m_levelSize.y * m_tileSize.y)
	{
		return m_collisionLayer[static_cast<int>(position.y / m_tileSize.y)][static_cast<int>(position.x / m_tileSize.x)] == eCollidableTile::eCollidable;
	}

	return false;
}

bool Shared::isTileCollidable(const std::vector<std::vector<eCollidableTile>>& collidableLayer, sf::Vector2i position)
{
	return false;
}

void Shared::changeTileAtPosition(std::vector<TileLayer>& tileLayers, std::vector<std::vector<eCollidableTile>>& collidableTileLayer, 
	eTileID newTileID, sf::Vector2f position)
{
	
}
