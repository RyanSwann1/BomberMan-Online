#include "TileManager.h"

//
//bool Shared::isTileOnPosition(const std::vector<TileLayer>& tileLayers, eTileID tile, sf::Vector2f position, sf::Vector2i tileSize)
//{
//	sf::Vector2i positionOnGrid = Utilities::convertToGridPosition(position, tileSize);
//	switch (tile)
//	{
//	case eTileID::eBox:
//		return tile == getTileAtPosition(tileLayers, "Box Layer", positionOnGrid);
//
//	case eTileID::eWall:
//		return tile == getTileAtPosition(tileLayers, "Wall Layer", positionOnGrid);
//	}
//
//	return false;
//}
//
//bool Shared::isTileOnPosition(const std::vector<TileLayer>& tileLayers, eTileID tile, sf::Vector2i position)
//{
//	switch (tile)
//	{
//	case eTileID::eBox:
//		return tile == getTileAtPosition(tileLayers, "Box Layer", position);
//
//	case eTileID::eWall:
//		return tile == getTileAtPosition(tileLayers, "Wall Layer", position);
//	}
//
//	return false;
//}
//
//bool Shared::isTileCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position, sf::Vector2i tileSize)
//{
//	sf::Vector2i positionOnGrid = Utilities::convertToGridPosition(position, tileSize);
//	return collisionLayer[positionOnGrid.y][positionOnGrid.x] == eCollidableTile::eCollidable;
//}
//
//bool Shared::isTileCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i position)
//{
//	return collisionLayer[position.y][position.x] == eCollidableTile::eCollidable;
//}
//
//void Shared::removeTileAtPosition(std::vector<TileLayer>& tileLayers, std::vector<std::vector<eCollidableTile>>& collisionLayer, 
//	eTileID tile, sf::Vector2f position, sf::Vector2i tileSize)
//{
//	switch (tile)
//	{
//	case eTileID::eBox :
//	{
//		auto* tileLayer = getTileLayer(tileLayers, "Box Layer");
//		assert(tileLayer);
//		if (tileLayer)
//		{
//			tileLayer->changeTile(Utilities::convertToGridPosition(position, tileSize), tile);
//			collisionLayer[]
//		}
//	}
//	
//	break;
//	case eTileID::eWall:
//	{
//		auto* tileLayer = getTileLayer(tileLayers, "Wall Layer");
//		assert(tileLayer);
//		if (tileLayer)
//		{
//			tileLayer->changeTile(Utilities::convertToGridPosition(position, tileSize), tile);
//		}
//	}
//		break;
//	}
//}

TileManager::TileManager()
{
}

bool TileManager::isTileOnPosition(eTileID tile, sf::Vector2f position, sf::Vector2i tileSize) const
{
	return false;
}

bool TileManager::isTileOnPosition(eTileID tile, sf::Vector2i position) const
{
	return false;
}

bool TileManager::isPositionCollidable(sf::Vector2f position, sf::Vector2i tileSize) const
{
	return false;
}

bool TileManager::isPositionCollidable(sf::Vector2i position) const
{
	return false;
}

void TileManager::removeTile(eTileID tile, sf::Vector2f position, sf::Vector2i tileSize)
{

}