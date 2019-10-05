#include "Precompiled.h"
#include "Signal.h"

TS_PACKAGE1(lang)

SignalBind::SignalBind()
{

}

SignalBind::~SignalBind()
{
	disconnect();
}

bool SignalBind::isConnected()
{
	return connectedSignalIndex != InvalidSignalIndex;
}

void SignalBind::disconnect()
{
	if (isConnected() && ownerSignal != nullptr)
		ownerSignal->disconnect(*this);

	ownerSignal = nullptr;
	connectedSignalIndex = InvalidSignalIndex;
}

TS_END_PACKAGE1()

