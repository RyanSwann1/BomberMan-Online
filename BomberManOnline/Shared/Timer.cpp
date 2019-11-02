#include "Timer.h"

Timer::Timer(eTimerActive active)
	: m_expirationTime(0.0f),
	m_elaspedTime(0.0f),
	m_active(static_cast<bool>(active))
{}

Timer::Timer(float expirationTime, eTimerActive active)
	: m_expirationTime(expirationTime),
	m_elaspedTime(0.0f),
	m_active(static_cast<bool>(active))
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
	m_elaspedTime = 0.0f;
	m_expirationTime = expirationTime;
}

void Timer::setActive(bool active)
{
	m_active = active;
}

void Timer::resetElaspedTime()
{
	m_elaspedTime = 0.0f;
}

void Timer::update(float deltaTime)
{
	if (m_active)
	{
		m_elaspedTime += deltaTime;
	}
}