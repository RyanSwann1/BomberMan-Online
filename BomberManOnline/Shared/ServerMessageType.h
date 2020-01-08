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
	ePathToRender,
	eNewCollidableTile,
	//PickUps
	eSpawnMovementPickUp,
	eSpawnExtraBombPickUp,
	eSpawnBiggerExplosionPickUp,
	eMovementPickUpCollision,
	eExtraBombPickUpCollision,
	eBiggerExplosionPickUpCollision,
	eRemovePickUpAtLocation
};