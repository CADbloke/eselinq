//PCH header with system and common definitions
#include "stdafx.h"

#include "free_list.hpp"

void *JET_API jet_realloc_cpp(void *context, void *buff, ulong length)
{
	if(buff)
		delete(buff);
	
	if(length)
		return new char[length];
	
	return 0;
}

namespace EseObjects
{

//max size used for alloca
size_t const ESEOBJECTS_MAX_ALLOCA = 64;

#include "ForwardReferences.hpp"
#include "EseVersion.hpp"
#include "EseException.hpp"
#include "DemandLoadFunction.hpp"
#include "FieldDataConversion.hpp"
#include "Instance.hpp"
#include "Session.hpp"
#include "Database.hpp"
#include "Transaction.hpp"
#include "TableID.hpp"
#include "Column.hpp"
#include "Positioning.hpp"
#include "Key.hpp"
#include "Index.hpp"
#include "Bookmark.hpp"
#include "SecondaryBookmark.hpp"
#include "Cursor.hpp"
#include "Table.hpp"

}
