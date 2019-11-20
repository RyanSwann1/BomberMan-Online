#pragma once

#include <SFML/Graphics.hpp>
#include "Resources.h"
#include "Timer.h"

class AnimatedSprite
{
public:
	AnimatedSprite(sf::Vector2f startingPosition, eAnimationName animationName);

	void setPosition(sf::Vector2f position);
	void setNewAnimation(eAnimationName newAnimationName);
	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);

private:
	int m_currentFrameID;
	bool m_animationFinished;
	sf::Sprite m_sprite;
	eAnimationName m_animationName;
	Timer m_frameTimer;
};