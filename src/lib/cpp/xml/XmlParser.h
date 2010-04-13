#pragma once

/**
 * XML processor package.
 *
 * @author Alexander Kozlov
 */
namespace XML
{
	// class declaration
	namespace Iterator
	{
		class AbstractIterator;
		class AttributeIterator;
		class ChildIterator;
	}

	/**
	 * IXMLDOMNode wrapper
	 */
	class Node
	{
		// friends
 		friend class Parser;
		friend class Iterator::AbstractIterator;
		friend class Iterator::AttributeIterator;
		friend class Iterator::ChildIterator;

	protected:
		Node(IXMLDOMNode *pDOMNode = NULL);

	public:
		~Node();

	public:
		CString GetName() const;
		DOMNodeType GetNodeType() const;

		Node SelectSingleNode(const CString &xPath);

		bool GetValue(CStringA &value) const;
		bool GetValue(CStringW &value) const;
		bool GetValue(ULONGLONG &value) const;
		bool GetValue(LONGLONG &value) const;
		bool GetValue(DWORD &value) const;
		bool GetValue(int &value) const;

		bool IsAttribute() const;
		bool HasChildren() const;

		bool operator ! () const;

	protected:
		Node& operator = (IXMLDOMNode *pDOMNode);

	protected:
		CComPtr<IXMLDOMNode> node;
	};

	namespace Iterator
	{
		class AbstractIterator
		{
		protected:
			AbstractIterator();
		public:
			// Node Access
			Node *operator -> ();
			Node &operator *  ();
		protected:
			// Reset to begin
			void reset();
		protected:
			int index;
			// Temp Node To Access
			Node node;
		};

		class ChildIterator : public AbstractIterator
		{
		public:
			ChildIterator(const Node &node);
			// Checking for end of list
			operator bool () const;
			// Iterate
			void operator ++ ();
			// Reset to begin
			void reset();
		private:
			// NodeList
			CComPtr<IXMLDOMNodeList> children;
		};

		class AttributeIterator : public AbstractIterator
		{
		public:
			AttributeIterator(const Node &node);
			// Checking for end of list
			operator bool () const;
			// Iterate
			void operator ++ ();
			// Reset to begin
			void reset();
		private:
			// Attributes
			CComPtr<IXMLDOMNamedNodeMap> attributes;
		};
	}

    /**
     * XML parser wrapper
     */
	class Parser
	{
	public:
		Parser();
		Parser(const CString &xml);

		bool ParseFile(LPCTSTR pszFileName);
		bool ParseString(const CString &xml);

		Node GetRoot();
		Node SelectSingleNode(const CString &xPath);

		bool operator ! () const;

	private:
		CComPtr<IXMLDOMDocument> document;
	};

	/**
	 * Handles simple/standard values
	 */
	inline bool xml_handle(Node &node, int &value)
	{
		return node.GetValue(value);
	}

	inline bool xml_handle(Node &node, DWORD &value)
	{
		return node.GetValue(value);
	}

	inline bool xml_handle(Node &node, ULONGLONG &value)
	{
		return node.GetValue(value);
	}

	inline bool xml_handle(Node &node, LONGLONG &value)
	{
		return node.GetValue(value);
	}

	inline bool xml_handle(Node &node, CStringA &value)
	{
		return node.GetValue(value);
	}

	inline bool xml_handle(Node &node, CStringW &value)
	{
		return node.GetValue(value);
	}

	inline bool xml_handle(Node &node, bool &value)
	{
		CString strValue;
		if (!node.GetValue(strValue))
			return false;

		ATLASSERT(strValue == _T("0") 
					|| strValue == _T("1") 
					|| !strValue.CompareNoCase( _T("true") ) 
					|| !strValue.CompareNoCase( _T("false") ));

		value = strValue == _T("1") || !strValue.CompareNoCase( _T("true") );

		return true;
	}

    inline bool xml_handle(CStringA &strResult, LPCSTR pszValue)
    {
	    strResult = pszValue;
	    return true;
    }

    inline bool xml_handle(CStringW &strResult, LPCWSTR pszValue)
    {
	    strResult = pszValue;
	    return true;
    }

    inline bool xml_handle(CStringA &strResult, const CStringA &strValue)
    {
	    strResult = strValue;
	    return true;
    }

    inline bool xml_handle(CStringW &strResult, const CStringW &strValue)
    {
	    strResult = strValue;
	    return true;
    }

	/**
	 * Handles arrays
	 */
	template<typename E, typename ETraits>
	inline bool xml_handle(Node &node, CAtlArray<E, ETraits> &values)
	{
		size_t size = values.GetCount();
		values.SetCount(size + 1);
		if (xml_handle(node, values[size]))
			return true;
		values.SetCount(size);
		return false;
	}

	template<typename T, typename TEqual>
	inline bool xml_handle(Node &node, CSimpleArray<T, TEqual> &values)
	{
		T tValue;
		if (!xml_handle(node, tValue))
			return false;

		values.Add(tValue);
		return true;
	}

