#pragma once

#include "Player.h"
#include "GameObject.h"
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
	eMakeDecision = 0,
	eMoveToBox,
	eMoveToNearestPlayer,
	eMoveToSafePosition,
	ePlantBomb,
	eSetPositionAtSafeArea,
	eWait
};

class Server;
class PlayerServerAI : public Player
{
public:
	PlayerServerAI(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType, Server& server);

	void update(float frameTime) override final;

private:
	Server& m_server;
	eAIBehaviour m_behavour;
	eAIState m_currentState;
	std::vector<sf::Vector2f> m_pathToTile;
	Timer m_waitTimer;
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