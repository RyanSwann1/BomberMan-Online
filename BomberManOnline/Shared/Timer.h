#pragma once

class Timer
{
public:
	Timer(bool active = false);
	Timer(float expirationTime, bool active = false);

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