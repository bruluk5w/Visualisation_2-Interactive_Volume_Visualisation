#pragma once // (c) 2020 Lukas Brunner

BRWL_NS


class Timer;

class TickProvider
{
	friend class Timer;
public:
	TickProvider();

	void nextFrame();
	uint64_t forceGetTick();

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
		paused(false),
		startTick(0),
		currentTick(0),
		lastTick(0),
		deltaTicks(0),
		gameTime(0),
		deltaTime(0),
		unmodifiedTime(0),
		lastUnmodifiedTime(0),
		deltaUnmodifiedTime(),
		frameCounter(0),
		tickProvider(tickProvider)
	{
		BRWL_EXCEPTION(tickProvider != nullptr, BRWL_CHAR_LITERAL("tickProvider may not be null"));
		BRWL_EXCEPTION(tickProvider->timer == nullptr, BRWL_CHAR_LITERAL("tickProvider may not be null"));
		tickProvider->timer = this;
	}

	void start()
	{
		if (started) return;
		started = true;
		tickProvider->nextFrame();
		lastTick = currentTick = startTick = tickProvider->tick;
		deltaTicks = 0;
		gameTime = 0;
		deltaTime = 0;
		unmodifiedTime = 0;
		lastUnmodifiedTime = 0;
		deltaUnmodifiedTime = 0;
	}

	void pause()
	{
		if (paused) return;

		paused = true;
	}

	void unpause()
	{
		if (!paused) return;

		paused = false;
	}

	void stop()
	{
		if (!started) return;

		started = false;
	}

	double getTime() const { return gameTime; }
	float getTimeF() const { return (float)gameTime; }
	double getDeltaTime() const { return deltaTime; }
	float getDeltaTimeF() const { return (float)deltaTime; }
	double getUnmodifiedTime() const { return unmodifiedTime; }
	float getUnmodifiedTimeF() const { return (float)unmodifiedTime; }
	double forceGetUnmodifiedTime() const {return (double)tickProvider->forceGetTick() - startTick / (double)tickProvider->frequency; }
	float forceGetUnmodifiedTimeF() const {return (float)((double)(tickProvider->forceGetTick() - startTick) / (double)tickProvider->frequency); }
	double getDeltaUnmodifiedTime() const { return deltaUnmodifiedTime; }
	float getDeltaUnmodifiedTimeF() const { return (float)deltaUnmodifiedTime; }

	bool isRunning() { return started; }

	size_t getFrameCount() const { return frameCounter; }

private:
	void onTickUpdated()
	{
		BRWL_EXCEPTION(started, BRWL_CHAR_LITERAL("The timer has to be started in order to call update"));
		++frameCounter;
		lastTick = currentTick;
		currentTick = tickProvider->tick;
		deltaTicks = currentTick - lastTick;
		lastUnmodifiedTime = unmodifiedTime;
		unmodifiedTime += (double)deltaTicks / (double)tickProvider->frequency;
		deltaUnmodifiedTime = unmodifiedTime - lastUnmodifiedTime;
		
		// Unmodified time which is updated by deltas diverges from unmodified time if calculated from absolute values!
		BRWL_CHECK(std::abs(unmodifiedTime - (double)(currentTick - startTick) / (double)tickProvider->frequency) < 0.000001, BRWL_CHAR_LITERAL("Unmodified time diverges!"));

		if (!paused) {
			double lastGameTime = gameTime;
			gameTime += deltaUnmodifiedTime;
			deltaTime = deltaUnmodifiedTime;
		}
		else
		{
			deltaTime = 0;
		}
	}

	bool started;
	bool paused;
	uint64_t startTick;
	uint64_t lastTick;
	uint64_t currentTick;
	uint64_t deltaTicks;
	double gameTime;
	double deltaTime;
	double lastUnmodifiedTime;
	double unmodifiedTime;
	double deltaUnmodifiedTime;
	size_t frameCounter;
	TickProvider* tickProvider;
};


BRWL_NS_END