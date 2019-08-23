#include "Resources.h"
#include <assert.h>
#include "XMLParser.h"
#include <iostream>

bool Textures::loadAllTextures()
{
	assert(!m_loadedAllTextures);
	m_tileSheet = Texture::load("level1.tmx", "tilesheet");
	if (m_tileSheet)
	{
		m_loadedAllTextures = true;
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