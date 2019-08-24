#pragma once

enum class eServerMessageType
{
	eInvalidRequest = 0,
	eValidRequest, 
	eInitializeClientID,
	eInitialGameData,
	ePlayerMove,
	eNewPlayerPosition
};