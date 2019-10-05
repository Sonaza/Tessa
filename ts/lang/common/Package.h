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

#define TS_DECLARE0(_name)						namespace ts { class _name; }
#define TS_DECLARE1(ns1, _name)					namespace ts { namespace ns1 { class _name; } }
#define TS_DECLARE2(ns1, ns2, _name)			namespace ts { namespace ns1 { namespace ns2 { class _name; } } }
#define TS_DECLARE3(ns1, ns2, ns3, _name)		namespace ts { namespace ns1 { namespace ns2 { namespace ns3 { class _name; } } } }
#define TS_DECLARE4(ns1, ns2, ns3, ns4, _name)	namespace ts { namespace ns1 { namespace ns2 { namespace ns3 { namespace ns4 { class _name; } } } } }

#define TS_DECLARE_STRUCT0(_name)						namespace ts { struct _name; }
#define TS_DECLARE_STRUCT1(ns1, _name)					namespace ts { namespace ns1 { struct _name; } }
#define TS_DECLARE_STRUCT2(ns1, ns2, _name)				namespace ts { namespace ns1 { namespace ns2 { struct _name; } } }
#define TS_DECLARE_STRUCT3(ns1, ns2, ns3, _name)		namespace ts { namespace ns1 { namespace ns2 { namespace ns3 { struct _name; } } } }
#define TS_DECLARE_STRUCT4(ns1, ns2, ns3, ns4, _name)	namespace ts { namespace ns1 { namespace ns2 { namespace ns3 { namespace ns4 { struct _name; } } } } }
