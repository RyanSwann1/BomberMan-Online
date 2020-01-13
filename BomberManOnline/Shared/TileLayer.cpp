#include "TileLayer.h"

TileLayer::TileLayer(std::vector<std::vector<int>>&& tileLayer, std::string&& name)
	: m_tileLayer(std::move(tileLayer)),
	m_name(std::move(name))
{}

eTileID TileLayer::getTile(sf::Vector2i position) const
{
	return static_cast<eTileID>(m_tileLayer[position.y][position.x]);
}

void TileLayer::changeTile(sf::Vector2i position, eTileID newTileID)
{
	m_tileLayer[position.y][position.x] = static_cast<int>(newTileID);
}