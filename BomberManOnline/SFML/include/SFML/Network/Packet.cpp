#include "Packet.hpp"
#include "ServerMessages.h"
#include <string>

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
		int controllerType = 0;
		receivedPacket >> controllerType;

		initialGameData.playerDetails.emplace_back(ID, spawnPosition, static_cast<ePlayerControllerType>(controllerType));
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
		packetToSend << static_cast<int>(playerDetails.controllerType);
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

sf::Packet & sf::operator>>(Packet & receivedPacket, ServerMessageBombPlacement & bombPlacementMessage)
{
	receivedPacket >> bombPlacementMessage.position.x >> bombPlacementMessage.position.y >> bombPlacementMessage.playerID;

	return receivedPacket;
}

sf::Packet & sf::operator<<(Packet & packetToSend, ServerMessageBombPlacement bombPlacementMessage)
{
	packetToSend << bombPlacementMessage.position.x << bombPlacementMessage.position.y << bombPlacementMessage.playerID;

	return packetToSend;
}