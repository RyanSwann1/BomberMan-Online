#include "Level.h"
#include "XMLParser.h"

//Tile Layer
TileLayer::TileLayer(std::vector<std::vector<int>>&& tileLayer)
	: m_tileLayer(std::move(tileLayer))
{}

void TileLayer::render(sf::RenderWindow & window, sf::Vector2i levelDimensions) const
{
	for (int y = 0; y < levelDimensions.y; ++y)
	{
		for (int x = 0; x < levelDimensions.x; ++x)
		{
			int tileID = m_tileLayer[y][x];

		}
	}
}

//Level
std::unique_ptr<Level> Level::create(const std::string & levelName)
{
	Level level;
	if (XMLParser::loadMapAsClient(level.m_levelName, level.m_mapDimensions, level.m_tileLayers, level.m_collisionLayer, level.m_spawnPositions))
	{
		return std::make_unique<Level>(level);
	}
	
	return std::unique_ptr<Level>();
}

void Level::render(sf::RenderWindow & window)
{

}