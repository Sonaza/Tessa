#include "Precompiled.h"
#include "ViewerEvent.h"

TS_PACKAGE2(app, viewer)

ViewerEvent::ViewerEvent()
{

}

ViewerEvent::~ViewerEvent()
{
	
}

ViewerEvent::ViewerEvent(const ViewerEvent &other)
{
	*this = other;
}

ViewerEvent &ViewerEvent::operator=(const ViewerEvent &other)
{
	if (this != &other)
	{
		type = other.type;
		dataString = other.dataString;
		dataBool = other.dataBool;
	}
	return *this;
}

ViewerEvent::ViewerEvent(ViewerEvent &&other)
{
	*this = std::move(other);
}

ViewerEvent &ViewerEvent::operator=(ViewerEvent &&other)
{
	if (this != &other)
	{
		type = std::exchange(other.type, Unspecified);
		dataString = std::move(other.dataString);
		dataBool = std::exchange(other.dataBool, false);
	}
	return *this;
}

TS_END_PACKAGE2()
