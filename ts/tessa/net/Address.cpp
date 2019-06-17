#include "Precompiled.h"
#include "ts/tessa/net/Address.h"

#include <cstdio>
#include <cstring>

#if TS_PLATFORM == TS_WINDOWS
	#include <WS2tcpip.h>
#elif TS_PLATFORM == TS_LINUX
	#include <sys/socket.h>
	#include <netinet/in.h>
#else
	#error "Address class not implemented on this platform"
#endif

#define ADDRESS_GET_NUM_BYTES(type) ((type) == ts::net::Address::AddressTypeIPv4 ? ts::net::Address::NumBytesAddressIPv4 : ts::net::Address::NumBytesAddressIPv6)
#define SWAP_SHORT_BYTE_ORDER(value) ((((value) >> 8) & 0x00FFu) | (((value) << 8) & 0xFF00u))

TS_PACKAGE1(net)

Address::Address()
	: port(0)
	, virtualPort(0)
	, type(AddressTypeIPv4)
{
	memset(ip, 0, sizeof(ip));
}

Address::Address(const AddressType type, const uint8_t* bytes, const uint16_t port, const uint32_t virtualPort)
	: port(port)
	, virtualPort(virtualPort)
	, type(type)
{
	memset(ip, 0, sizeof(ip));

	TS_ASSERT(bytes != nullptr);
	memcpy(ip, bytes, ADDRESS_GET_NUM_BYTES(type));
}

uint32_t Address::getIPv4AsInt() const
{
	TS_ASSERT(type != AddressTypeIPv6 && "IPv6 address cannot be converted to integer value.");
	if (type != AddressTypeIPv4)
		return 0;

	return static_cast<uint32_t>(ip[0] | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24));
}

void Address::setIPv4FromInt(uint32_t i)
{
	TS_ASSERT(type != AddressTypeIPv6 && "Can't set integer value to an IPv6 address.");
	if (type != AddressTypeIPv4)
		return;

	ip[0] = ((i >> 24) & 0xFF);
	ip[1] = ((i >> 16) & 0xFF);
	ip[2] = ((i >> 8) & 0xFF);
	ip[3] = ((i >> 0) & 0xFF);
}

void Address::retrieveBytes(uint8_t *outBytes, const AddressType inType) const
{
	TS_ASSERT(outBytes != nullptr);
	TS_ASSERT(type == inType && "Address types do not match");

	if (type == inType)
	{
		const size_t numBytes = ADDRESS_GET_NUM_BYTES(inType);
		memcpy(outBytes, ip, numBytes);
	}
}

void Address::setBytes(const AddressType inType, const uint8_t *inBytes)
{
	TS_ASSERT(inBytes != nullptr);

	type = inType;

	const size_t numBytes = ADDRESS_GET_NUM_BYTES(inType);
	memcpy(ip, inBytes, numBytes);
}

void Address::convertToIPv6()
{
	if (type == Address::AddressTypeIPv6)
		return;

	Address newAddress;
	net::convertToIPv6(*this, &newAddress);
	*this = newAddress;
}

std::string Address::getHostName()
{
	SOCKADDR_STORAGE hostAddress;

	switch (type)
	{
		case AddressType::AddressTypeIPv4:
		{
			SOCKADDR_IN temp;
			memset(&temp, 0, sizeof(SOCKADDR_IN));
			temp.sin_family = AF_INET6;
			temp.sin_addr.S_un.S_addr = getIPv4AsInt();
			temp.sin_port = htons(u_short(port));
			hostAddress = *reinterpret_cast<SOCKADDR_STORAGE*>(&temp);
		}
		break;
		case AddressType::AddressTypeIPv6:
		{
			SOCKADDR_IN6 temp;
			memset(&temp, 0, sizeof(SOCKADDR_IN6));
			temp.sin6_family = AF_INET6;
			retrieveBytes(temp.sin6_addr.s6_addr, Address::AddressTypeIPv6);
			temp.sin6_port = htons(u_short(port));
			hostAddress = *reinterpret_cast<SOCKADDR_STORAGE*>(&temp);
		}
		break;
		default:
			TS_ASSERT(0);
		break;
	}

	char hostname[NI_MAXHOST] = { 0 };
	char servInfo[NI_MAXSERV] = { 0 };

	DWORD ret = getnameinfo(
		(sockaddr*)&hostAddress,
		sizeof(SOCKADDR_STORAGE),
		hostname, NI_MAXHOST,
		servInfo, NI_MAXSERV, NI_NUMERICSERV
	);

	if (ret == 0)
		return std::string(hostname);

	TS_LOG_ERROR("getnameinfo returned %d\n", ret);
	return std::string();
}

