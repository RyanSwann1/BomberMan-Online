#pragma once

#include "NonCopyable.h"
#include "FrameDetails.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class Texture : private NonCopyable
{
public:	
	static std::unique_ptr<Texture> load(const std::string& levelName, const std::string& imageFileName);

	const sf::Texture& getTexture() const;

	sf::IntRect getFrameRect(int tileID) const;

private:
	Texture() {}
	sf::Texture m_texture;
	sf::Vector2i m_tileSize;
	sf::Vector2i m_size;
	int m_columns;
};