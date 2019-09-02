#include "AnimatedSprite.h"
#include "Resources.h"
#include "Texture.h"
#include <assert.h>

AnimatedSprite::AnimatedSprite(sf::Vector2f startingPosition, eAnimationName animationName)
	: m_currentFrameID(Animations::getInstance().animations[static_cast<int>(animationName)].startFrameID),
	m_sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(m_currentFrameID)),
	m_animationName(animationName),
	m_animationTimer(0)	
{
	m_sprite.setPosition(startingPosition);
}

void AnimatedSprite::setPosition(sf::Vector2f position)
{
	if(Animations::getInstance().animations[static_cast<int>(m_animationName)].flipped)
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
		AnimationDetails animationDetails = Animations::getInstance().animations[static_cast<int>(m_animationName)];
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
	m_animationTimer.update(deltaTime);

	if (m_animationTimer.isExpired())
	{
		AnimationDetails animationDetails = Animations::getInstance().animations[static_cast<int>(m_animationName)];
		switch (animationDetails.direction)
		{
		case eDirection::eRight :
			if (m_currentFrameID + 1 < animationDetails.endFrameID)
			{
				++m_currentFrameID;
			}
			else
			{
				m_currentFrameID = animationDetails.startFrameID;
			}
			break;
		case eDirection::eDown :
			if (m_currentFrameID + 16 < animationDetails.endFrameID)
			{
				m_currentFrameID += 16;
			}
			else
			{
				m_currentFrameID = animationDetails.startFrameID;
			}
			break;
		case eDirection::eUp :
			if (m_currentFrameID - 16 < animationDetails.endFrameID)
			{
				m_currentFrameID -= 16;
			}
			else
			{
				m_currentFrameID = animationDetails.startFrameID;
			}
			break;
		}

		m_sprite.setTextureRect(Textures::getInstance().getTileSheet().getFrameRect(m_currentFrameID));
	}
}