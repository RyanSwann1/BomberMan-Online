#pragma once

#include "PlayerServer.h"

enum class eAIBehaviour
{
	ePassive = 0, //Target boxes until non left, then Player
	eAggressive //Target Player when in sight
};

enum class eAIState
{
	eMakeDecision = 0,
	eMovingToBox,
	eMovingToTargetPlayer,
	eMovingToNearestPlayer,
	eMovingToSafePosition,
	eMovingToPickUp,
	eSetDestinationAtBox,
	eSetDestinationToTargetPlayer,
	eSetDestinationAtSafePosition,
	ePlantBomb,
	ePlantAndKickBomb,
	eWait
};

class PlayerServerAI : public PlayerServer
{
public:
	PlayerServerAI(int ID, sf::Vector2f startingPosition, Server& server);

	void update(float frameTime) override final;
	virtual bool placeBomb() override final;

private:
	Server& m_server;
	eAIBehaviour m_behavour;
	eAIState m_currentState;
	std::vector<sf::Vector2f> m_pathToTile;
	Timer m_waitTimer;
	int m_targetPlayerID;

	void handleAIStates(float frameTime);
	void onSetDestinationToTargetPlayer(const PlayerServer& targetPlayer);

#ifdef RENDER_PATHING
	void handleRenderPathing();
#endif // RENDER_PATHING
};