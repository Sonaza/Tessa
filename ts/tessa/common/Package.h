#pragma once

#define TS_PACKAGE0()					namespace ts {
#define TS_PACKAGE1(ns1)				namespace ts { namespace ns1 {
#define TS_PACKAGE2(ns1, ns2)			namespace ts { namespace ns1 { namespace ns2 {
#define TS_PACKAGE3(ns1, ns2, ns3)		namespace ts { namespace ns1 { namespace ns2 { namespace ns3 {
#define TS_PACKAGE4(ns1, ns2, ns3, ns4)	namespace ts { namespace ns1 { namespace ns2 { namespace ns3 { namespace ns4 {

#define TS_END_PACKAGE0() }
#define TS_END_PACKAGE1() } }
#define TS_END_PACKAGE2() } } }
#define TS_END_PACKAGE3() } } } }
#define TS_END_PACKAGE4() } } } } }
