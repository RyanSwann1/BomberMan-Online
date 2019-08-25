#pragma once

class Timer
{
public:
	Timer(float expirationTime, bool active = false);

	bool isExpired() const;
	
	void setActive(bool active);
	void resetElaspedTime();
	void update(float deltaTime);

private:
	float m_expirationTime;
	float m_elaspedTime;
	bool m_active;
};