#include "AnimatedSprite.h"
#include "GameObjectClient.h"
#include "Resources.h"
#include "Texture.h"
#include <assert.h>

constexpr float TOTAL_BOMB_ANIMATIONS = 3.0f;
constexpr float TOTAL_EXPLOSION_ANIMATIONS = 3.0f;

AnimatedSprite::AnimatedSprite(sf::Vector2f startingPosition, eAnimationName animationName)
	: m_currentFrameID(Animations::getInstance().getAnimationDetails(animationName).startFrameID),
	m_animationFinished(false),
	m_sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(m_currentFrameID)),
	m_animationName(animationName),
	m_frameTimer(0.0f, eTimerActive::eTrue)	
{
	m_sprite.setPosition(startingPosition);

	switch (animationName)
	{
	case eAnimationName::ePlayerIdleDown :
	case eAnimationName::ePlayerIdleLeft :
	case eAnimationName::ePlayerIdleRight :
	case eAnimationName::ePlayerIdleUp :
	case eAnimationName::eMovementSpeedPickUp : 
	case eAnimationName::eExtraBombPickUp :
	case eAnimationName::eBiggerExplosionPickUp :
		m_frameTimer.setExpiredTime(0.0f);
		break;
	case eAnimationName::ePlayerMoveLeft :
	case eAnimationName::ePlayerMoveRight :
	case eAnimationName::ePlayerMoveUp :
	case eAnimationName::ePlayerMoveDown :
		m_frameTimer.setExpiredTime(0.25f);
		break;
	case eAnimationName::eBomb :
		m_frameTimer.setExpiredTime(BOMB_LIFETIME_DURATION / TOTAL_BOMB_ANIMATIONS);
		break;
	case eAnimationName::eExplosion :
		m_frameTimer.setExpiredTime(EXPLOSION_LIFETIME_DURATION / TOTAL_EXPLOSION_ANIMATIONS);
		break;
	}
}

void AnimatedSprite::setPosition(sf::Vector2f position)
{
	if(Animations::getInstance().getAnimationDetails(m_animationName).flipped)
	{ 
		position.x += 16;
		m_sprite.setPosition(position);
	}
	else
	{
		m_sprite.setPosition(position);
	}
}

void AnimatedSprite::setNewAnimation(eAnimationName newAnimationName)
{
	if (m_animationName != newAnimationName)
	{
		m_animationName = newAnimationName;
		const AnimationDetails& animationDetails = Animations::getInstance().getAnimationDetails(m_animationName);
		m_currentFrameID = animationDetails.startFrameID;
		m_sprite.setTextureRect(Textures::getInstance().getTileSheet().getFrameRect(m_currentFrameID));
		if (animationDetails.flipped)
		{
			m_sprite.setScale(-1.f, 1.f);
		}
		else
		{
			m_sprite.setScale(1.f, 1.f);
		}
	}
}

void AnimatedSprite::render(sf::RenderWindow & window) const
{
	window.draw(m_sprite);
}

void AnimatedSprite::update(float deltaTime)
{
	m_frameTimer.update(deltaTime);

	if (!m_animationFinished && m_frameTimer.isExpired())
	{
		m_frameTimer.resetElaspedTime();

		const AnimationDetails& animationDetails = Animations::getInstance().getAnimationDetails(m_animationName);
		switch (animationDetails.direction)
		{
		case eDirection::eRight:
			if (m_currentFrameID + 1 <= animationDetails.endFrameID)
			{
				++m_currentFrameID;
			}
			else
			{
				if (!animationDetails.repeatable)
				{
					m_animationFinished = true;
				}

				m_currentFrameID = animationDetails.startFrameID;
			}
			break;
		case eDirection::eDown:
			if (m_currentFrameID + 15 <= animationDetails.endFrameID)
			{
				m_currentFrameID += 15;
			}
			else
			{
				if (!animationDetails.repeatable)
				{
					m_animationFinished = true;
				}

				m_currentFrameID = animationDetails.startFrameID;
			}
			break;
		case eDirection::eUp:
			if (m_currentFrameID - 15 >= animationDetails.endFrameID)
			{
				m_currentFrameID -= 15;
			}
			else
			{
				if (!animationDetails.repeatable)
				{
					m_animationFinished = true;
				}

				m_currentFrameID = animationDetails.startFrameID;
			}
			break;
		}

		m_sprite.setTextureRect(Textures::getInstance().getTileSheet().getFrameRect(m_currentFrameID));
	}
}