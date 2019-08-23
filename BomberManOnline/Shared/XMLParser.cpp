#include "XMLParser.h"
#include "Base64.h"
#include "tinyxml.h"
#include <assert.h>
#include "Level.h"

std::vector<TileLayer> parseTileLayers(const TiXmlElement& rootElement, const sf::Vector2i mapSize);
sf::Vector2i parseMapSize(const TiXmlElement& rootElement);
sf::Vector2i parseTileSize(const TiXmlElement& rootElement);
std::vector<sf::Vector2i> parseCollisionLayer(const TiXmlElement& rootElement, int tileSize);

std::vector<std::vector<int>> decodeTileLayer(const TiXmlElement & tileLayerElement, sf::Vector2i mapSize);
std::vector<sf::Vector2i> parsePlayerSpawnPositions(const TiXmlElement & rootElement, sf::Vector2i tileSize);

bool XMLParser::loadTextureDetails(const std::string& filePath, std::string& imagePath, std::vector<FrameDetails>& frames)
{
	TiXmlDocument xmlFile;
	if (!xmlFile.LoadFile(filePath))
	{
		return false;
	}

	const auto& rootElement = xmlFile.RootElement();
	imagePath = rootElement->Attribute("imagePath");

	int i = 0; //Acts as the frame ID for each iteration
	for (const TiXmlElement* e = rootElement->FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
	{
		int height = 0;
		e->Attribute("height", &height);
		int width = 0;
		e->Attribute("width", &width);
		int y = 0;
		e->Attribute("y", &y);
		int x = 0;
		e->Attribute("x", &x);
		int frameID = i;
		++i;
		
		frames.emplace_back(height, width, y, x, frameID);	
	}

	return true;
}

bool XMLParser::loadMapAsClient(const std::string & mapName, sf::Vector2i & mapDimensions, std::vector<TileLayer>& tileLayers, 
	std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions)
{
	TiXmlDocument xmlFile;
	if (!xmlFile.LoadFile(mapName))
	{
		return false;
	}

	const auto& rootElement = xmlFile.RootElement();
	mapDimensions = parseMapSize(*rootElement);
	tileLayers = parseTileLayers(*rootElement, mapDimensions);
	spawnPositions = parsePlayerSpawnPositions(*rootElement, parseTileSize(*rootElement));
	collisionLayer = parseCollisionLayer(*rootElement, parseTileSize(*rootElement).x);

	return true;
}

bool XMLParser::loadMapAsServer(const std::string & mapName, sf::Vector2i & mapDimensions, std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions)
{
	TiXmlDocument xmlFile;
	if (!xmlFile.LoadFile(mapName))
	{
		return false;
	}

	const auto& rootElement = xmlFile.RootElement();
	mapDimensions = parseMapSize(*rootElement);
	spawnPositions = parsePlayerSpawnPositions(*rootElement, parseTileSize(*rootElement));
	collisionLayer = parseCollisionLayer(*rootElement, parseTileSize(*rootElement).x);

	return true;
}

std::vector<std::vector<int>> decodeTileLayer(const TiXmlElement & tileLayerElement, sf::Vector2i mapSize)
{
	std::vector<std::vector<int>> tileData;
	tileData.reserve(mapSize.y);

	std::string decodedIDs; //Base64 decoded information
	const TiXmlElement* dataNode = nullptr; //Store our node once we find it
	for (const TiXmlElement* e = tileLayerElement.FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
	{
		if (e->Value() == std::string("data"))
		{
			dataNode = e;
		}
	}
	assert(dataNode);

	Base64 base64;
	const TiXmlText* text = dataNode->FirstChild()->ToText();
	const std::string t = text->Value();
	decodedIDs = base64.base64_decode(t);

	const std::vector<int> layerColumns(mapSize.x);
	for (int i = 0; i < mapSize.y; ++i)
	{
		tileData.push_back(layerColumns);
	}

	for (int rows = 0; rows < mapSize.y; ++rows)
	{
		for (int cols = 0; cols < mapSize.x; ++cols)
		{
			tileData[rows][cols] = *((int*)decodedIDs.data() + rows * mapSize.x + cols) - 1;
		}
	}

	return tileData;
}

std::vector<TileLayer> parseTileLayers(const TiXmlElement & rootElement, const sf::Vector2i mapSize)
{
	std::vector<TileLayer> tileLayers;
	for (const auto* tileLayerElement = rootElement.FirstChildElement();
		tileLayerElement != nullptr; tileLayerElement = tileLayerElement->NextSiblingElement())
	{
		if (tileLayerElement->Value() != std::string("layer"))
		{
			continue;
		}


		tileLayers.emplace_back(std::move(decodeTileLayer(*tileLayerElement, mapSize)));
	}

	assert(!tileLayers.empty());
	return tileLayers;
}

sf::Vector2i parseMapSize(const TiXmlElement & rootElement)
{
	sf::Vector2i mapSize(0, 0);
	rootElement.Attribute("width", &mapSize.x);
	rootElement.Attribute("height", &mapSize.y);
	assert(mapSize.x != 0 && mapSize.y != 0);
	return mapSize;
}

sf::Vector2i parseTileSize(const TiXmlElement & rootElement)
{
	sf::Vector2i tileSize(0, 0);
	rootElement.Attribute("tilewidth", &tileSize.x);
	rootElement.Attribute("tileheight", &tileSize.y);
	assert(tileSize.x != 0 && tileSize.y != 0);
	return tileSize;
}

std::vector<sf::Vector2i> parsePlayerSpawnPositions(const TiXmlElement & rootElement, sf::Vector2i tileSize)
{
	std::vector<sf::Vector2i> factionSpawnPositions;
	for (const auto* entityElementRoot = rootElement.FirstChildElement(); entityElementRoot != nullptr; entityElementRoot = entityElementRoot->NextSiblingElement())
	{
		if (entityElementRoot->Value() != std::string("objectgroup") || entityElementRoot->Attribute("name") != std::string("SpawnPositionLayer"))
		{
			continue;
		}

		for (const auto* entityElement = entityElementRoot->FirstChildElement(); entityElement != nullptr; entityElement = entityElement->NextSiblingElement())
		{
			sf::Vector2i spawnPosition;
			entityElement->Attribute("x", &spawnPosition.x);
			entityElement->Attribute("y", &spawnPosition.y);
			//startingPosition.y -= tileSize; //Tiled Hack
			spawnPosition.x /= 24;
			spawnPosition.y /= 28;
			factionSpawnPositions.emplace_back(spawnPosition);
		}
	}

	assert(!factionSpawnPositions.empty());
	return factionSpawnPositions;
}

std::vector<sf::Vector2i> parseCollisionLayer(const TiXmlElement & rootElement, int tileSize)
{
	std::vector<sf::Vector2i> collidablePositions;
	for (const auto* collisionLayerElement = rootElement.FirstChildElement(); collisionLayerElement != nullptr;
		collisionLayerElement = collisionLayerElement->NextSiblingElement())
	{
		if (collisionLayerElement->Value() != std::string("objectgroup"))
		{
			continue;
		}

		if (collisionLayerElement->Attribute("name") != std::string("Collision Layer"))
		{
			continue;
		}

		for (const auto* collisionElement = collisionLayerElement->FirstChildElement();
			collisionElement != nullptr; collisionElement = collisionElement->NextSiblingElement())
		{
			int xPosition = 0, yPosition = 0;
			collisionElement->Attribute("x", &xPosition);
			collisionElement->Attribute("y", &yPosition);
			//Hack for Tiled.
			yPosition -= tileSize;
			collidablePositions.emplace_back(xPosition, yPosition);
		}
	}

	assert(!collidablePositions.empty());
	return collidablePositions;
}