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
	eBombStart = 236,
	eBombEnd = 266,
	eExplosionStart = 284,
	eExplosionEnd =	254,
	eMovementSpeedPickUp = 254,
	eExtraBombPickUp = 220,
	eBiggerExplosionPickUp = 284
};

enum class eAnimationName
{
	//Player Idle
	ePlayerIdleUp = 0,
	ePlayerIdleDown,
	ePlayerIdleLeft,
	ePlayerIdleRight,
	//Player Move
	ePlayerMoveUp,
	ePlayerMoveDown,
	ePlayerMoveRight,
	ePlayerMoveLeft,
	//Game Objects
	eBomb,
	eExplosion,
	eMovementSpeedPickUp,
	eExtraBombPickUp,
	eBiggerExplosionPickUp,
	eTotal
};

enum class eAnimationFlipped
{
	eFalse = 0,
	eTrue
};

enum class eAnimationRepeatable
{
	eFalse = 0,
	eTrue
};

struct AnimationDetails
{
	AnimationDetails(eDirection type, eFrameID startFrameID, eFrameID endFrameID, 
		eAnimationRepeatable repeatable = eAnimationRepeatable::eTrue, eAnimationFlipped flipped = eAnimationFlipped::eFalse);

	const eDirection direction;
	const int startFrameID;
	const int endFrameID;
	const bool flipped;
	const bool repeatable;
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

	const AnimationDetails& getAnimationDetails(eAnimationName animationName)
	{
		return animations[static_cast<int>(animationName)];
	}

private:
	Animations() {}

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
		AnimationDetails(eDirection::eRight, eFrameID::ePlayerMoveLeftStart, eFrameID::ePlayerMoveLeftEnd, eAnimationRepeatable::eTrue, eAnimationFlipped::eTrue),
		//Game Objects
		AnimationDetails(eDirection::eDown,  eFrameID::eBombStart, eFrameID::eBombEnd, eAnimationRepeatable::eFalse),
		AnimationDetails(eDirection::eUp, eFrameID::eExplosionStart, eFrameID::eExplosionEnd, eAnimationRepeatable::eFalse),
		AnimationDetails(eDirection::eNone, eFrameID::eMovementSpeedPickUp, eFrameID::eMovementSpeedPickUp),
		AnimationDetails(eDirection::eNone, eFrameID::eExtraBombPickUp, eFrameID::eExtraBombPickUp),
		AnimationDetails(eDirection::eNone, eFrameID::eBiggerExplosionPickUp, eFrameID::eBiggerExplosionPickUp)
	};
};