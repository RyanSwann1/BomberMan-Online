#include "TileManager.h"
#include "Resources.h"
#include "Texture.h"
#include <assert.h>

TileManager::TileManager()
	: m_collisionLayer(),
	m_tileLayers()
{}

const std::vector<std::vector<eCollidableTile>>& TileManager::getCollisionLayer() const
{
	return m_collisionLayer;
}

bool TileManager::isTileOnPosition(eTileID tile, sf::Vector2i position) const
{
	switch (tile)
	{
	case eTileID::eBox:
		return tile == getTileAtPosition("Box Layer", position);

	case eTileID::eWall:
		return tile == getTileAtPosition("Wall Layer", position);
	}

	return false;
}

bool TileManager::isPositionCollidable(sf::Vector2i position) const
{
	return m_collisionLayer[position.y][position.x] == eCollidableTile::eCollidable;
}

bool TileManager::isPositionAdjacentToBox(sf::Vector2i position) const
{
	const auto& boxTileLayer = getTileLayer("Box Layer");
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (boxTileLayer.getTile({ x, position.y }) == eTileID::eBox)
		{
			return true;
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (boxTileLayer.getTile({ position.x, y }) == eTileID::eBox)
		{
			return true;
		}
	}

	return false;
}

void TileManager::removeTile(eTileID tileToRemove, sf::Vector2i position)
{
	switch (tileToRemove)
	{
	case eTileID::eBox :
		getTileLayer("Box Layer").removeTile(position);
		m_collisionLayer[position.y][position.x] = eCollidableTile::eNonCollidable;
		break;

	case eTileID::eWall :
		getTileLayer("Wall Layer").removeTile(position);
		m_collisionLayer[position.y][position.x] = eCollidableTile::eNonCollidable;
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

eTileID TileManager::getTileAtPosition(const std::string& tileLayerName, sf::Vector2i position) const
{
	auto tileLayer = std::find_if(m_tileLayers.cbegin(), m_tileLayers.cend(), [tileLayerName](const auto& tileLayer) { return tileLayer.m_name == tileLayerName; });
	assert(tileLayer != m_tileLayers.cend());

	return tileLayer->getTile(position);
}

TileLayer& TileManager::getTileLayer(const std::string& name)
{
	auto tileLayer = std::find_if(m_tileLayers.begin(), m_tileLayers.end(), [name](const auto& tileLayer) { return tileLayer.m_name == name; });
	assert(tileLayer != m_tileLayers.end());
	return (*tileLayer);
}

const TileLayer& TileManager::getTileLayer(const std::string& name) const
{
	auto tileLayer = std::find_if(m_tileLayers.begin(), m_tileLayers.end(), [name](const auto& tileLayer) { return tileLayer.m_name == name; });
	assert(tileLayer != m_tileLayers.end());
	return (*tileLayer);
}