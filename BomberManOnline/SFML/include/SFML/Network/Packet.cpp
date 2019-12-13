#include "Packet.hpp"
#include "ServerMessages.h"
#include <string>

sf::Packet& sf::operator>>(Packet& receivedPacket, std::vector<sf::Vector2f>& path)
{
	int pathSize = 0;
	receivedPacket >> pathSize;
	for (int i = 0; i < pathSize; ++i)
	{
		sf::Vector2f position;
		receivedPacket >> position;
		path.push_back(position);
	}

	return receivedPacket;
}

sf::Packet& sf::operator<<(Packet& packetToSend, const std::vector<sf::Vector2f>& path)
{
	packetToSend << static_cast<int>(path.size());

	for (sf::Vector2f i : path)
	{
		packetToSend << i;
	}

	return packetToSend;
}

sf::Packet& sf::operator>>(Packet& receivedPacket, eDirection& direction)
{
	int dir = 0;
	receivedPacket >> dir;
	direction = static_cast<eDirection>(dir);

	return receivedPacket;
}

sf::Packet& sf::operator<<(Packet& packetToSend, eDirection direction)
{
	packetToSend << static_cast<int>(direction);
	return packetToSend;
}

sf::Packet& sf::operator>>(Packet& receivedPacket, sf::Vector2f& position)
{
	receivedPacket >> position.x >> position.y;
	return receivedPacket;
}

sf::Packet& sf::operator<<(Packet& packetToSend, sf::Vector2f position)
{
	packetToSend << position.x << position.y;
	return packetToSend;
}

sf::Packet & sf::operator>>(Packet & receivedPacket, eServerMessageType & serverMessage)
{
	int messageType = 0;
	receivedPacket >> messageType;
	serverMessage = static_cast<eServerMessageType>(messageType);
	
	return receivedPacket;
}

sf::Packet & sf::operator<<(Packet & packetToSend, eServerMessageType serverMessage)
{
	packetToSend << static_cast<int>(serverMessage);
	return packetToSend;
}

sf::Packet & sf::operator>>(Packet & receivedPacket, ServerMessageInvalidMove & invalidMoveMessage)
{
	receivedPacket >> invalidMoveMessage.invalidPosition.x >> invalidMoveMessage.invalidPosition.y >>
		invalidMoveMessage.lastValidPosition.x >> invalidMoveMessage.lastValidPosition.y;

	return receivedPacket;
}

sf::Packet & sf::operator<<(Packet & packetToSend, ServerMessageInvalidMove invalidMoveMessage)
{
	packetToSend << invalidMoveMessage.invalidPosition.x << invalidMoveMessage.invalidPosition.y <<
		invalidMoveMessage.lastValidPosition.x << invalidMoveMessage.lastValidPosition.y;

	return packetToSend;
}

sf::Packet & sf::operator>>(Packet & receivedPacket, ServerMessageInitialGameData & initialGameData)
{
	receivedPacket >> initialGameData.levelName;
	int totalPlayerDetails = 0;
	receivedPacket >> totalPlayerDetails;
	for (int i = 0; i < totalPlayerDetails; ++i)
	{
		int ID = 0;
		receivedPacket >> ID;
		sf::Vector2f spawnPosition;
		receivedPacket >> spawnPosition.x >> spawnPosition.y;


		initialGameData.playerDetails.emplace_back(ID, spawnPosition);
	}
	
	return receivedPacket;
}

sf::Packet & sf::operator<<(Packet & packetToSend, const ServerMessageInitialGameData & initialGameData)
{
	packetToSend << initialGameData.levelName;
 	packetToSend << static_cast<int>(initialGameData.playerDetails.size());

	for (auto& playerDetails : initialGameData.playerDetails)
	{
		packetToSend << playerDetails.ID;
		packetToSend << playerDetails.spawnPosition.x << playerDetails.spawnPosition.y;
	}

	return packetToSend;
}

sf::Packet & sf::operator>>(Packet & receivedPacket, ServerMessagePlayerMove & playerMoveMessage)
{
	receivedPacket >> playerMoveMessage.newPosition.x >> playerMoveMessage.newPosition.y >> playerMoveMessage.speed;

	return receivedPacket;
}

sf::Packet & sf::operator<<(Packet & packetToSend, ServerMessagePlayerMove playerMoveMessage)
{
	packetToSend << playerMoveMessage.newPosition.x << playerMoveMessage.newPosition.y << playerMoveMessage.speed;

	return packetToSend;
}