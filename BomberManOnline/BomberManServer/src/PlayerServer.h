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
	eMovingToBox,
	eMovingToTargetPlayer,
	eMovingToNearestPlayer,
	eMovingToSafePosition,
	eMovingToPickUp,
	eSetDestinationAtBox,
	eSetDestinationToTargetPlayer,
	eSetDestinationAtSafePosition,
	ePlantBomb,
	ePlantAndKickBomb,
	eWait
};

class Server;
class PlayerServer : public Player
{
public:
	PlayerServer(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType);

	void setNewPosition(sf::Vector2f newPosition, Server& server);
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