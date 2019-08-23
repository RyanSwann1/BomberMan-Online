#include "NetworkHandler.h"
#include <iostream>
#include "Level.h"
#include "Resources.h"
#include "Player.h"
#include <algorithm>

//http://www.codersblock.org/blog/multiplayer-fps-part-1

sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor)
{
	if (factor > 1.f)
	{
		factor = 1.f;
	}
	else if (factor < 0.f)
	{
		factor = 0.f;
	}

	return pointA + (pointB - pointA) * factor;
}

bool isPositionCollidable(const Level& level, sf::Vector2f position)
{
	sf::Vector2i pos(position.x, position.y);
	auto cIter = std::find_if(level.getCollisionLayer().cbegin(), level.getCollisionLayer().cend(), [pos](const auto collidablePosition)
		{ return collidablePosition == pos; });
	
	if (cIter == level.getCollisionLayer().cend())
	{
		return false;
	}
	else
	{
		return true;
	}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(640, 480), "SFML_WINDOW", sf::Style::Default);
	
	//if (!NetworkHandler::getInstance().connectToServer())
	//{
	//	std::cerr << "Couldn't connect to server\n";
	//	return -1;
	//}

	if (!Textures::getInstance().loadAllTextures())
	{
		return -1;
	}

	std::unique_ptr<Level> level = Level::create("Level1.tmx");
	if (!level)
	{
		return -1;
	}

	int tileSize = Textures::getInstance().getTileSheet().getTileSize();
	Player player(tileSize);
	
	sf::Clock gameClock;
	float deltaTime = 0;
	float factor = 0;
	const auto& collisionLayer = level->getCollisionLayer();
	while (window.isOpen())
	{
		sf::Event sfmlEvent;
		while (window.pollEvent(sfmlEvent))
		{
			if (sfmlEvent.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		//Move player Left
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) &&
			!player.m_moving && !isPositionCollidable(*level, sf::Vector2f(player.m_position.x - tileSize, player.m_position.y)))
		{
			factor = 0;
			player.m_newPosition = sf::Vector2f(player.m_position.x - tileSize, player.m_position.y);
			player.m_moving = true;
		}
		//Move player right
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) &&
			!player.m_moving && !isPositionCollidable(*level, sf::Vector2f(player.m_position.x + tileSize, player.m_position.y)))
		{
			factor = 0;
			player.m_newPosition = sf::Vector2f(player.m_position.x + tileSize, player.m_position.y);
			player.m_moving = true;
		}
		//Move player up
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) &&
			!player.m_moving && !isPositionCollidable(*level, sf::Vector2f(player.m_position.x, player.m_position.y - tileSize)))
		{
			factor = 0;
			player.m_newPosition = sf::Vector2f(player.m_position.x, player.m_position.y - tileSize);
			player.m_moving = true;
		}
		//Move player down
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) &&
			!player.m_moving && !isPositionCollidable(*level, sf::Vector2f(player.m_position.x, player.m_position.y + tileSize)))
		{
			factor = 0;
			player.m_newPosition = sf::Vector2f(player.m_position.x, player.m_position.y + tileSize);
			player.m_moving = true;
		}
		
		if (player.m_moving)
		{
			factor += deltaTime * player.m_movementSpeed;
			player.m_position = Interpolate(player.m_previousPosition, player.m_newPosition, factor);
			player.m_shape.setPosition(player.m_position);
			if (player.m_position == player.m_newPosition)
			{
				player.m_previousPosition = player.m_newPosition;
				player.m_position = player.m_newPosition;
				player.m_moving = false;
			}
		}

		window.clear(sf::Color::Black);
		level->render(window);
		window.draw(player.m_shape);
		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}
}