#pragma once

#define TS_IPV6_WITH_PORT_MAX_LEN 54
#define TS_IPV6_MAX_LEN 46

TS_PACKAGE1(net)
	
/**
* Address used by sockets
*/
class Address
{
public:
	enum AddressType
	{
		AddressTypeIPv4,
		AddressTypeIPv6,
	};

	Address();

	/* Sets the IP address to the given bytes according to given type.
	 *
	 * Number of bytes to be copied is derived from the given type:
	 *   if type == AddressTypeIPv4, inBytes must be 4 bytes in size.
	 *   if type == AddressTypeIPv6, inBytes must be 16 bytes in size.
	 */
	Address(const AddressType type, const uint8_t *inBytes, const uint16_t port = 0, const uint32_t virtualPort = 0);

	Address(const std::string &address);

	// First 4 bytes are used when type is IPv4, all 16 when type is IPv6
	uint8_t ip[16];
	uint16_t port;
	uint32_t virtualPort;
	AddressType type;

	// Retrieves the 4 IPv4 bytes of as 32-bit uint (does not work for IPv6 addresses)
	uint32_t getIPv4AsInt() const;

	// Sets the 4 IPv4 bytes of an address from 32-bit uint (does not work for IPv6 addresses)
	void setIPv4FromInt(uint32_t i);

	/* Copies current address bytes to the output byte array.
	 * Current address type and given address type must match.
	 *
	 * Number of bytes to be copied is derived from the given address type:
	 *   if type == AddressTypeIPv4, outBytes must be 4 bytes in size.
	 *   if type == AddressTypeIPv6, outBytes must be 16 bytes in size.
	 */
	void retrieveBytes(uint8_t *outBytes, const AddressType type) const;

	/* Sets current address type and copies given bytes.
	 *
	 * Number of bytes to be copied is derived from the given address type:
	 *   if type == AddressTypeIPv4, inBytes must be 4 bytes in size.
	 *   if type == AddressTypeIPv6, inBytes must be 16 bytes in size.
	 */
	void setBytes(const AddressType type, const uint8_t *inBytes);

	/* Converts an IPv4 address to IPv4-mapped IPv6 format
	 * Example:
	 *    IPv4: 172.26.12.156
	 *      becomes
	 *    IPv6: ::ffff:172.26.12.156
	 */
	void convertToIPv6();

	std::string getHostName();

	/* Returns true if current address is IPv4-mapped IPv6 address.
	 */
	bool isIPv4mappedIPv6() const;

	/* Constructs char buffer out of the IP and then tries to construct type T object with it.
	  */
	template<class T> T toString() const;

	bool operator!=(const Address &other) const;
	bool operator==(const Address &other) const;

	static const size_t NumBytesAddressIPv4 = 4;
	static const size_t NumBytesAddressIPv6 = 16;
};

/* Converts an IPv4 address to IPv4-mapped IPv6 format
 * Example:
 *    IPv4: 172.26.12.156
 *      becomes
 *    IPv6: ::ffff:172.26.12.156
 */
void convertToIPv6(const Address &originalIPv4, Address *newIPv6);

/* Parses a given address string.
 * The function attempts to derive address type and sets the output address type as such.
 *
 * Parse supports IPv4 and IPv6 addresses (including contracted and IPv4-mapped forms)
 *
 * Returns true if parse was successful.
 */
bool parseAddress(const char *str, Address &address);

/* Parses a given address string without port.
 * The function attempts to derive address type and sets the output address type as such.
 *
 * Parse supports IPv4 and IPv6 addresses (including contracted and IPv4-mapped forms)
 * Resulting address will not have port set.
 *
 * Returns true if parse was successful.
 */
bool parseAddressWithoutPort(const char *str, Address &address);

/* Converts port value from string to 16-bit uint.
 * 
 * Returns true if parse was successful.
 */
bool parsePort(const char *str, uint16_t &port);

/* Outputs given address to the output buffer.
 * Will print both IPv4 and IPv6 addresses. IPv6 addresses are printed in contracted form.
 *
 * Buffer must be at least 54 (FB_IPV6_WITH_PORT_MAX_LEN) characters in length including the terminating null byte.
 */
void printAddress(char *outBuffer, const Address &address);

/* Outputs given address without port to the output buffer.
 * Will print both IPv4 and IPv6 addresses. IPv6 addresses are printed in contracted form.
 *
 * Buffer must be at least 46 (FB_IPV6_MAX_LEN) characters in length including the terminating null byte.
 */
void printAddressWithoutPort(char *outBuffer, const Address &address);

enum ResolveAddressType
{
	ResolveAddressTypeIPv4 = Address::AddressTypeIPv4,
	ResolveAddressTypeIPv6 = Address::AddressTypeIPv6,
	ResolveAddressTypeDontCare,
};

/* Resolves a given host name as ip address.
 * 
 * Set preferredType to the type of ip address you wish to receive.
 * If preferredType is set to ResolveAddressTypeDontCare, resolve may return both IPv4 and IPv6 addresses.
 *
 * Returns true if resolve was successful.
 */
bool resolveAddress(const char *hostname, Address &address, ResolveAddressType preferredType = ResolveAddressType::ResolveAddressTypeDontCare);

/* Validates IP address by trying to parse it as a given type.
 *
 * Returns true if parse was successful and the address is valid.
 */
bool validateIPAddress(const char* addressString, Address::AddressType type);

/* Extracts IP address and port strings separately from a given address string.
 * If address string does not have port, resulting port string will be left empty.
 *
 * If outAddressType != NULL, the type value is set to the recognised address type.
 *
 * outAddress buffer must be at least 46 (FB_IPV6_MAX_LEN) characters in size.
 * outPort buffer must be at least 6 characters in size.
 *
 * Returns true if extraction was successful.
 */
bool extractIPStringAndPort(const char *addressString, char* outAddress, char* outPort, Address::AddressType* outAddressType = NULL);

template<class T>
T Address::toString() const
{
	char buffer[64];
	printAddress(buffer, *this);
	return T(buffer);
}

TS_END_PACKAGE1()
