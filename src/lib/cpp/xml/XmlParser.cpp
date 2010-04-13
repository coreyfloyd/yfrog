#include "StdAfx.h"
#include "XmlParser.h"

namespace XML
{

Node::Node(IXMLDOMNode *pDOMNode)
	: node(pDOMNode)
{
}

Node::~Node()
{
}

Node &Node::operator = (IXMLDOMNode *pDOMNode)
{
	node = pDOMNode;
	return *this;
}

bool Node::operator ! () const
{
	return node == NULL;
}

DOMNodeType Node::GetNodeType() const
{
	DOMNodeType type;
	if (!node || FAILED(node->get_nodeType(&type)))
		return NODE_INVALID;

	return type;
}

Node Node::SelectSingleNode(const CString &xPath)
{
	Node selected;
	if (!node || FAILED(node->selectSingleNode((CComBSTR)xPath, &selected.node)))
		return Node();

	return selected;
}

bool Node::IsAttribute() const
{
	DOMNodeType type;
	if (!node || FAILED(node->get_nodeType(&type)))
		return false;

	return NODE_ATTRIBUTE == type;
}

bool Node::HasChildren() const
{
	VARIANT_BOOL vbHasChildNodes = VARIANT_FALSE;
	if (!node || FAILED(node->hasChildNodes(&vbHasChildNodes)))
		return false;

	return VARIANT_TRUE == vbHasChildNodes;
}

CString Node::GetName() const
{
	CComBSTR bsName;
	if (!node || FAILED(node->get_nodeName(&bsName)) || !bsName)
		return CString();
	
	return (CString)bsName;
}

bool Node::GetValue(CStringA &value) const
{
    CStringW strValue;
    if (!GetValue(strValue))
        return false;

    value = (CStringA)strValue;
    return true;
}

bool Node::GetValue(CStringW &value) const
{
	value.Empty();

	if (!node)
		return false;

	DOMNodeType type = GetNodeType();
	if (NODE_TEXT == type || NODE_ATTRIBUTE == type)
	{
		CComVariant vtValue;
		if (FAILED(node->get_nodeValue(&vtValue)))
			return false;

		if (VT_EMPTY == vtValue.vt)
			return true;

		if (vtValue.vt != VT_BSTR && FAILED(vtValue.ChangeType(VT_BSTR)))
			return false;
			
		value = (CStringW)vtValue.bstrVal;
	}
	else if (NODE_ELEMENT == type)
	{
		for (Iterator::ChildIterator it(*this); (bool)it; ++it)
		{
			if (NODE_TEXT != it->GetNodeType())
				continue;

			CStringW strValue;
			if (it->GetValue(strValue))
				value += strValue;
		}
	}
	else
	{
		//TODO: get value from other node type
		return false;
	}

	value.Trim();

	return true;
}

bool Node::GetValue(int &value) const
{
	value = 0;

	CString strValue;
	if (!GetValue(strValue) || strValue.IsEmpty())
		return false;

	return 1 == _stscanf_s(strValue, _T("%i"), &value);
}

bool Node::GetValue(DWORD &value) const
{
	value = 0;

	CString strValue;
	if (!GetValue(strValue) || strValue.IsEmpty())
		return false;

	return 1 == _stscanf_s(strValue, _T("%u"), &value);
}

bool Node::GetValue(ULONGLONG &value) const
{
	value = 0;

	CString strValue;
	if (!GetValue(strValue) || strValue.IsEmpty())
		return false;

	return 1 == _stscanf_s(strValue, _T("%I64u"), &value);
}

bool Node::GetValue(LONGLONG &value) const
{
	value = 0;

	CString strValue;
	if (!GetValue(strValue) || strValue.IsEmpty())
		return false;

	return 1 == _stscanf_s(strValue, _T("%I64i"), &value);
}

namespace Iterator
{
	//////////////////////////////////////////////////////////////////////////

	AbstractIterator::AbstractIterator()
		: index(0), node(NULL)
	{
	}

	Node* AbstractIterator::operator -> ()
	{
		return &node;
	}

	Node& AbstractIterator::operator * ()
	{
		return node;
	}

	void AbstractIterator::reset()
	{
		index = 0;
		node.node.Release();
	}

	//////////////////////////////////////////////////////////////////////////

