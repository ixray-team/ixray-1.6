#pragma once
#include "../xrScripts/script_export_space.h"

struct CPhraseDialogExporter
{
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CPhraseDialogExporter)
#undef script_type_list
#define script_type_list save_type_list(CPhraseDialogExporter)
