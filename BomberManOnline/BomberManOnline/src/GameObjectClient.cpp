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
	: GameObject(startingPosition, expirationTime, type),
	m_sprite(startingPosition, startingAnimationName)
{}

void GameObjectClient::render(sf::RenderWindow & window) const
{
	m_sprite.render(window);
}

void GameObjectClient::update(float deltaTime)
{
	GameObject::update(deltaTime);
	m_sprite.update(deltaTime);
}

//Bomb Client
BombClient::BombClient(sf::Vector2f startingPosition, int explosionRange)
	: GameObjectClient(startingPosition, BOMB_LIFETIME_DURATION, eAnimationName::eBiggerExplosionPickUp, eGameObjectType::eBiggerExplosionPickUp, 
		eTimerActive::eTrue),
	m_explosionSize(explosionRange)
{}

int BombClient::getExplosionSize() const
{
	return m_explosionSize;
}