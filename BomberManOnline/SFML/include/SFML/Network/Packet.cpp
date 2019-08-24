#include "Packet.hpp"
#include "ServerMessageType.h"
#include "ServerMessages.h"

sf::Packet & sf::operator>>(Packet & packetReceived, eServerMessageType & serverMessageType)
{
	packetReceived >> serverMessageType;

	return packetReceived;
}

sf::Packet & sf::operator<<(Packet & packetToSend, const eServerMessageType serverMessageType)
{
	packetToSend << serverMessageType;

	return packetToSend;
}

sf::Packet & sf::operator>>(Packet & packetReceived, ServerMessageInitialGameData & messageReceived)
{
	packetReceived >> messageReceived.levelName;

	return packetReceived;

	// TODO: insert return statement here
}

sf::Packet & sf::operator<<(Packet & packetToSend, const ServerMessageInitialGameData & messageToSend)
{

	return packetToSend;
	// TODO: insert return statement here
}
