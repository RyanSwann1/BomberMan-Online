#pragma once

enum class eServerMessageType
{
	eInvalidRequest = 0,
	eValidRequest, 
	ePlayerMove,
	eNewPlayerPosition
};