	ChildIterator::ChildIterator(const Node &node)
	{
		if (!node.HasChildren())
			return;

		const CComPtr<IXMLDOMNode> &domNode = node.node;
		if (!domNode || FAILED(domNode->get_childNodes(&children)))
			return;

		if (children) children->get_item(index, &AbstractIterator::node.node);
	}

	ChildIterator::operator bool () const
	{
		long length = 0;
		if (!children || FAILED(children->get_length(&length)))
			return false;

		return index < length;
	}

	void ChildIterator::operator ++ ()
	{
		long length = 0;
		if (!children || FAILED(children->get_length(&length)))
			return;

		AbstractIterator::node.node.Release();

		if (++index < length)
			children->get_item(index, &AbstractIterator::node.node);
	}

	void ChildIterator::reset()
	{
		AbstractIterator::reset();
		if (children) children->get_item(index, &AbstractIterator::node.node);
	}

	//////////////////////////////////////////////////////////////////////////

	AttributeIterator::AttributeIterator(const Node &node)
		: attributes(NULL)
	{
		const CComPtr<IXMLDOMNode> &domNode = node.node;
		if (!domNode || FAILED(domNode->get_attributes(&attributes)))
			return;

		if (attributes) attributes->get_item(index, &AbstractIterator::node.node);
	}

	AttributeIterator::operator bool () const
	{
		long length = 0;
		if (!attributes || FAILED(attributes->get_length(&length)))
			return false;

		return index < length;
	}

	void AttributeIterator::operator ++ ()
	{
		long length = 0;
		if (!attributes || FAILED(attributes->get_length(&length)))
			return;

		AbstractIterator::node.node.Release();

		if (++index < length)
			attributes->get_item(index, &AbstractIterator::node.node);
	}

	void AttributeIterator::reset()
	{
		AbstractIterator::reset();
		if (attributes) attributes->get_item(index, &AbstractIterator::node.node);
	}

	//////////////////////////////////////////////////////////////////////////

}// namespace Iterator

///////////////////////////////////////////////////////////////////////////////
Parser::Parser()
{
	document.CoCreateInstance( __uuidof(DOMDocument) );
}

Parser::Parser(const CString &xml)
{
	document.CoCreateInstance( __uuidof(DOMDocument) );

	ParseString(xml);
}

bool Parser::ParseString(const CString &xml)
{
	ATLASSERT(document.p != NULL);

	// Load the XML document file...
	VARIANT_BOOL bSuccess = VARIANT_FALSE;
	if (!document || FAILED(document->loadXML((CComBSTR)xml, &bSuccess )))
		return false;

	return VARIANT_TRUE == bSuccess;
}

bool Parser::ParseFile(LPCTSTR pszFileName)
{
	ATLASSERT(document.p != NULL);

	// Load the XML document file...
	VARIANT_BOOL bSuccess = VARIANT_FALSE;
	if (!document || FAILED(document->load((CComVariant)pszFileName, &bSuccess)))
		return false;

	return VARIANT_TRUE == bSuccess;
}

Node Parser::GetRoot()
{
	VARIANT_BOOL vbHasChildNodes = VARIANT_FALSE;
	if (!document || FAILED(document->hasChildNodes(&vbHasChildNodes)) || VARIANT_TRUE != vbHasChildNodes)
		return Node();

	CComPtr<IXMLDOMNodeList> children;
	if (FAILED(document->get_childNodes(&children)))
		return Node();

	long length = 0;
	if (!children || FAILED(children->get_length(&length)) || !length)
		return Node();

	if (1 == length)
	{
		Node node;
		if (FAILED(children->get_item(0, &node.node)))
			return Node();
		return node;
	}

	for (int i = 0; i < length; ++i)
	{
		Node node;
		if (FAILED(children->get_item(i, &node.node)) || !node)
			continue;

		if (NODE_ELEMENT != node.GetNodeType())
			continue;

		return node;
	}
	
	return Node();
}

Node Parser::SelectSingleNode(const CString &xPath)
{
	Node node;
	if (!document || FAILED(document->selectSingleNode((CComBSTR)xPath, &node.node)))
		return Node();

	return node;
}

bool Parser::operator ! () const
{
	return !document;
}

} // namespace XML
