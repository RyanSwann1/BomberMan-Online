#pragma once

enum class eServerMessageType
{
	eInvalidMoveRequest = 0,
	eInitializeClientID,
	eInitialGameData,
	ePlayerMove,
	ePlayerMoveToPosition,
	eNewPlayerPosition,
	ePlayerBombPlacementRequest,
	eValidBombPlacementRequest,
	ePlaceBomb,
	eBombExplosion,
	ePlayerDisconnected,
	eRequestDisconnection,
	eRequestKickBomb,
	eBombKicked,
	//PickUps
	eSpawnMovementPickUp,
	eSpawnExtraBombPickUp,
	eSpawnBiggerExplosionPickUp,
	eMovementPickUpCollision,
	eExtraBombPickUpCollision,
	eBiggerExplosionPickUpCollision
};