bool Address::isIPv4mappedIPv6() const
{
	return ip[0]  == 0x00 && ip[1]  == 0x00 &&
	       ip[2]  == 0x00 && ip[3]  == 0x00 &&
	       ip[4]  == 0x00 && ip[5]  == 0x00 &&
	       ip[6]  == 0x00 && ip[7]  == 0x00 &&
	       ip[8]  == 0x00 && ip[9]  == 0x00 &&
	       ip[10] == 0xFF && ip[11] == 0xFF;
}

bool Address::operator!=(const Address &other) const
{
	return !(*this == other);
}

bool Address::operator==(const Address &other) const
{
	if (type != other.type)
		return false;

	const size_t s = ADDRESS_GET_NUM_BYTES(type);
	for (size_t i = 0; i < s; ++i)
	{
		if (ip[i] != other.ip[i])
			return false;
	}

	return port == other.port && virtualPort == other.virtualPort;
}

// Converts an IPv4 address to IPv4-mapped IPv6 format
void convertToIPv6(const Address &originalIPv4, Address *newIPv6)
{
	if (originalIPv4.type == Address::AddressTypeIPv6)
	{
		*newIPv6 = originalIPv4;
		return;
	}

	newIPv6->type = Address::AddressTypeIPv6;

	memset(newIPv6->ip, 0, Address::NumBytesAddressIPv6);

	newIPv6->ip[10] = 0xff;
	newIPv6->ip[11] = 0xff;
	newIPv6->ip[12] = originalIPv4.ip[0];
	newIPv6->ip[13] = originalIPv4.ip[1];
	newIPv6->ip[14] = originalIPv4.ip[2];
	newIPv6->ip[15] = originalIPv4.ip[3];

	newIPv6->port = originalIPv4.port;
	newIPv6->virtualPort = originalIPv4.virtualPort;
}

/* Assumes the given address has a port attached and returns the numbers found after last colon.
 * 
 * You shouldn't try feeding it raw IPv6 addresses.
 */
bool parsePortFromIP(const char* str, uint16_t* port)
{
	const char* last = strrchr(str, ':');
	return last && sscanf(last + 1, "%hu", port) == 1;
}

