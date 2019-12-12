#pragma once

#include "Player.h"
#include "GameObject.h"
#include <SFML/Network.hpp>
#include <memory>
#include <utility>
#include <vector>

enum class eAIBehaviour
{
	ePassive = 0, //Target boxes until non left, then Player
	eAggressive //Target Player when in sight
};

enum class eAIState
{
	eMakeDecision = 0,
	eSetTargetAtBox,
	eMoveToBox,
	eSetPositionToTargetPlayer,
	eMovingToTargetPlayer,
	eSetTargetAtSafePosition,
	eMoveToSafePosition,
	eMoveToPickUp,
	ePlantBomb,
	eWait
};

class Server;
class PlayerServer : public Player
{
public:
	PlayerServer(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType);

	void setNewPosition(sf::Vector2f newPosition, Server& server);
};

class PlayerServerAI : public PlayerServer
{
public:
	PlayerServerAI(int ID, sf::Vector2f startingPosition, Server& server);

	void update(float frameTime) override final;

private:
	Server& m_server;
	eAIBehaviour m_behavour;
	eAIState m_currentState;
	std::vector<sf::Vector2f> m_pathToTile;
	Timer m_waitTimer;
	int m_targetPlayerID;

	void handleAIStates(float frameTime);
	void destroyPlayer_FILLERNAME(const Player& targetPlayer);
	void onMovingToTargetPlayerState(const PlayerServer& targetPlayer);
};

class PlayerServerHuman : public PlayerServer
{
public:
	PlayerServerHuman(std::unique_ptr<sf::TcpSocket> tcpSocket, int ID, sf::Vector2f startingPosition, sf::SocketSelector& socketSelector);
	~PlayerServerHuman() override;
	
	std::unique_ptr<sf::TcpSocket>& getTCPSocket();

private:
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
};