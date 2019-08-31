#pragma once

#include "Texture.h"
#include "Sprite.h"
#include <array>
#include <vector>

enum class eAnimationName
{
	ePlayerMoveUp = 0,
	ePlayerMoveDown,
	ePlayerMoveRight,
	ePlayerMoveLeft,
	eBomb,
	eTotal
};

struct AnimationDetails
{
	AnimationDetails(eAnimationType type, std::vector<eTileID>&& tileIDs);

	const eAnimationType type;
	const std::vector<eTileID> tileIDs;
};

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

struct Animations
{
	Animations() {}

	const std::array<AnimationDetails, static_cast<size_t>(eAnimationName::eTotal)> animations
	{
		AnimationDetails(eAnimationType::eHorizontal, { eTileID::ePlayerMoveUpStart, eTileID::ePlayerMoveUpEnd }),
		AnimationDetails(eAnimationType::eHorizontal, { eTileID::ePlayerMoveDownStart, eTileID::ePlayerMoveDownEnd }),
		AnimationDetails(eAnimationType::eHorizontal, { eTileID::ePlayerMoveRightStart, eTileID::ePlayerMoveRightEnd }),
		AnimationDetails(eAnimationType::eHorizontal, { eTileID::ePlayerMoveLeftStart, eTileID::ePlayerMoveLeftEnd }),
		AnimationDetails(eAnimationType::eVertical, { eTileID::eBombStart, eTileID::eBombEnd })
	};
};