	/**
	 * Handles as map
	 */
	template<typename K, typename V>
	inline bool xml_handle(Node &node, CAtlMap<K, V> &values)
	{
		return xml_handle(node, values[node.GetName()]);
	}

	/**
	 * Handles as map
	 */
	template<typename K, typename V>
	inline bool xml_handle(Node &node, CAtlMap<K, V>* &rpValues)
	{
		if (!rpValues)
			rpValues = new CAtlMap<K, V>();

		return rpValues ? xml_handle(node, (*rpValues)[node.GetName()]) : false;
	}

	/**
	 * Processes compound object
	 */
	template<typename T>
	bool xml_process(Node &node, T &t)
	{
		bool processed = false;
		for (Iterator::AttributeIterator it(node); (bool)it; ++it)
			if (xml_map(*it, t)) processed = true;

		for (Iterator::ChildIterator it(node); (bool)it; ++it)
			if (xml_map(*it, t)) processed = true;

		return processed;
	}

	/**
	 * Macroses for mapping.
	 */
	#define XML_SERIALIZE_HANDLE(class_name) \
		namespace XML { inline bool xml_handle(Node &node, class_name &value)\
							{ return xml_process(node, value); } }

	#define XML_SERIALIZE_MAP(class_name) \
		XML_SERIALIZE_HANDLE(class_name) \
		namespace XML { inline bool xml_map(Node &node, class_name &value)\
						{\
							bool bMap = false;\
							CString name(node.GetName());

	#define XML_SERIALIZE_MAP_EX(class_name, base_class) \
		XML_SERIALIZE_HANDLE(class_name) \
		namespace XML { inline bool xml_map(Node &node, class_name &value)\
						{\
							bool bMap = xml_handle(node, (base_class&)value);\
							CString name(node.GetName());

	#define XML_MAP_ELEMENT(element)			 if (name == _T(#element)) return xml_handle(node, value.element);
	#define XML_MAP_ELEMENT2(element, member)	 if (name == _T(element)) return xml_handle(node, value.member);
	#define XML_MAP_ATTRIBUTE(attribute)		 if (node.IsAttribute() && name == _T(#attribute)) return xml_handle(node, value.attribute);
	#define XML_MAP_ATTRIBUTE2(attribute, member) if (node.IsAttribute() && name == _T(attribute)) return xml_handle(node, value.member);
	#define XML_MAP_ALL_ATTRIBUTES(attributes)	 if (node.IsAttribute()) return xml_handle(node, value.attributes);
	#define XML_MAP_TEXT(element)			     return xml_handle(node, value.element);
    #define XML_MAP_PROCESS_ELEMENT(element)	 if (name == _T(element)) return xml_process(node, value); 

	#define XML_END_SERIALIZE_MAP() \
				return bMap;\
		}}

    // Copy enum value
    inline void xml_enum_string(CString &strResult, LPCTSTR pszValue)
	    { strResult = pszValue; }

    // Empty stub to compile templates
    inline void xml_enum_string(Node&, LPCTSTR)
	    { }

    #define XML_ENUM_TYPE_MAP(enum_name) \
        namespace XML {\
	    template <typename X>/*map template declaration*/ \
	    bool xml_map(X &x, enum_name &enum_value, bool bExport = false); \
	    /* specialized handlers */ \
	    inline bool xml_handle(Node &node, enum_name &enum_value) \
		    { return xml_map(node, enum_value); } \
	    inline bool xml_handle(CString &strResult, const enum_name &enum_value) \
		    { return xml_map(strResult, (enum_name &)enum_value, true); } \
	    inline bool xml_handle(Node &node, enum_name* &enum_value) \
		    { \
			    if (!enum_value) enum_value = new enum_name; \
			    return xml_handle(node, *enum_value); \
		    } \
	    inline bool xml_handle(CString &strResult, const enum_name* enum_value) \
		    { return enum_value ? xml_handle(strResult, *enum_value) : false; } \
	    /* map template */ \
	    template <typename X>\
	    inline bool xml_map(X &x, enum_name &enum_value, bool bExport/* = false*/)\
		    {\
			    CString strValue; \
			    if (!bExport && (!xml_handle(x, strValue) || strValue.IsEmpty())) \
				    return false;

    #define XML_ENUM_VALUE(value)\
			    else if (!bExport && !strValue.CompareNoCase( _T(#value) ))\
				    enum_value = value;\
			    else if (bExport && enum_value == value)\
				    xml_enum_string(x, _T(#value));

    #define XML_ENUM_VALUE2(value, string_value)\
			    else if (!bExport && !strValue.CompareNoCase( _T(string_value) ))\
				    enum_value = value;\
			    else if (bExport && enum_value == value)\
				    xml_enum_string(x, _T(string_value));

    #define XML_END_ENUM_TYPE_MAP() \
			    else\
			    {\
				    ATLTRACE( _T("Unsupported enum value: %s\n"), strValue);\
				    ATLASSERT(FALSE);\
				    xml_enum_string(x, _T(""));\
				    return false;\
			    }\
			    \
			    return true; \
            }}

}// namespace XML
