#include "Level.h"
#include "XMLParser.h"

//Tile Layer
TileLayer::TileLayer(std::vector<std::vector<int>>&& tileLayer)
	: m_tileLayer(std::move(tileLayer))
{}

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

