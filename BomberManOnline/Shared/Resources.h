#pragma once

#include "Texture.h"
#include "Direction.h"
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
	eBombStart = 266,
	eExplosionStart = 284,
	eExplosionEnd =	254
};

enum class eAnimationName
{
	ePlayerIdleUp = 0,
	ePlayerIdleDown,
	ePlayerIdleLeft,
	ePlayerIdleRight,
	ePlayerMoveUp,
	ePlayerMoveDown,
	ePlayerMoveRight,
	ePlayerMoveLeft,
	eBomb,
	eExplosion,
	eTotal
};

enum class eAnimationFlipped
{
	eFalse = 0,
	eTrue
};

struct AnimationDetails
{
	AnimationDetails(eDirection type, eFrameID startFrameID, eFrameID endFrameID, eAnimationFlipped flipped = eAnimationFlipped::eFalse);

	const eDirection direction;
	const int startFrameID;
	const int endFrameID;
	const bool flipped;
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
		//Player Idle
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveUpStart, eFrameID::ePlayerMoveUpStart),
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveDownStart, eFrameID::ePlayerMoveDownStart),
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveRightStart, eFrameID::ePlayerMoveRightStart),
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveLeftStart, eFrameID::ePlayerMoveLeftStart),
		//Player Move
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveUpStart, eFrameID::ePlayerMoveUpEnd),
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveDownStart, eFrameID::ePlayerMoveDownEnd),
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveRightStart, eFrameID::ePlayerMoveRightEnd),
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveLeftStart, eFrameID::ePlayerMoveLeftEnd, eAnimationFlipped::eTrue),
		//Bomb
		AnimationDetails(eDirection::eDown,  eFrameID::eBombStart, eFrameID::eBombEnd),
		//Explosion
		AnimationDetails(eDirection::eUp, eFrameID::eExplosionStart, eFrameID::eExplosionEnd)
	};

private:
	Animations() {}
};