bool parseIPv6(const char* str, uint8_t* outBytes, uint16_t* outPort = nullptr)
{
	TS_ASSERT(strlen(str) < TS_IPV6_WITH_PORT_MAX_LEN);
	TS_ASSERT(outBytes != nullptr);

	char address[TS_IPV6_MAX_LEN];
	memset(address, 0, TS_IPV6_MAX_LEN);

	const char* openBracket = strchr(str, '[');
	const char* closeBracket = strchr(str, ']');

	if (openBracket && closeBracket)
	{
		if (openBracket >= closeBracket)
			return false;

		TS_ASSERT((closeBracket - openBracket - 1) < TS_IPV6_MAX_LEN);
		memcpy(address, openBracket + 1, static_cast<size_t>(closeBracket - openBracket - 1));
	}
	else
	{
		if (openBracket == nullptr && closeBracket == nullptr)
		{
			TS_ASSERT(strlen(str) < TS_IPV6_MAX_LEN);
			strcpy(address, str);
		}
		else
		{
			return false;
		}
	}

	union
	{
		uint8_t bytes[16];
		uint16_t shorts[8]; // stored in big endian byte order
	} ip;
	memset(ip.bytes, 0, 16);

	bool contracted = false;
	
	int8_t numColons = 0;
	int8_t numPeriods = 0;
	for (size_t i = 0; address[i] != '\0'; ++i)
	{
		if (address[i] == ':')
		{
			// There should not be any colons after first periods are found (ipv4 mapped ipv6)
			if (numPeriods > 0)
				return false;

			numColons++;

			// Quick check to see if there are valid number of colons sequentially
			// or if a contraction was already detected earlier
			if (address[i + 1] == ':')
			{
				if (!contracted)
					contracted = true;
				else
					return false;
			}
		}
		else if (address[i] == '.')
		{
			numPeriods++;
		}
	}

	// If address is IPv4 mapped then it must have exactly 3 periods in it
	if (numPeriods > 0 && numPeriods != 3)
		return false;

	bool ipv4mapped = (numPeriods > 0);

	// If not contracted the number of colons should be 7, or if ipv4mapped 6 
	if (!contracted && ((!ipv4mapped && numColons < 7) || (ipv4mapped && numColons < 6)))
		return false;

	contracted = false;

	int8_t sid = 0;
	const char* colonPtr = address - 1;
	do
	{
		int8_t soffset = 1;

		const char* nextColonPtr = strchr(colonPtr + 1, ':');
		char temp[16] = { 0 };

		int32_t len = nextColonPtr > colonPtr ? int32_t(nextColonPtr - colonPtr - 1) : (int32_t)strlen(colonPtr + 1);
		if (len > 0)
		{
			// Normal segment is up to 4 characters long
			if (len <= 4)
			{
				memcpy(temp, colonPtr + 1, static_cast<size_t>(len));
				if (sscanf(temp, "%hx", &ip.shorts[sid]) != 1)
					return false;

				// Storing values in big endian byte order so convert the endianness if needed
				#if TS_BYTE_ORDER == TS_LITTLE_ENDIAN
					ip.shorts[sid] = SWAP_SHORT_BYTE_ORDER(ip.shorts[sid]);
				#endif
			}
			else if (ipv4mapped && sid == 6 && len <= 15) // Check for special case ipv4-mapped segment
			{
				memcpy(temp, colonPtr + 1, static_cast<size_t>(len));
				if (sscanf(temp, "%hhu.%hhu.%hhu.%hhu", &ip.bytes[12], &ip.bytes[13], &ip.bytes[14], &ip.bytes[15]) != 4)
				{
					// Parsing IPv4-mapped segment failed
					return false;
				}
				else
				{
					// IPv4-mapped segment is the last one, loop can be broken
					break;
				}
			}
			else
			{
				// Segment is unexpected length, address is probably not valid
				return false;
			}
		}
		else if (!contracted)
		{
			soffset = (!ipv4mapped ? 8 : 7) - numColons;
			contracted = true;
		}

		sid += soffset;
	} while ((colonPtr = strchr(colonPtr + 1, ':')) != nullptr);
	
	memcpy(outBytes, ip.bytes, 16);

	if (outPort && openBracket && closeBracket)
	{
		parsePortFromIP(str, outPort);
	}

	return true;
}

bool parseAddressWithoutPort(const char *str, Address &address)
{
	bool success = sscanf(str, "%hhu.%hhu.%hhu.%hhu", &address.ip[0], &address.ip[1], &address.ip[2], &address.ip[3]) == 4;

	if (success)
	{
		// Just making sure the type is right
		address.type = Address::AddressTypeIPv4;
		return true;
	}
	else // Try parsing as IPv6
	{
		uint8_t bytes[16];
		success = parseIPv6(str, bytes);
		if (success)
		{
			memcpy(address.ip, bytes, 16);
			address.type = Address::AddressTypeIPv6;
			return true;
		}
	}

	return false;
}

bool parseAddress(const char *str, Address &address)
{
	bool success = sscanf(str, "%hhu.%hhu.%hhu.%hhu:%hu", &address.ip[0], &address.ip[1], &address.ip[2], &address.ip[3], &address.port) == 5;

	if (success)
	{
		// Just making sure the type is right
		address.type = Address::AddressTypeIPv4;
		return true;
	}
	else // Try parsing as IPv6
	{
		uint8_t bytes[16];
		uint16_t port;
		success = parseIPv6(str, bytes, &port);
		if (success)
		{
			memcpy(address.ip, bytes, 16);
			address.port = port;
			address.type = Address::AddressTypeIPv6;
			return true;
		}
	}

	return false;
}

