#include "Texture.h"
#include <utility>
#include <assert.h>
#include "XMLParser.h"

std::unique_ptr<Texture> Texture::load(const std::string & levelName, const std::string& imageFileName)
{
	Texture* texture = new Texture();
	std::unique_ptr<Texture> uniqueTexture = std::unique_ptr<Texture>(texture);
	if (XMLParser::parseTextureDetails(uniqueTexture->m_tileSize, uniqueTexture->m_size, uniqueTexture->m_columns, levelName, imageFileName)
		 && texture->m_texture.loadFromFile(imageFileName + ".png"))
	{
		return uniqueTexture;
	}
	else
	{
		return std::unique_ptr<Texture>();
	}
}

const sf::Texture & Texture::getTexture() const
{
	return m_texture;
}

int Texture::getTileSize() const
{
	assert(m_tileSize.x == m_tileSize.y);
	return m_tileSize.x;
}

sf::IntRect Texture::getFrameRect(int tileID) const
{
	return sf::IntRect(tileID % m_columns * m_tileSize.x, tileID / m_columns * m_tileSize.x, m_tileSize.x, m_tileSize.y);
}