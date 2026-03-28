#include "APIEnvir.h"
#include "ACAPinc.h"
#include "APICommon.h"

// -----------------------------------------------------------------------------
// Helper to Uppercase while ignoring <tags> (Autotexts)
// -----------------------------------------------------------------------------
static GS::UniString SafeUpperCase (const GS::UniString& input)
{
    GS::UniString result;
    bool inTag = false;
    UIndex lastStart = 0;

    for (UIndex i = 0; i < input.GetLength (); i++) {
        if (input[i] == '<' && !inTag) {
            if (i > lastStart) {
                GS::UniString part = input.GetSubstring (lastStart, i - lastStart);
                result += part.ToUpperCase ();
            }
            inTag = true;
            lastStart = i;
        } else if (input[i] == '>' && inTag) {
            inTag = false;
            GS::UniString part = input.GetSubstring (lastStart, i - lastStart + 1);
            result += part;
            lastStart = i + 1;
        }
    }

    if (lastStart < input.GetLength ()) {
        GS::UniString part = input.GetSubstring (lastStart, input.GetLength () - lastStart);
        if (!inTag) {
            result += part.ToUpperCase ();
        } else {
            result += part;
        }
    }

    return result;
}

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

                    if (type == API_TextID || (type == API_LabelID && element.label.labelClass == APILblClass_Text)) {
                        if (ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_TextContent) == NoError) {
                            if (memo.textContent != nullptr) {
                                GS::UniString text = *memo.textContent;
                                text = SafeUpperCase (text);
                                delete memo.textContent;
                                memo.textContent = new GS::UniString (text);

                                ACAPI_ELEMENT_MASK_CLEAR (mask);
                                ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_TextContent, true);
                            }
                            ACAPI_DisposeElemMemoHdls (&memo);
                        }
                    } else if (type == API_LabelID && element.label.labelClass == APILblClass_Symbol) {
                        if (ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_AddPars) == NoError) {
                            if (memo.params != nullptr) {
                                Int32 nPars = (Int32) (BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType));
                                API_AddParType* pars = (API_AddParType*) *memo.params;
                                for (Int32 i = 0; i < nPars; i++) {
                                    if (pars[i].typeID == APIParT_CString && pars[i].typeMod == API_ParSimple) {
                                        GS::UniString val (pars[i].value.uStr);
                                        val = SafeUpperCase (val);
                                        GS::ucscpy (pars[i].value.uStr, (const GS::uchar_t*) val.ToUStr ());
                                    }
                                }
                                ACAPI_ELEMENT_MASK_CLEAR (mask);
                                ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_AddPars, true);
                            }
                            ACAPI_DisposeElemMemoHdls (&memo);
                        }
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
