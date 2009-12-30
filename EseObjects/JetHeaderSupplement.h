//This header exists to provide retro versions of items that can't work in older versions of ESE with a newer version specified.

#if defined(_M_AMD64) || defined(_M_IA64)
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif

//This struct has an extra member in 6.0+ which screws up array alignment when calling into a <6.0 library.
//The version defined below is JET_INDEXCREATE as defined for 5.2
typedef struct tagJET_INDEXCREATE_SHORT
	{
	unsigned long			cbStruct;				// size of this structure (for future expansion)
	char					*szIndexName;			// index name
	char					*szKey;					// index key definition
	unsigned long			cbKey;					// size of key definition in szKey
	JET_GRBIT				grbit;					// index options
	unsigned long			ulDensity;				// index density

	union
		{
		unsigned long		lcid;					// lcid for the index (if JET_bitIndexUnicode NOT specified)
		JET_UNICODEINDEX	*pidxunicode;			// pointer to JET_UNICODEINDEX struct (if JET_bitIndexUnicode specified)
		};

	union
		{
		unsigned long		cbVarSegMac;			// maximum length of variable length columns in index key (if JET_bitIndexTupleLimits specified)
		JET_TUPLELIMITS		*ptuplelimits;			// pointer to JET_TUPLELIMITS struct (if JET_bitIndexTupleLimits specified)
		};

	JET_CONDITIONALCOLUMN_A	*rgconditionalcolumn;	// pointer to conditional column structure
	unsigned long			cConditionalColumn;		// number of conditional columns
	JET_ERR					err;					// returned error code
	} JET_INDEXCREATE_SHORT;

#include <poppack.h>
