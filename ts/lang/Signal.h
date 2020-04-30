#pragma once

#include <functional>
#include <type_traits>
#include <map>

TS_PACKAGE1(lang)

typedef int32_t SignalConnectionIndex;
static const SignalConnectionIndex InvalidSignalIndex = -1;

class AbstractSignalBase;

enum SignalPriority : SizeType
{
	SignalPriority_VeryHigh = 100,
	SignalPriority_High     = 80,
	SignalPriority_Normal   = 60,
	SignalPriority_Low      = 40,
	SignalPriority_VeryLow  = 20,
};

template<class... Args>
class Signal;

class SignalBind : public lang::NoncopyableAndNonmovable
{
	template<class... Args>
	friend class Signal;

public:
	SignalBind();
	~SignalBind();

	template<class... Args, class ClassType>
	void connect(Signal<Args...> &signal, void(ClassType::*function)(Args...), ClassType *instance);
	template<class... Args, class ClassType>
	void connect(Signal<Args...> &signal, SignalPriority priority, void(ClassType::*function)(Args...), ClassType *instance);

	template<class... Args, class Function>
	void connect(Signal<Args...> &signal, Function &&function);
	template<class... Args, class Function>
	void connect(Signal<Args...> &signal, SignalPriority priority, Function &&function);

	void disconnect();
	bool isConnected();

private:
	AbstractSignalBase *ownerSignal = nullptr;
	SignalConnectionIndex connectedSignalIndex = InvalidSignalIndex;
};

template<class... Args, class ClassType>
void SignalBind::connect(Signal<Args...> &signal, void(ClassType::*function)(Args...), ClassType *instance)
{
	signal.connect(*this, SignalPriority_Normal, function, instance);
}

template<class... Args, class ClassType>
void SignalBind::connect(Signal<Args...> &signal, SignalPriority priority, void(ClassType::*function)(Args...), ClassType *instance)
{
	signal.connect(*this, priority, function, instance);
}

template<class... Args, class Function>
void SignalBind::connect(Signal<Args...> &signal, Function &&function)
{
	typedef typename std::result_of<Function(Args...)>::type ReturnType;
	static_assert(!std::is_same<ReturnType, void>::value, "Signal callback return type must be void.");

	signal.connect(*this, SignalPriority_Normal, std::forward<Function>(function));
}

template<class... Args, class Function>
void SignalBind::connect(Signal<Args...> &signal, SignalPriority priority, Function &&function)
{
	signal.connect(*this, priority, std::forward<Function>(function));
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
	template <class ClassType>
	void connect(SignalBind &signalBind, SignalPriority priority, void(ClassType::*function)(Args...), ClassType *instance);

	template <class Function>
	void connect(SignalBind &signalBind, Function &&function);
	template <class Function>
	void connect(SignalBind &signalBind, SignalPriority priority, Function &&function);

	virtual void disconnect(SignalBind &signalBind);

	bool hasConnections() const;
	void disconnectAll();

	void operator()(Args... args);

private:
	struct SignalConnectionKey
	{
		SignalConnectionIndex connectionIndex;
		SignalPriority priority;

		SignalConnectionKey(SignalConnectionIndex index, SignalPriority priority)
			: connectionIndex(index)
			, priority(priority)
		{
		}

		SignalConnectionKey(SignalConnectionIndex index)
			: connectionIndex(index)
			, priority(SignalPriority_Normal)
		{
		}

		bool operator==(const SignalConnectionKey &other) const
		{
			return connectionIndex = other.connectionIndex;
		}

		bool operator<(const SignalConnectionKey &other) const
		{
			return priority > other.priority ||
				(priority == other.priority && connectionIndex < other.connectionIndex);
		}
	};

	struct SignalFunctionContainer
	{
		SignalBind *signalBind;
		std::function<void(Args...)> signalFunction;
	};

	typedef std::map<SignalConnectionKey, SignalFunctionContainer> ConnectedSignalsList;
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
	connect(signalBind, SignalPriority_Normal, function, instance);
}

template<class... Args>
template<class ClassType>
void Signal<Args...>::connect(SignalBind &signalBind, SignalPriority priority, void(ClassType::*function)(Args...), ClassType *instance)
{
	TS_ASSERT(function != nullptr && instance != nullptr);
	if (function == nullptr || instance == nullptr)
		return;

	std::function<void(Args...)> bound = [function, instance](Args... args)
	{
		(instance->*function)(std::forward<Args>(args)...);
	};
	connect(
		signalBind,
		priority,
		std::move(bound)
	);
}

template<class... Args>
template <class Function>
void Signal<Args...>::connect(SignalBind &signalBind, Function &&function)
{
	conenct(signalBind, SignalPriority_Normal, std::move(function));
}

template<class... Args>
template <class Function>
void Signal<Args...>::connect(SignalBind &signalBind, SignalPriority priority, Function &&function)
{
	signalBind.connectedSignalIndex = ++nextConnectionId;
	signalBind.ownerSignal = this;

	connectedSignals.insert(std::make_pair(
		SignalConnectionKey { signalBind.connectedSignalIndex, priority },
		SignalFunctionContainer { &signalBind, std::move(function) }
	));
}

template<class... Args>
void Signal<Args...>::disconnect(SignalBind &signalBind)
{
	if (signalBind.connectedSignalIndex == InvalidSignalIndex)
		return;

	typename ConnectedSignalsList::iterator it = connectedSignals.find(signalBind.connectedSignalIndex);
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
