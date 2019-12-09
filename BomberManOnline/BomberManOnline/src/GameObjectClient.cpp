#include "GameObjectClient.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "Resources.h"
#include "Utilities.h"
#include "Level.h"

//Game Object Client
GameObjectClient::GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName, eGameObjectType type, 
	eTimerActive timerActive)
	: GameObject(startingPosition, expirationTime, type, timerActive),
	m_sprite(startingPosition, startingAnimationName)
{}

void GameObjectClient::render(sf::RenderWindow & window) const
{
	m_sprite.render(window);
}

void GameObjectClient::update(float deltaTime)
{
	GameObject::update(deltaTime);

	m_sprite.setPosition(m_position);
	m_sprite.update(deltaTime);
}