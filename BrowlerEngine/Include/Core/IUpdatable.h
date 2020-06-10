#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

class Engine;

class IUpdatable
{
	friend class Engine;
	void internalInit() {
		initialized = init();
	};

	void internalDestroy() {
		initialized = false;
		destroy();
	}
public:
	virtual bool init() = 0;
	virtual void update(double dt) = 0;
	virtual void destroy() = 0;

	bool isInitialized() const { return initialized; }

	bool initialized = false;
};

BRWL_NS_END