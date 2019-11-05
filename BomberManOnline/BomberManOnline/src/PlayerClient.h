#pragma once

#include "Player.h"
#include <SFML/Graphics.hpp>
#include "AnimatedSprite.h"

struct MovementPoint
{
	MovementPoint(sf::Vector2f position, eDirection moveDirection);

	sf::Vector2f position;
	eDirection moveDirection;
};

enum class eCollidableTile;
struct MovementPoint;
class PlayerClient : public Player
{
public:
	PlayerClient(int ID, sf::Vector2f startingPosition, ePlayerType playerType);

	void update(float deltaTime) override;
	void render(sf::RenderWindow& window) const;

	void setNewPosition(sf::Vector2f newPosition, const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i tileSize,
		std::vector<MovementPoint>& localPlayerPreviousPositions);
	void plantBomb();
	void stopAtPosition(sf::Vector2f position);

private:
	const ePlayerType m_playerType;
	AnimatedSprite m_sprite;
	eDirection m_moveDirection;
};