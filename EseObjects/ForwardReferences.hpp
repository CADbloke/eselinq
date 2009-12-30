//various forward references needed by components in EseObjects

ref class TableID;
ref class Table;
ref class Cursor;
ref class Column;
ref class Key;

JET_TABLEID GetTableTableID(Table ^Tab);
JET_SESID GetTableSesid(Table ^Tab);
TableID ^GetTableIDObj(Table ^Tab);

JET_TABLEID GetCursorTableID(Cursor ^Csr);
JET_SESID GetCurosrSesid(Cursor ^Csr);

///<summary>Extra options for Unicode mapping. Represents JET_UNICODEINDEX.dwMapFlags. See Win32 LCMapMapString for option details. LCMAP_SORTKEY must always be included.</summary>
public value struct UnicodeMapFlags
{
	///<summary>Mandatory to be set to True</summary>
	bool SortKey;
	bool ByteReverse;
	bool IgnoreCase;
	bool IgnoreNonSpace;
	bool IgnoreSymbols;
	bool IgnoreKanaType;
	bool IgnoreWidth;
	bool StringSort;
};

JET_GRBIT UnicodeMapFlagsToBits(UnicodeMapFlags o)
{
	//bool converts to either 1 for true or 0 for false
	//multiplying to either identity of the bitmask or 0
	JET_GRBIT b = 0;
	b |= o.SortKey * LCMAP_SORTKEY;
	b |= o.ByteReverse * LCMAP_BYTEREV;
	b |= o.IgnoreCase * NORM_IGNORECASE;
	b |= o.IgnoreNonSpace * NORM_IGNORENONSPACE;
	b |= o.IgnoreSymbols * NORM_IGNORESYMBOLS;
	b |= o.IgnoreKanaType * NORM_IGNOREKANATYPE;
	b |= o.IgnoreWidth * NORM_IGNOREWIDTH;
	b |= o.StringSort * SORT_STRINGSORT;
	return b;
}

UnicodeMapFlags UnicodeMapFlagsFromBits(JET_GRBIT b)
{
	UnicodeMapFlags o;
	o.SortKey = b & LCMAP_SORTKEY;
	o.ByteReverse = b & LCMAP_BYTEREV;
	o.IgnoreCase = b & NORM_IGNORECASE;
	o.IgnoreNonSpace = b & NORM_IGNORENONSPACE;
	o.IgnoreSymbols = b & NORM_IGNORESYMBOLS;
	o.IgnoreKanaType = b & NORM_IGNOREKANATYPE;
	o.IgnoreWidth = b & NORM_IGNOREWIDTH;
	o.StringSort = b & SORT_STRINGSORT;
	return o;
}

///<summary>Single column value pair.</summary>
public value struct Field
{
	///<summary>Column the field belongs to.</summary>
	Column ^Col;
	///<summary>Value of the field.</summary>
	Object ^Val;

	Field(Column ^Col, Object ^Val) :
		Col(Col),
		Val(Val)
	{}
};