#include "Resources.h"
#include <assert.h>
#include "XMLParser/XMLParser.h"
#include <iostream>

bool Textures::loadAllTextures()
{
	assert(!m_loadedAllTextures);
	m_tileSheet = Texture::load("level1.tmx", "tilesheet");
	if (m_tileSheet)
	{
		m_loadedAllTextures = true;
		return true;
	}
	else
	{
		std::cout << "Failed to load texture\n";
		return false;
	}
}

const Texture & Textures::getTileSheet() const
{
	assert(m_tileSheet);
	return *m_tileSheet;
}

//Animation Details
AnimationDetails::AnimationDetails(eAnimationType type, std::vector<eTileID>&& tileIDs)
	: type(type),
	tileIDs(std::move(tileIDs))
{}