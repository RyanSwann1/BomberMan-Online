#pragma once

#include "Player.h"
#include "GameObject.h"
#include <SFML/Network.hpp>
#include <memory>
#include <utility>
#include <vector>

//TWO ISSUES
//1. eAIState:eMovingToTargetPlayerSafePath
//Don't adjust the path when moving to the player - temporary fix
//2. PathFinding::getInstance().getPathToClosestSafePosition(m_position, m_pathToTile, m_server);
//Can sometimes result in an empty path
//this shouldn't happen
//If happens, get selection of potential positions to move to
//Choose the furthest one away from the target player to move to

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
	eMoveToNearestPlayer,
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
	void onSetPositionToTargetPlayerState(const PlayerServer& targetPlayer);

#ifdef RENDER_PATHING
	void handleRenderPathing();
#endif // RENDER_PATHING
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