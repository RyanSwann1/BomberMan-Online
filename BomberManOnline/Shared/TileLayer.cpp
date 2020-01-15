#include "TileLayer.h"
#include <assert.h>
#include "TileID.h"
#include <iostream>

TileLayer::TileLayer(std::vector<std::vector<int>>&& tileLayer, std::string&& name)
	: m_tileLayer(std::move(tileLayer)),
	m_name(std::move(name))
{}

int TileLayer::getTileID(sf::Vector2i position) const
{
	return m_tileLayer[position.y][position.x];
}

void TileLayer::removeTile(eTileID tileToRemove, sf::Vector2i position)
{
	assert(m_tileLayer[position.y][position.x] == static_cast<int>(tileToRemove));
	std::cout << m_tileLayer[position.y][position.x] << "\n";
	m_tileLayer[position.y][position.x] = static_cast<int>(eTileID::eBlank);
}