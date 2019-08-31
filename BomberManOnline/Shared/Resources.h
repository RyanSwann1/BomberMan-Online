#pragma once

#include "Texture.h"
#include <array>
#include <vector>

enum class eFrameID
{
	ePlayerMoveUpStart = 263,
	ePlayerMoveUpEnd = 264,
	ePlayerMoveDownStart = 256,
	ePlayerMoveDownEnd = 258,
	ePlayerMoveRightStart = 259,
	ePlayerMoveRightEnd = 262,
	ePlayerMoveLeftStart = 259,
	ePlayerMoveLeftEnd = 262,
	eBombEnd = 236,
	eBombStart = 266
};

enum class eAnimationType
{
	eHorizontal = 0,
	eVertical
};

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
	AnimationDetails(eAnimationType type, eFrameID startFrameID, eFrameID endFrameID);

	const eAnimationType type;
	const int startFrameID;
	const int endFrameID;
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

class Animations : NonCopyable
{
public:
	static Animations& getInstance()
	{
		static Animations instance;
		return instance;
	}

	const std::array<AnimationDetails, static_cast<size_t>(eAnimationName::eTotal)> animations
	{
		AnimationDetails(eAnimationType::eHorizontal, eFrameID::ePlayerMoveUpStart, eFrameID::ePlayerMoveUpEnd),
		AnimationDetails(eAnimationType::eHorizontal, eFrameID::ePlayerMoveDownStart, eFrameID::ePlayerMoveDownEnd),
		AnimationDetails(eAnimationType::eHorizontal, eFrameID::ePlayerMoveRightStart, eFrameID::ePlayerMoveRightEnd),
		AnimationDetails(eAnimationType::eHorizontal, eFrameID::ePlayerMoveLeftStart, eFrameID::ePlayerMoveLeftEnd),
		AnimationDetails(eAnimationType::eVertical,  eFrameID::eBombStart, eFrameID::eBombEnd)
	};

private:
	Animations() {}
};