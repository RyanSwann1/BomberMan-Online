#pragma once

enum class eServerMessageType
{
	eInvalidRequest = 0,
	eValidRequest, 
	eInitialGameData,
	ePlayerMove,
	eNewPlayerPosition
};