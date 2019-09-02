#pragma once

#include "Player.h"
#include <SFML/Network.hpp>
#include <memory>
#include <utility>
#include <vector>

enum class eAIBehaviour
{
	ePassive = 0,
	eAggressive
};

enum class eAIState
{
	eIdle = 0,
	eMoveToBox,
	eMoveToPlayer,
	eMoveToSafePosition,
	ePlantBomb,
	eSetSafePosition,
	eSetTargetPosition,
	eWait
};

struct PlayerServerHuman : public Player
{
	PlayerServerHuman(std::unique_ptr<sf::TcpSocket> tcpSocket, int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
		: Player(ID, startingPosition, controllerType),
		m_tcpSocket(std::move(tcpSocket))
	{}
	~PlayerServerHuman() override;

	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
};

struct PlayerServerAI : public Player
{
	PlayerServerAI(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
		: Player(ID, startingPosition, controllerType),
		m_behavour(eAIBehaviour::ePassive),
		m_currentState(eAIState::eIdle),
		m_pathToTile(),
		m_waitTimer(2.5f)
	{}

	eAIBehaviour m_behavour;
	eAIState m_currentState;
	std::vector<sf::Vector2f> m_pathToTile;
	Timer m_waitTimer;
};

struct BombServer
{
	BombServer(sf::Vector2f startingPosition, float expirationTime)
		: m_position(startingPosition),
		m_lifeTime(expirationTime, true)
	{}

	sf::Vector2f m_position;
	Timer m_lifeTime;
};