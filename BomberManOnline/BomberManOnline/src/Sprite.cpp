#include "Sprite.h"
#include "Resources.h"
#include "Texture.h"
#include <assert.h>

Sprite::Sprite(sf::Vector2f startingPosition, eAnimationName animationName, float frameExpirationTime)
	: m_currentFrameID(Animations::getInstance().animations[static_cast<int>(animationName)].startFrameID),
	m_sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(m_currentFrameID)),
	m_animationName(animationName),
	m_animationTimer(frameExpirationTime)	
{
	m_sprite.setPosition(startingPosition);
}

void Sprite::setNewAnimation(eAnimationName newAnimationName)
{
	assert(m_animationName == newAnimationName);

	m_animationName = newAnimationName;
	m_currentFrameID = Animations::getInstance().animations[static_cast<int>(m_animationName)].startFrameID;
}

void Sprite::render(sf::RenderWindow & window) const
{
	window.draw(m_sprite);
}

void Sprite::update(float deltaTime)
{
	m_animationTimer.update(deltaTime);

	if (m_animationTimer.isExpired())
	{
		AnimationDetails animationDetails = Animations::getInstance().animations[static_cast<int>(m_animationName)];

		switch (animationDetails.type)
		{
		case eAnimationType::eHorizontal:
			updateHorizontalAnimation(animationDetails);
			break;
		case eAnimationType::eVertical:
			updateVerticalAnimation(animationDetails);
			break;
		}
	}
}

void Sprite::updateHorizontalAnimation(AnimationDetails animationDetails)
{
	if (m_currentFrameID + 1 < animationDetails.endFrameID)
	{
		++m_currentFrameID;
	}
	else
	{
		m_currentFrameID = animationDetails.startFrameID;
	}
}

void Sprite::updateVerticalAnimation(AnimationDetails animationDetails)
{
	if (m_currentFrameID + 16 < animationDetails.endFrameID)
	{
		m_currentFrameID += 16;
	}
	else
	{
		m_currentFrameID = animationDetails.startFrameID;
	}
}