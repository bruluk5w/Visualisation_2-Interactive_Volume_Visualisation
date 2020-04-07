#pragma once

BRWL_NS

class Timer;

class TickProvider
{
	friend class Timer;
public:
	TickProvider();

	void nextFrame();

	uint64_t tick;
	uint64_t frequency;

	Timer* timer;
};

class Timer
{
	friend class TickProvider;
public:
	Timer(TickProvider* tickProvider) :
		started(false),
		startTick(0),
		lastTick(0),
		gameTime(0),
		tickProvider(tickProvider)
	{
		BRWL_EXCEPTION(tickProvider != nullptr, "tickProvider may not be null");
		BRWL_EXCEPTION(tickProvider->timer == nullptr, "tickProvider may not be null");
		tickProvider->timer = this;
	}

	void start()
	{
		if (started) return;

		tickProvider->nextFrame();
		startTick = tickProvider->tick;
		deltaTime = 0;
		gameTime = 0;
		started = true;
	}

	void stop()
	{
		if (!started) return;

		started = false;
	}

	double getGameTime() const { return gameTime; }
	float getGameTimeF() const { return (float)gameTime; }
	double getDeltaTime() const { return deltaTime; }
	float getDeltaTimeF() const { return (float)deltaTime; }

	bool isRunning() { return started; }

private:
	void onTickUpdated()
	{
		double lastGameTime = gameTime;
		gameTime = (double)(tickProvider->tick - startTick) / (double)tickProvider->frequency;
		deltaTime = gameTime - lastGameTime;
	}

	bool started;
	uint64_t startTick;
	uint64_t lastTick;
	uint64_t deltaTicks;
	double gameTime;
	double deltaTime;
	TickProvider* tickProvider;
};

BRWL_NS_END