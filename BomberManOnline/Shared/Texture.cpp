#include "Texture.h"
#include <utility>
#include <assert.h>
#include "XMLParser/XMLParser.h"

Texture::Texture()
	: m_texture(),
	m_tileSize(),
	m_size(),
	m_columns(0)
{}

std::unique_ptr<Texture> Texture::load(const std::string & levelName, const std::string& imageFileName)
{
	Texture* texture = new Texture();
	if (XMLParser::parseTextureDetails(texture->m_tileSize, texture->m_size, texture->m_columns, levelName, imageFileName)
		 && texture->m_texture.loadFromFile(imageFileName + ".png"))
	{
		return std::unique_ptr<Texture>(texture);
	}
	else
	{
		delete texture;
		return std::unique_ptr<Texture>();
	}
}

const sf::Texture & Texture::getTexture() const
{
	return m_texture;
}

sf::Vector2i Texture::getTileSize() const
{
	return m_tileSize;
}

sf::IntRect Texture::getFrameRect(int tileID) const
{
	return sf::IntRect(tileID % m_columns * m_tileSize.x, tileID / m_columns * m_tileSize.x, m_tileSize.x, m_tileSize.y);
}