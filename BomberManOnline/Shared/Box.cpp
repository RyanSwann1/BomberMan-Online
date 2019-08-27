#include "Box.h"
#include "Resources.h"

//Box
Box::Box(sf::Vector2f startingPosition)
	: position(startingPosition),
	sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(204))
{
	sprite.setPosition(startingPosition);
}