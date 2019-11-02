#pragma once

enum class eTimerActive
{
	eFalse = 0,
	eTrue
};

class Timer
{
public:
	Timer(eTimerActive active = eTimerActive::eFalse);
	Timer(float expirationTime, eTimerActive active = eTimerActive::eFalse);

	float getExpirationTime() const;
	bool isExpired() const;
	
	void setExpiredTime(float expirationTime);
	void setActive(bool active);
	void resetElaspedTime();
	void update(float deltaTime);

private:
	float m_expirationTime;
	float m_elaspedTime;
	bool m_active;
};