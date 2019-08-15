#pragma once

#include <functional>

TS_PACKAGE1(lang)

typedef int32_t SignalConnectionIndex;
static const SignalConnectionIndex InvalidSignalIndex = -1;

class AbstractSignalBase;

class SignalBind : public lang::NoncopyableAndNonmovable
{
	template<class... Args>
	friend class Signal;

public:
	SignalBind();
	~SignalBind();

	template<class ClassType, class... Args>
	void connect(Signal<Args...> &signal, void(ClassType::*function)(Args...), ClassType *instance);

	template<class... Args>
	void connect(Signal<Args...> &signal, std::function<void(Args...)> function);

	void disconnect();
	bool isConnected();

private:
	AbstractSignalBase *ownerSignal = nullptr;
	SignalConnectionIndex connectedSignalIndex = InvalidSignalIndex;
};

template<class ClassType, class... Args>
void SignalBind::connect(Signal<Args...> &signal, void(ClassType::*function)(Args...), ClassType *instance)
{
	signal.connect(*this, function, instance);
}

template<class... Args>
void SignalBind::connect(Signal<Args...> &signal, std::function<void(Args...)> function)
{
	signal.connect(*this, function);
}

class AbstractSignalBase
{
	friend class SignalBind;

public:
	AbstractSignalBase() {}
	virtual ~AbstractSignalBase() {}

	virtual void disconnect(SignalBind &signalBind) = 0;
};

template<class... Args>
class Signal : public AbstractSignalBase
{
public:
	Signal();
	virtual ~Signal();

	template <class ClassType>
	void connect(SignalBind &signalBind, void(ClassType::*function)(Args...), ClassType *instance);
	void connect(SignalBind &signalBind, const std::function<void(Args...)> &function);
	void disconnect(SignalBind &signalBind);

	bool hasConnections() const;
	void disconnectAll();

	void operator()(Args... args);

private:
	struct SignalFunctionContainer
	{
		SignalBind *signalBind;
		std::function<void(Args...)> signalFunction;
	};
	typedef std::map<SizeType, SignalFunctionContainer> ConnectedSignalsList;
	ConnectedSignalsList connectedSignals;

	SignalConnectionIndex nextConnectionId = 0;
};

template<class... Args>
Signal<Args...>::Signal()
{
}

template<class... Args>
Signal<Args...>::~Signal()
{
	disconnectAll();
}

template<class... Args>
template<class ClassType>
void Signal<Args...>::connect(SignalBind &signalBind, void(ClassType::*function)(Args...), ClassType *instance)
{
	TS_ASSERT(function != nullptr);
	TS_ASSERT(instance != nullptr);
	if (function == nullptr || instance == nullptr)
		return;

	connect(signalBind, [function, instance](Args... args)
	{
		(instance->*function)(std::forward<Args>(args)...);
	});
}

template<class... Args>
void Signal<Args...>::connect(SignalBind &signalBind, const std::function<void(Args...)> &function)
{
	TS_ASSERT(function != false);

	signalBind.connectedSignalIndex = ++nextConnectionId;
	signalBind.ownerSignal = this;

	connectedSignals.insert(std::make_pair(
		signalBind.connectedSignalIndex,
		SignalFunctionContainer { &signalBind, std::move(function) }
	));
}

template<class... Args>
void Signal<Args...>::disconnect(SignalBind &signalBind)
{
	if (signalBind.connectedSignalIndex == InvalidSignalIndex)
		return;

	ConnectedSignalsList::iterator it = connectedSignals.find(signalBind.connectedSignalIndex);
	if (it != connectedSignals.end())
		connectedSignals.erase(it);
}

template<class... Args>
bool Signal<Args...>::hasConnections() const
{
	return !connectedSignals.empty();
}

template<class... Args>
void Signal<Args...>::disconnectAll()
{
	for (auto &it : connectedSignals)
	{
		// Reset SignalBind's references so things don't explode
		SignalFunctionContainer &sfc = it.second;
		sfc.signalBind->ownerSignal = nullptr;
		sfc.signalBind->connectedSignalIndex = InvalidSignalIndex;
	}
	connectedSignals.clear();
}

template<class... Args>
void Signal<Args...>::operator()(Args... args)
{
	for (auto &it : connectedSignals)
	{
		std::invoke(it.second.signalFunction, std::forward<Args>(args)...);
	}
}

TS_END_PACKAGE1()
