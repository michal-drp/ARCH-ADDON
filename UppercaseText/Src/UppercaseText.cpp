#include "APIEnvir.h"
#include "ACAPinc.h"
#include "APICommon.h"

// -----------------------------------------------------------------------------
// Uppercase selected text
// -----------------------------------------------------------------------------
static void Do_UppercaseText ()
{
    ACAPI_CallUndoableCommand ("Uppercase Selected Text", [&] () -> GSErrCode {
        API_SelectionInfo   selectionInfo;
        GS::Array<API_Neig> selNeigs;

        GSErrCode err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
        if (err != NoError) return err;

        for (const API_Neig& neig : selNeigs) {
            API_ElemType type = Neig_To_ElemID (neig.neigID);
            if (type == API_TextID || type == API_LabelID) {
                API_Element element {};
                API_Element mask {};
                API_ElementMemo memo {};

                element.header.type = type;
                element.header.guid = neig.guid;

                if (ACAPI_Element_Get (&element) == NoError) {
                    bool processText = false;
                    if (type == API_TextID) {
                        processText = true;
                    } else if (type == API_LabelID && element.label.labelClass == APILblClass_Text) {
                        processText = true;
                    }

                    if (processText && ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_TextContent) == NoError) {
                        if (memo.textContent != nullptr) {
                            GS::UniString text = *memo.textContent;
                            text = text.ToUpperCase ();
                            delete memo.textContent;
                            memo.textContent = new GS::UniString (text);

                            ACAPI_ELEMENT_MASK_CLEAR (mask);
                            ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_TextContent, true);
                        }
                        ACAPI_DisposeElemMemoHdls (&memo);
                    }
                }
            }
        }
        return NoError;
    });
}

// -----------------------------------------------------------------------------
// MenuCommandHandler
// -----------------------------------------------------------------------------
GSErrCode MenuCommandHandler (const API_MenuParams *params)
{
    switch (params->menuItemRef.itemIndex) {
        case 1: Do_UppercaseText (); break;
    }
    return NoError;
}

// -----------------------------------------------------------------------------
// Required Archicad API Functions
// -----------------------------------------------------------------------------
API_AddonType CheckEnvironment (API_EnvirParams* envir)
{
    RSGetIndString (&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule ());
    RSGetIndString (&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule ());
    return APIAddon_Normal;
}

GSErrCode RegisterInterface (void)
{
    ACAPI_MenuItem_RegisterMenu (32500, 0, MenuCode_UserDef, MenuFlag_Default);
    return NoError;
}

GSErrCode Initialize (void)
{
    return ACAPI_MenuItem_InstallMenuHandler (32500, MenuCommandHandler);
}

GSErrCode FreeData (void)
{
    return NoError;
}
