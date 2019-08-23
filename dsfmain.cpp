#include "Level/Level.h"
#include "Level/LevelParser.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <functional>
#include <queue>
#include "Texture.h"
#include <algorithm>

//https://gamedev.stackexchange.com/questions/44037/what-are-the-most-common-ai-systems-implemented-in-tower-defense-games

//5, 15 - Start Point
//13, 0 - Ending Point

class Graph
{
public:
	Graph(const std::vector<sf::FloatRect>& collisionLayer, sf::Vector2f startingPoint, sf::Vector2f endingPoint, int tileSize)
		: m_points(),
		m_complete(false)
	{	
		std::vector<sf::Vector2f> points;

		for (int y = -1; y <= 1; y += 2)
		{
			for (int x = -1; x <= 1; x += 2)
			{
				for (const auto& collidableTile : collisionLayer)
				{
					sf::FloatRect tileAABB(sf::Vector2f(startingPoint.x + x * tileSize, startingPoint.y + y * tileSize), sf::Vector2f(tileSize, tileSize));
					if (!tileAABB.intersects(collidableTile))
					{
						points.emplace_back(startingPoint.x + x, startingPoint.y + y);
					}	
				}
			}
		}

		int i = 0;
	}

private:
	std::vector<sf::Vector2f> m_points;
	bool m_complete;
};

enum class FactionName
{
	Yellow = 0,
	Green
};

struct ShipDestroyedEvent
{
	ShipDestroyedEvent(int ID, FactionName factionName)
		: m_shipID(ID),
		m_factionName(factionName)
	{}

	int m_shipID;
	FactionName m_factionName;
};

enum class GameEvent
{
	ShipDestroyed = 0,
	GameWon
};

struct Message
{
	Message(GameEvent gameEvent, void* data)
		: m_data(data),
		m_gameEvent(gameEvent)
	{}

	void* m_data;
	GameEvent m_gameEvent;
};

void onMessage(Message message)
{
	if (message.m_gameEvent == GameEvent::ShipDestroyed)
	{
		auto shipDestroyedEvent = static_cast<ShipDestroyedEvent*>(message.m_data);
	}
}

void foo()
{
	std::vector<std::unique_ptr<int>> v;
	for (int i = 0; i < 1000; ++i)
	{

	}
}

sf::Vector2f Interpolate(
	sf::Vector2f pointA,
	sf::Vector2f pointB,
	float factor
) {
	
	if (factor > 1.f)
		factor = 1.f;

	else if (factor < 0.f)
		factor = 0.f;

	float x = (pointB.x - pointA.x) * factor;
	//std::cout << x << "\n";
	return pointA + (pointB - pointA) * factor;
}

int main()
{

	sf::RenderWindow window(sf::VideoMode(1200, 1200), "SFML_WINDOW", sf::Style::Default);
	window.setFramerateLimit(60);
	const Level level(LevelParser::parseLevel("MapOne.tmx"));
	Graph movementGraph(level.getCollisionLayer(), sf::Vector2f(5, 15), sf::Vector2f(13, 0), level.getDetails().m_tileSize);
	const int tileSize = level.getDetails().m_tileSize;

	std::vector<int> numbers
	{
		1,2,3,4,5,6,7,8,9,10
	};
	
	auto cIter = std::find_if(numbers.cbegin(), numbers.cend(), [] (const auto& numb) 
	{
		return numb == 11;
	});

	if (cIter == numbers.cend())
	{
		int i = 0;
	}

	std::reverse(numbers.begin(), numbers.end());
	sf::View view;

	sf::Vector2f pointA(100, 100);
	sf::Vector2f pointB(380, 100);
	sf::RectangleShape rect1;
	rect1.setSize({ 32, 32 });
	rect1.setPosition(pointA);
	rect1.setFillColor(sf::Color::Red);
	sf::Clock gameClock;

	gameClock.restart();
	float deltaTime = 0.f;
	float timeElasped = 0.f;
	float factor = 0.f;
	bool playerMoving = false;
	bool destinationReached = false;

	while (window.isOpen())
	{
		deltaTime = gameClock.restart().asSeconds();

		sf::Event sfmlEvent;
		while (window.pollEvent(sfmlEvent))
		{
			if (sfmlEvent.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (sfmlEvent.type == sf::Event::KeyPressed)
			{
				if (sfmlEvent.key.code == sf::Keyboard::A && !playerMoving)
				{
					factor = 0;
					playerMoving = true;
					destinationReached = false;
					pointB = pointA;
					pointB.x -= 32;
				}
				else if (sfmlEvent.key.code == sf::Keyboard::D && !playerMoving)
				{
					factor = 0;
					playerMoving = true;
					destinationReached = false;
					pointB = pointA;
					pointB.x += 32;
				}
				else if (sfmlEvent.key.code == sf::Keyboard::S && !playerMoving)
				{
					factor = 0;
					playerMoving = true;
					destinationReached = false;
					pointB = pointA;
					pointB.y += 32;
				}
				else if (sfmlEvent.key.code == sf::Keyboard::W && !playerMoving)
				{
					factor = 0;
					playerMoving = true;
					destinationReached = false;
					pointB = pointA;
					pointB.y -= 32;
				}
			}

		}

		sf::RectangleShape spawnRect(sf::Vector2f(tileSize, tileSize));
		spawnRect.setPosition(sf::Vector2f(5 * tileSize, 15 * tileSize));
		spawnRect.setFillColor(sf::Color::Red);

		sf::RectangleShape mouseRect;
		mouseRect.setSize(sf::Vector2f(tileSize, tileSize));
		mouseRect.setOutlineColor(sf::Color::Red);
		mouseRect.setOutlineThickness(2.5f);
		mouseRect.setFillColor(sf::Color::Transparent);

		timeElasped += deltaTime;
		factor += deltaTime * 10.5f;


		if (playerMoving)
		{
			rect1.setPosition(Interpolate(pointA, pointB, factor));	
		}
	
	
		if (rect1.getPosition() == pointB && !destinationReached)
		{
			destinationReached = true;
			//std::cout << "Message Sent\n";
			playerMoving = false;
			factor = 0;
			pointA = rect1.getPosition();
		}
		


		//const sf::IntRect mouseAABB(sf::Vector2i(sf::Mouse::getPosition(window).x - tileSize, sf::Mouse::getPosition(window).y - tileSize), 
		//	sf::Vector2i(tileSize, tileSize));
		//for (int y = 0; y < level.getDetails().m_size.y; ++y)
		//{
		//	for (int x = 0; x < level.getDetails().m_size.x; ++x)
		//	{
		//		sf::IntRect tileRect(sf::Vector2i(x * tileSize, y * tileSize), sf::Vector2i(tileSize, tileSize));
		//		if (mouseAABB.intersects(tileRect))
		//		{
		//			mouseRect.setPosition(tileRect.left, tileRect.top);
		//		}
		//	}
		//}

		window.clear(sf::Color::Black);
		window.draw(rect1);
		window.display();

		
	}

	
	std::unique_ptr<Texture> texture1 = Texture::load(5, 5, true);
	std::unique_ptr<Texture> texture2 = Texture::load(10, 10, false);




	char c;
	std::cin >> c;
	
	return 0;
}