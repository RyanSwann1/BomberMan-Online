#include "Resources.h"
#include <assert.h>

bool Textures::loadAllTextures()
{
	return false;
}

const Texture & Textures::getTileSheet() const
{
	assert(m_tileSheet);
	return *m_tileSheet;
}
