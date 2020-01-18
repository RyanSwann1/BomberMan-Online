#include "TileManager.h"
#include "TileID.h"
#include <assert.h>
#include "Texture.h"
#include "Resources.h"
#include "Utilities.h"

const std::string GAME_OBJECT_LAYER = "Game Object Layer";
const std::string GROUND_LAYER = "Ground Layer";

TileManager::TileManager()
	: m_collisionLayer(),
	m_tileLayers()
{}

const std::vector<std::vector<eCollidableTile>>& TileManager::getCollisionLayer() const
{
	return m_collisionLayer;
}

bool TileManager::isPositionAdjacentToBox(sf::Vector2i position) const
{
	for (sf::Vector2i direction : Utilities::getAllDirections())
	{
		sf::Vector2i adjacentPosition = position + direction;
		if (getTileLayer(GAME_OBJECT_LAYER).getTileID(adjacentPosition) == static_cast<int>(eTileID::eBox))
		{
			return true;
		}
	}

	return false;
}

void TileManager::removeTile(eTileID tileToRemove, sf::Vector2i position)
{
	getTileLayer(GAME_OBJECT_LAYER).removeTile(tileToRemove, position);

	if (m_collisionLayer[position.y][position.x] == eCollidableTile::eCollidableTile)
	{
		changeCollidableTile(position, eCollidableTile::eNonCollidable);
	}
}

void TileManager::changeTile(eTileID newTile, sf::Vector2i position)
{
	switch (newTile)
	{
	case eTileID::eBox:
	case eTileID::eWall:
	case eTileID::eBush:
		changeCollidableTile(position, eCollidableTile::eCollidableTile);
		getTileLayer(GAME_OBJECT_LAYER).changeTile(newTile, position);
		break;
	case eTileID::eBlank :
		changeCollidableTile(position, eCollidableTile::eNonCollidable);
		getTileLayer(GAME_OBJECT_LAYER).changeTile(newTile, position);
		break;
	case eTileID::eGrass:
	case eTileID::eDirt:
		changeCollidableTile(position, eCollidableTile::eNonCollidable);
		getTileLayer(GROUND_LAYER).changeTile(newTile, position);
		break;
	}
}

void TileManager::render(sf::RenderWindow& window, sf::Vector2i levelSize) const
{
	//Tile Layer
	for (const auto& tileLayer : m_tileLayers)
	{
		for (int y = 0; y < levelSize.y; ++y)
		{
			for (int x = 0; x < levelSize.x; ++x)
			{
				int tileID = tileLayer.m_tileLayer[y][x];
				if (tileID > 0)
				{
					const auto& tileSheet = Textures::getInstance().getTileSheet();
					sf::Sprite tileSprite(tileSheet.getTexture(), tileSheet.getFrameRect(tileID));
					tileSprite.setPosition(x * tileSheet.getTileSize().x, y * tileSheet.getTileSize().y);

					window.draw(tileSprite);
				}
			}
		}
	}
}

TileLayer& TileManager::getTileLayer(const std::string& name)
{
	auto tileLayer = std::find_if(m_tileLayers.begin(), m_tileLayers.end(), [name](const auto& tileLayer) { return tileLayer.m_name == name; });
	assert(tileLayer != m_tileLayers.end());

	return (*tileLayer);
}

void TileManager::changeCollidableTile(sf::Vector2i position, eCollidableTile newCollidableType)
{
	m_collisionLayer[position.y][position.x] = newCollidableType;
}

const TileLayer& TileManager::getTileLayer(const std::string& name) const
{
	auto tileLayer = std::find_if(m_tileLayers.cbegin(), m_tileLayers.cend(), [name](const auto& tileLayer) { return tileLayer.m_name == name; });
	assert(tileLayer != m_tileLayers.cend());

	return (*tileLayer);
}

bool TileManager::isTileOnPosition(eTileID tile, sf::Vector2i position) const
{
	switch (tile)
	{
	case eTileID::eBlank:
	case eTileID::eBox:
	case eTileID::eBush:
	case eTileID::eWall:
		return getTileLayer(GAME_OBJECT_LAYER).getTileID(position) == static_cast<int>(tile);
	case eTileID::eDirt:
	case eTileID::eGrass:
		return getTileLayer(GROUND_LAYER).getTileID(position) == static_cast<int>(tile);
	}
}

bool TileManager::isPositionCollidable(sf::Vector2i position) const
{
	return m_collisionLayer[position.y][position.x] == eCollidableTile::eCollidableTile;
}