bool parsePort(const char *str, uint16_t &port)
{
	return sscanf(str, "%hu", &port) == 1;
}

void printIPv6(char* outBuffer, size_t outBufferSize, const uint8_t* bytes)
{
	TS_ASSERT(outBuffer != nullptr);

	uint16_t segments[8];
	memset(segments, 0, sizeof(segments));

	size_t mostZeroesNum = 0;
	size_t mostZeroesStart = 0;
	size_t currentZeroesNum = 0;
	size_t currentZeroesStart = 0;

	for (size_t i = 0; i < 8; ++i)
	{
		segments[i] = (bytes[i * 2 + 1] | (bytes[i * 2] << 8)) & 0xFFFFu;

		if (segments[i] == 0)
		{
			currentZeroesNum++;
			if (i == 0 || segments[i - 1] != 0) currentZeroesStart = i;
		}

		if ((i > 0 && segments[i] != 0 && segments[i - 1] == 0) || i == 7)
		{
			if (currentZeroesNum > mostZeroesNum)
			{
				mostZeroesNum = currentZeroesNum;
				mostZeroesStart = currentZeroesStart;
				currentZeroesStart = 0;
				currentZeroesNum = 0;
			}
		}
	}

	bool ipv4mapped = segments[0] == 0x0000 &&
	                  segments[1] == 0x0000 &&
	                  segments[2] == 0x0000 &&
	                  segments[3] == 0x0000 &&
	                  segments[4] == 0x0000 &&
	                  segments[5] == 0xFFFF;

	char buffer[TS_IPV6_MAX_LEN] = { 0 };
	char tempBuffer[16] = { 0 };

	bool prevContracted = false;
	for (size_t i = 0; i < 8; ++i)
	{
		bool contracted = (i >= mostZeroesStart && i < mostZeroesStart + mostZeroesNum);
		if (!contracted)
		{
			if (i > 0 && !prevContracted) strcat(buffer, ":");

			if (!ipv4mapped || i < 6)
			{
				sprintf(tempBuffer, "%hx", segments[i]);
				strcat(buffer, tempBuffer);
			}
			else
			{
				sprintf(tempBuffer, "%hhu.%hhu.%hhu.%hhu",
					uint8_t((segments[6] >> 8) & 0xFFu),
					uint8_t( segments[6]       & 0xFFu),
					uint8_t((segments[7] >> 8) & 0xFFu),
					uint8_t( segments[7]       & 0xFFu)
				);
				strcat(buffer, tempBuffer);
				break;
			}
		}
		else if (!prevContracted)
		{
			strcat(buffer, "::");
		}

		prevContracted = contracted;
	}

	strncpy(outBuffer, buffer, outBufferSize);
}

void printAddress(char *buffer, const Address &address)
{
	switch(address.type)
	{
		case Address::AddressTypeIPv4:
		{
			sprintf(buffer, "%hhu.%hhu.%hhu.%hhu:%hu", address.ip[0], address.ip[1], address.ip[2], address.ip[3], address.port);
		}
		return;

		case Address::AddressTypeIPv6:
		{
			char ipv6buffer[TS_IPV6_MAX_LEN];
			printIPv6(ipv6buffer, TS_IPV6_MAX_LEN, address.ip);
			sprintf(buffer, "[%s]:%hu", ipv6buffer, address.port);
		}
		return;
	}
}

void printAddressWithoutPort(char *buffer, const Address &address)
{
	switch(address.type)
	{
		case Address::AddressTypeIPv4:
		{
			sprintf(buffer, "%hhu.%hhu.%hhu.%hhu", address.ip[0], address.ip[1], address.ip[2], address.ip[3]);
		}
		return;

		case Address::AddressTypeIPv6:
		{
			printIPv6(buffer, TS_IPV6_MAX_LEN, address.ip);
		}
		return;
	}
}

