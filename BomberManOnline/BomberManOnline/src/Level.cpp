#include "Level.h"
#include "XMLParser/XMLParser.h"
#include "Resources.h"

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

const std::vector<sf::Vector2i>& Level::getCollisionLayer() const
{
	return m_collisionLayer;
}

void Level::render(sf::RenderWindow & window) const
{
	const auto& tileSheet = Textures::getInstance().getTileSheet();
	for (const auto& tileLayer : m_tileLayers)
	{
		for (int y = 0; y < m_levelDimensions.y; ++y)
		{
			for (int x = 0; x < m_levelDimensions.x; ++x)
			{
				int tileID = tileLayer.m_tileLayer[y][x];
				if (tileID > 0)
				{
					sf::Sprite tileSprite(Textures::getInstance().getTileSheet().getTexture(), tileSheet.getFrameRect(tileID));
					tileSprite.setPosition(x * tileSheet.getTileSize(), y * tileSheet.getTileSize());

					window.draw(tileSprite);
				}
			}
		}
	}
}