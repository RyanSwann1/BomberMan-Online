#pragma once

enum class eServerMessageType
{
	eInvalidMoveRequest = 0,
	eValidMoveRequest, 
	eInitializeClientID,
	eInitialGameData,
	ePlayerMove,
	ePlayerMoveToPosition,
	eNewPlayerPosition
};