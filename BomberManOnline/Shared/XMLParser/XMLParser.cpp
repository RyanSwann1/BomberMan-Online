#include "XMLParser.h"
#include "Base64.h"
#include "tinyxml.h"
#include <assert.h>
#include "FrameDetails.h"
#include "TileLayer.h"

std::vector<TileLayer> parseTileLayers(const TiXmlElement& rootElement, const sf::Vector2i mapSize);
sf::Vector2i parseMapSize(const TiXmlElement& rootElement);
sf::Vector2i parseTileSize(const TiXmlElement& rootElement);
std::vector<std::vector<int>> decodeTileLayer(const TiXmlElement & tileLayerElement, sf::Vector2i mapSize);
std::vector<sf::Vector2f> parseObjectLayer(const TiXmlElement & rootElement, sf::Vector2i tileSize, const std::string& layerName);

bool XMLParser::parseTextureDetails(sf::Vector2i& tileSize, sf::Vector2i& textureSize, int& columns, const std::string& levelFileName, const std::string& textureFileName)
{
	TiXmlDocument file;
	bool fileLoaded = file.LoadFile(levelFileName);
	assert(fileLoaded);
	bool textureDetailsFound = false;

	const auto& rootElement = file.RootElement();
	for (const auto* tileSheetElement = rootElement->FirstChildElement();
		tileSheetElement != nullptr; tileSheetElement = tileSheetElement->NextSiblingElement())
	{
		if (tileSheetElement->Value() != std::string("tileset"))
		{
			continue;
		}

		if (tileSheetElement->Attribute("name") == textureFileName)
		{
			tileSheetElement->FirstChildElement()->Attribute("width", &textureSize.x);
			tileSheetElement->FirstChildElement()->Attribute("height", &textureSize.y);
			tileSheetElement->Attribute("tilewidth", &tileSize.x);
			tileSheetElement->Attribute("tileheight", &tileSize.y);
			columns = textureSize.x / tileSize.x;
			
			textureDetailsFound = true;
		}
	}

	return textureDetailsFound;
}

bool XMLParser::loadMapAsClient(const std::string & mapName, sf::Vector2i & mapDimensions, std::vector<TileLayer>& tileLayers, 
	std::vector<sf::Vector2f>& collisionLayer, std::vector<sf::Vector2f>& spawnPositions, std::vector<sf::Vector2f>& boxes)
{
	TiXmlDocument xmlFile;
	if (!xmlFile.LoadFile(mapName))
	{
		return false;
	}

	const auto& rootElement = xmlFile.RootElement();
	mapDimensions = parseMapSize(*rootElement);
	tileLayers = parseTileLayers(*rootElement, mapDimensions);
	spawnPositions = parseObjectLayer(*rootElement, parseTileSize(*rootElement), "Spawn Position Layer");
	collisionLayer = parseObjectLayer(*rootElement, parseTileSize(*rootElement), "Collision Layer");
	boxes = parseObjectLayer(*rootElement, parseTileSize(*rootElement), "Box Layer");

	return true;
}

bool XMLParser::loadMapAsServer(const std::string & mapName, sf::Vector2i & mapDimensions, std::vector<sf::Vector2f>& collisionLayer, 
	std::vector<sf::Vector2f>& spawnPositions, std::vector<sf::Vector2f>& boxes)
{
	TiXmlDocument xmlFile;
	if (!xmlFile.LoadFile(mapName))
	{
		return false;
	}

	const auto& rootElement = xmlFile.RootElement();
	mapDimensions = parseMapSize(*rootElement);
	spawnPositions = parseObjectLayer(*rootElement, parseTileSize(*rootElement), "Spawn Position Layer");
	collisionLayer = parseObjectLayer(*rootElement, parseTileSize(*rootElement), "Collision Layer");
	boxes = parseObjectLayer(*rootElement, parseTileSize(*rootElement), "Box Layer");

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

std::vector<sf::Vector2f> parseObjectLayer(const TiXmlElement & rootElement, sf::Vector2i tileSize, const std::string & layerName)
{
	std::vector<sf::Vector2f> objects;
	for (const auto* entityElementRoot = rootElement.FirstChildElement(); entityElementRoot != nullptr; entityElementRoot = entityElementRoot->NextSiblingElement())
	{
		if (entityElementRoot->Value() != std::string("objectgroup") || entityElementRoot->Attribute("name") != std::string(layerName))
		{
			continue;
		}

		for (const auto* entityElement = entityElementRoot->FirstChildElement(); entityElement != nullptr; entityElement = entityElement->NextSiblingElement())
		{
			sf::Vector2i spawnPosition;
			entityElement->Attribute("x", &spawnPosition.x);
			entityElement->Attribute("y", &spawnPosition.y);
			spawnPosition.y -= tileSize.y; //Tiled Hack
			objects.emplace_back(sf::Vector2f(static_cast<float>(spawnPosition.x), static_cast<float>(spawnPosition.y)));
		}
	}

	assert(!objects.empty());
	return objects;
}