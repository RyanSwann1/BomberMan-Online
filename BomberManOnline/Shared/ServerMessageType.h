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
	eExplodeBomb,
	eDestroyBox,
	ePlayerDisconnected,
	eRequestDisconnection,
	//PickUps
	eSpawnMovementPickUp,
	eSpawnExtraBombPickUp,
	eSpawnBiggerExplosionPickUp,
	eMovementPickUpCollision,
	eExtraBombPickUpCollision,
	eBiggerExplosionPickUpCollision
};