#include "Resources.h"
#include <assert.h>
#include "XMLParser/XMLParser.h"
#include <iostream>

//Textures
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
AnimationDetails::AnimationDetails(eDirection direction, eFrameID startFrameID, eFrameID endFrameID, 
	eAnimationRepeatable repeatable, eAnimationFlipped flipped)
	: direction(direction),
	startFrameID(static_cast<int>(startFrameID)),
	endFrameID(static_cast<int>(endFrameID)),
	flipped(static_cast<bool>(flipped)),
	repeatable(static_cast<bool>(repeatable))
{}