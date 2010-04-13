#pragma once

#include "XmlParser.h"
using namespace XML;

/**
 * Stream operators for parser.
 */
template<class T>
inline bool operator >> (Parser &parser, T &t)
{
	return !parser ? false : xml_handle(parser.GetRoot(), t);
}

template<class T>
inline bool operator >> (const CString &xml, T &t)
{
	if (xml.IsEmpty())
		return false;

	Parser parser(xml);
	return parser >> t;
}

