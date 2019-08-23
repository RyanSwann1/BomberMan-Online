#include "Level.h"
#include "XMLParser.h"
#include "Resources.h"

//Tile Layer
TileLayer::TileLayer(std::vector<std::vector<int>>&& tileLayer)
	: m_tileLayer(std::move(tileLayer))
{}

void TileLayer::render(sf::RenderWindow & window, sf::Vector2i levelDimensions) const
{
	auto& tileSheet = Textures::getInstance().getTileSheet();

	for (int y = 0; y < levelDimensions.y; ++y)
	{
		for (int x = 0; x < levelDimensions.x; ++x)
		{
			int tileID = m_tileLayer[y][x];
			if (tileID > 0)
			{
				sf::Sprite tileSprite(tileSheet.getTexture(), tileSheet.getFrameRect(tileID));
				tileSprite.setPosition(x * tileSheet.getTileSize(), y * tileSheet.getTileSize());
				window.draw(tileSprite);
			}
		}
	}
}

//Level
std::unique_ptr<Level> Level::create(const std::string & levelName)
{
	Level* level = new Level;
	std::unique_ptr<Level> uniqueLevel = std::unique_ptr<Level>(level);
	uniqueLevel->m_levelName = levelName;
	if (XMLParser::loadMapAsClient(uniqueLevel->m_levelName, level->m_levelDimensions, level->m_tileLayers, level->m_collisionLayer, level->m_spawnPositions))
	{
		return uniqueLevel;
	}
	else
	{
		return std::unique_ptr<Level>();
	}
}

void Level::render(sf::RenderWindow & window)
{
	for (const auto& tileLayer : m_tileLayers)
	{
		tileLayer.render(window, m_levelDimensions);
	}
}