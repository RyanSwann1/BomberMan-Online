#include "Timer.h"

Timer::Timer(bool active)
	: m_expirationTime(0),
	m_elaspedTime(0),
	m_active(active)
{}

Timer::Timer(float expirationTime, bool active)
	: m_expirationTime(expirationTime),
	m_elaspedTime(0),
	m_active(active)
{}

float Timer::getExpirationTime() const
{
	return m_expirationTime;
}

bool Timer::isExpired() const
{
	return m_elaspedTime >= m_expirationTime;
}

void Timer::setExpiredTime(float expirationTime)
{
	m_elaspedTime = 0;
	m_expirationTime = expirationTime;
}

void Timer::setActive(bool active)
{
	m_active = active;
}

void Timer::resetElaspedTime()
{
	m_elaspedTime = 0;
}

void Timer::update(float deltaTime)
{
	if (m_active)
	{
		m_elaspedTime += deltaTime;
	}
}