bool validateIPAddress(const char* str, Address::AddressType type)
{
	switch(type)
	{
	case Address::AddressTypeIPv4:
	{
		uint8_t temp[4];
		uint16_t temp2;
		return sscanf(str, "%hhu.%hhu.%hhu.%hhu:%hu", &temp[0], &temp[1], &temp[2], &temp[3], &temp2) == 5 ||
		       sscanf(str, "%hhu.%hhu.%hhu.%hhu", &temp[0], &temp[1], &temp[2], &temp[3]) == 4;
	}

	case Address::AddressTypeIPv6:
	{
		uint8_t temp[16];
		return parseIPv6(str, temp);
	}

	default:
		return false;
	}
}

bool extractIPStringAndPort(const char *str, char* outAddress, char* outPort, Address::AddressType* outAddressType)
{
	TS_ASSERT(strlen(str) < TS_IPV6_WITH_PORT_MAX_LEN);
	TS_ASSERT(outAddress != nullptr);

	char address[TS_IPV6_MAX_LEN];
	memset(address, 0, TS_IPV6_MAX_LEN);

	char port[6] = { 0 };

	bool isValid = false;
	Address::AddressType type = Address::AddressTypeIPv4;

	const char* openBracket = strchr(str, '[');
	const char* closeBracket = strchr(str, ']');

	if (!openBracket && !closeBracket)
	{
		TS_ASSERT(strlen(str) < TS_IPV6_MAX_LEN);
		strcpy(address, str);

		isValid = validateIPAddress(address, Address::AddressTypeIPv4);
		if (isValid)
		{
			type = Address::AddressTypeIPv4;

			// Remove and copy port from IPv4 string if one exists
			char* colon = strchr(address, ':');
			if (colon)
			{
				colon[0] = '\0';
				strncpy(port, colon + 1, 5);
			}
		}
		else
		{
			isValid = validateIPAddress(address, Address::AddressTypeIPv6);
			if (isValid) type = Address::AddressTypeIPv6;
		}
	}
	else if (openBracket && closeBracket && openBracket < closeBracket)
	{
		TS_ASSERT((closeBracket - openBracket - 1) < TS_IPV6_MAX_LEN);
		memcpy(address, openBracket + 1, static_cast<size_t>(closeBracket - openBracket - 1));

		isValid = validateIPAddress(address, Address::AddressTypeIPv6);
		if (isValid)
		{
			type = Address::AddressTypeIPv6;
			strncpy(port, closeBracket + 2, 5);
		}
	}

	if (isValid)
	{
		strcpy(outAddress, address);
		if (outPort) strcpy(outPort, port);
		if (outAddressType) *outAddressType = type;
		return true;
	}

	return false;
}

bool resolveAddress(const char* hostname, Address &address, ResolveAddressType preferredType)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	switch (preferredType)
	{
		case ResolveAddressType::ResolveAddressTypeIPv4: hints.ai_family = AF_INET; break;
		case ResolveAddressType::ResolveAddressTypeIPv6: hints.ai_family = AF_INET6; break;
		default: hints.ai_family = AF_UNSPEC; break;
	}

	addrinfo *result = nullptr;
	DWORD r = getaddrinfo(hostname, nullptr, &hints, &result);
	if (r != 0)
	{
		TS_PRINTF("getaddrinfo failed %d\n", r);
		return false;
	}

	bool found = false;

	addrinfo *ptr = nullptr;
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		switch (ptr->ai_family)
		{
			case AF_INET:
			{
				SOCKADDR_IN addr = *reinterpret_cast<SOCKADDR_IN*>(ptr->ai_addr);
				address = Address(
					Address::AddressTypeIPv4,
					reinterpret_cast<uint8_t*>(&addr.sin_addr.S_un.S_un_b),
					ntohs(addr.sin_port)
				);
				found = true;
			}
			break;
			case AF_INET6:
			{
				SOCKADDR_IN6 addr = *reinterpret_cast<SOCKADDR_IN6*>(ptr->ai_addr);
				address = Address(
					Address::AddressTypeIPv6,
					addr.sin6_addr.s6_addr,
					ntohs(addr.sin6_port)
					);
				found = true;
			}
			break;
			default: break;
		}
		if (found) break;
	}

	freeaddrinfo(result);
	return true;
}

TS_END_PACKAGE1()
