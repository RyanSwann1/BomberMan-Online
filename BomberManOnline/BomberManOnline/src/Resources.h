#pragma once

#include "Texture.h"

class Textures : private NonCopyable
{
public:
	static Textures& getInstance()
	{
		static Textures instance;
		return instance;
	}

	const Texture& getTileSheet() const;
	bool loadAllTextures();

private:
	std::unique_ptr<Texture> m_tileSheet;
	bool m_loadedAllTextures = false;
};