#pragma once

#include <SFML/Graphics.hpp>
#include "Resources.h"
#include "Timer.h"

class AnimatedSprite
{
public:
	AnimatedSprite(sf::Vector2f startingPosition, eAnimationName animationName, float frameExpirationTime);

	void setPosition(sf::Vector2f position);
	void setNewAnimation(eAnimationName newAnimationName);
	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);

private:
	int m_currentFrameID;
	sf::Sprite m_sprite;
	eAnimationName m_animationName;
	Timer m_animationTimer;
	
	void updateHorizontalAnimation(AnimationDetails animationDetails);
	void updateVerticalAnimation(AnimationDetails animationDetails);
};