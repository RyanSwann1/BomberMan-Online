#pragma once

#include <memory>
#include "Texture.h"

class Textures
{
public:
	static Textures& getInstance()
	{
		static Textures instance;
		return instance;
	}

	bool loadAllTextures();

	const Texture& getTileSheet() const;

private:
	std::unique_ptr<Texture> m_tileSheet;
	bool m_loadedAllTextures = false;
};