/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

//	UFormElementFactory.cp

#include "UFormElementFactory.h"

// PowerPlant
#include <LPane.h>
#include <LCommander.h>
#include <UReanimator.h>
#include <UMemoryMgr.h>
//#include <LEventHandler.h>
//#include <LStyleSet.h>
//#include <VStyleSet.h>

// MacFE
#include "CHTMLView.h"
#include "CBrowserContext.h"
#include "CEditorWindow.h"
#include "CURLDispatcher.h"
#include "resgui.h"
#include "uprefd.h"
#include "mforms.h"
#include "CConfigActiveScroller.h"
#include "CHyperScroller.h"
#include "macgui.h"
#include "macutil.h"
#include "ufilemgr.h"
#include "uintl.h"
#include "CAutoPtr.h"

// Backend
#include "lo_ele.h"	// for struct def'n
#include "xlate.h"
#include "fe_proto.h"
#include "prefapi.h"
#include "edt.h"

// macros
#define FEDATAPANE	((FormFEData *)formElem->element_data->ele_minimal.FE_Data)->fPane	
#define FEDATAHOST 	((FormFEData *)formElem->element_data->ele_minimal.FE_Data)->fHost
// These defines add extra spacing to the controls.
// The spacing is needed because the layout puts them too close otherwise
#define buttonTopExtra 1
#define buttomLeftExtra 1
#define buttonRightExtra 1
#define buttonBottomExtra 1

#define textTopExtra 1
#define textLeftExtra 1
#define textRightExtra 1
#define textBottomExtra 1

void FE_FreeFormElement(
	MWContext* 				/* inContext */,
	LO_FormElementData *	inFormData)
{
	UFormElementFactory::FreeFormElement(inFormData);
}

// moose: temporary hack until we fix up lo_ele.h and ntypes.h to have an HTMLarea struct
typedef struct lo_FormElementTextareaData_struct lo_FormElementHTMLareaData;

// prototypes
Boolean SetFE_Data(LO_FormElementStruct *formElem, LPane * pane, LFormElement *host, LCommander *commander);
Boolean HasFormWidget(LO_FormElementStruct *formElem);

// utilities
inline  FormFEData *GetFEData(LO_FormElementStruct *formElem) { return ((FormFEData *)formElem->element_data->ele_minimal.FE_Data); }
inline	LPane *GetFEDataPane( LO_FormElementStruct *formElem) { return FEDATAPANE; } 

// Sets the value of FE_Data structure in form element
// Returns true if form is being created from scratch
Boolean SetFE_Data(
	LO_FormElementStruct* formElem,
	LPane* pane,
	LFormElement *host,
	LCommander *commander)
{
	if (!formElem->element_data)
		return false;
	FormFEData *feData;
	Boolean noFEData = ( formElem->element_data->ele_minimal.FE_Data == NULL );
	
	if ( noFEData )
	{
		FormFEData* feData = new FormFEData;
		ThrowIfNil_( feData );
		formElem->element_data->ele_minimal.FE_Data = feData;
	}

	feData = (FormFEData *)formElem->element_data->ele_minimal.FE_Data;	
	feData->fPane = pane;
	feData->fHost = host;
	feData->fCommander = commander;
	
	Assert_(host);
	if (host)
		host->SetFEData(feData);
	return noFEData;
}

Boolean HasFormWidget(
	LO_FormElementStruct *formElem)
{
	return ((formElem->element_data->ele_minimal.FE_Data != NULL) &&
			(FEDATAPANE != NULL));
}

/******************************************************************************
 *	CWhiteScroller 
 * White background instead of gray 'wscr'
 ******************************************************************************/
class CWhiteScroller: public CConfigActiveScroller
{
public:
	//friend class CHyperView;		// this class no longer exists
	
					enum { class_ID = 'wscr' };

					CWhiteScroller( LStream* inStream );	
	virtual Boolean	FocusDraw(LPane* inSubPane = nil);
	virtual void	DrawSelf();
};

CWhiteScroller::CWhiteScroller(LStream *inStream)
	:	CConfigActiveScroller(inStream)
{
}

// The only real routine. Sets background to white
Boolean CWhiteScroller::FocusDraw(LPane* /*inSubPane*/)
{
	if (LScroller::FocusDraw())
	{
		UGraphics::SetBack(CPrefs::White);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CWhiteScroller::DrawSelf()
{
	Rect	frame;
	if (CalcLocalFrameRect(frame) && IsVisible() && 
		mUpdateRgnH && ((*mUpdateRgnH)->rgnBBox.top != (*mUpdateRgnH)->rgnBBox.bottom))	// Only erase on update
		::EraseRect(&frame);
		
//	if ((mVerticalBar && mVerticalBar->IsVisible()) || (mHorizontalBar && mHorizontalBar->IsVisible()))
//		::EraseRect(&frame);

	LScroller::DrawSelf();	
}

/*********************************************************************
 * StModifyPPob
 * Modifying PPob for dynamic change of the text resource ID when loading
 * Constructor modifies the resource by sticking a resID at the appropriate
 * offset.
 * Destructor restores it.
 * Offsets are generated by looking them up in Resourcerer
 *********************************************************************/
class StModifyPPob {
	Int16 fResID;
	Int16 fOffset;
	Int16 fOldTextR;
public:
	StModifyPPob(Int16 resID, // Resource of the PPob to be modified
				Int16 offset, // Offset into the resource
				Int16 csid,		// Charset of the current doc
				Boolean button);	// Which TxtR are we looking for (TRUE = button, FALSE = text)
	~StModifyPPob();
};

StModifyPPob::StModifyPPob(Int16 resID, Int16 offset, Int16 csid, Boolean button)
{
	fResID = resID;
	fOffset = offset;

	Int16 newTextR;
	if (button)
		newTextR = CPrefs::GetButtonFontTextResIDs(csid);
	else 
		newTextR = CPrefs::GetTextFieldTextResIDs(csid);
	
	Handle r = ::GetResource('PPob', fResID);
	ThrowIfNil_(r);
	HNoPurge(r);
	// Modify the resource
	StHandleLocker lock(r);
	Int16 * myPointer = (Int16*)(*r + fOffset);
	fOldTextR = *myPointer;
	*myPointer = newTextR;
}

StModifyPPob::~StModifyPPob()
{
	Handle r = ::GetResource('PPob', fResID);
	StHandleLocker lock(r);
	Int16 * myPointer = (Int16*)(*r + fOffset);;
	*myPointer = fOldTextR;
	HPurge(r);
}

void UFormElementFactory::RegisterFormTypes()
{
	RegisterClass_( CFormButton);
	RegisterClass_( CGAFormPushButton);
	RegisterClass_( CFormList);
	RegisterClass_( CFormLittleText);
	RegisterClass_( CFormBigText);
	RegisterClass_( CFormHTMLArea);
	RegisterClass_( CFormRadio);
	RegisterClass_( CGAFormRadio);
	RegisterClass_( CFormCheckbox);
	RegisterClass_( CGAFormCheckbox);
	RegisterClass_( CFormFile);
	RegisterClass_( CFormFileEditField);
	RegisterClass_( CWhiteScroller);
}

//
// Figures out what kind of form element we're dealing with and dispatches *that*
//
void UFormElementFactory::MakeFormElem(
	CHTMLView* inHTMLView,
	CNSContext*	inNSContext,
	LO_FormElementStruct* formElem)
{
	if (!formElem->element_data)
		return;
	Try_	{
	Int32		width;
	Int32		height;
	Int32		baseline;

	switch ( formElem->element_data->type )
	{
		case FORM_TYPE_TEXT:
		case FORM_TYPE_PASSWORD:
		case FORM_TYPE_ISINDEX:
			MakeTextFormElem(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;
		case FORM_TYPE_READONLY:
			MakeReadOnlyFormElem(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;
		
		case FORM_TYPE_TEXTAREA:
			MakeTextArea(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;
		
#ifdef ENDER
		case FORM_TYPE_HTMLAREA:
			MakeHTMLArea(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;
#endif /*ENDER*/
		
		case FORM_TYPE_RADIO:
		case FORM_TYPE_CHECKBOX:
			MakeToggle(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;
		
		case FORM_TYPE_HIDDEN:
		case FORM_TYPE_KEYGEN:
		case FORM_TYPE_OBJECT:
			width = height = baseline = 0;
			break;
		
		case FORM_TYPE_SUBMIT:
		case FORM_TYPE_RESET:
		case FORM_TYPE_BUTTON:
			MakeButton(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;

		case FORM_TYPE_FILE:
			MakeFilePicker(inHTMLView, inNSContext, width, height, baseline, formElem);
			break;

		case FORM_TYPE_SELECT_ONE:
			MakePopup(inHTMLView, inNSContext, width, height, baseline,formElem);
			break;
		
		case FORM_TYPE_SELECT_MULT:
			MakeList(inHTMLView, inNSContext, width, height, baseline, formElem);
		break;
		
		case FORM_TYPE_IMAGE:
		case FORM_TYPE_JOT:
		default:
			Assert_(FALSE);
		break;
	};
	
	formElem->width = width;
	formElem->height = height;
	formElem->baseline = baseline;
	}
	Catch_(inErr)
	{
		Assert_(FALSE);
	}
	EndCatch_
}

// Creates a simple edit text field
// textData->size is the number of characters
// textData->max_size is the maximum number of characters
// textData->default_text is the default text
LPane*	UFormElementFactory::MakeTextFormElem(
	CHTMLView* inHTMLView,
	CNSContext*	inNSContext,
	Int32 &width, Int32 &height, Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	CFormLittleText * editField;
	if (!HasFormWidget(formElem))
	{
	// If we did not have a control, create one
		lo_FormElementTextData * textData = (lo_FormElementTextData *) formElem->element_data;
		LCommander::SetDefaultCommander(inHTMLView);	// For tabbing
		LPane::SetDefaultView(inHTMLView);
		switch (textData->type)	{
			case FORM_TYPE_TEXT:
			case FORM_TYPE_ISINDEX:
			{
				StModifyPPob modifier(formTextField, 0x3F, inNSContext->GetWinCSID(), false);
				editField = (CFormLittleText *)UReanimator::ReadObjects('PPob', formTextField);
			}
				break;
			case FORM_TYPE_PASSWORD:
				editField = (CFormLittleText *)UReanimator::ReadObjects('PPob', formPasswordField);
				break;
			default:
				Throw_(0);
		}
		ThrowIfNil_(editField);
	
		editField->InitFormElement(*inNSContext, formElem);

		editField->FinishCreate();
		editField->AddListener(inHTMLView);
		if (textData->max_size < 0)
			textData->max_size = 32000;
		editField->SetMaxChars(textData->max_size);
		editField->SetVisibleChars(textData->size);
		editField->SetLayoutForm(formElem);
		// Should allow undo
		LUndoer* theUndoer = new LUndoer;
		editField->AddAttachment(theUndoer);

	// Reset the value
		ResetFormElement(formElem, FALSE, SetFE_Data(formElem,editField,editField, editField));

	// Secure fields are disabled
	}
	else
	{
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		editField = (CFormLittleText *)FEDATAPANE;
		if (!editField)
			return NULL;
		editField->SetLayoutForm(formElem);		// CWhiteEdit field only (never uses it...)
		editField->SetLayoutElement(formElem);	// mocha mix-in needs to know too...
	}
	
// Figure out dimensions
	SDimension16 size;
	editField->GetFrameSize(size);
	width = size.width + textLeftExtra + textRightExtra;
	height = size.height + textTopExtra + textBottomExtra;;
	TEHandle textH = editField->GetMacTEH();
	baseline = (*textH)->fontAscent + 2 + textTopExtra;
	return editField;
}

// Very similar to MakeTextFormElem, for now
LPane * UFormElementFactory::MakeReadOnlyFormElem(
	CHTMLView* inHTMLView,
	CNSContext*	inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	CFormLittleText * editField;
	if (!HasFormWidget(formElem))
	{
	// If we did not have a control, create one
		lo_FormElementMinimalData * readData = (lo_FormElementMinimalData *) formElem->element_data;
		LCommander::SetDefaultCommander(inHTMLView);	// For tabbing
		LPane::SetDefaultView(inHTMLView);
		StModifyPPob modifier(formTextField, 0x3F, inNSContext->GetWinCSID(), false);
		editField = (CFormLittleText *)UReanimator::ReadObjects('PPob', formTextField);
		ThrowIfNil_(editField);

		editField->InitFormElement(*inNSContext, formElem);
		editField->FinishCreate();
		editField->SetLayoutForm(formElem);
		editField->Disable();

	// Reset the value
		ResetFormElement(formElem, FALSE, SetFE_Data(formElem,editField,editField, editField));
		char * newText;
		PA_LOCK(newText, char* , readData->value);
		editField->SetDescriptor(CStr255(newText));
		editField->SetVisibleChars(newText ? XP_STRLEN(newText) : 10);
		PA_UNLOCK(readData->value);

	}
	else
	{
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//

		editField = (CFormLittleText *)FEDATAPANE;
		if (!editField)
			return NULL;
		editField->SetLayoutForm(formElem);
		editField->SetLayoutElement(formElem);
	}
	
	
	
// Figure out dimensions
	SDimension16 size;
	editField->GetFrameSize(size);
	width = size.width + textLeftExtra + textRightExtra;
	height = size.height + textTopExtra + textBottomExtra;;
	TEHandle textH = editField->GetMacTEH();
	baseline = (*textH)->fontAscent + 2 + textTopExtra;
	return editField;
}

#define BigTextLeftIndent 5	// same constants are in the forms.r
#define BigTextRightIndent 1
#define BigTextTopIndent 1
#define BigTextBottomIndent 1
// Creates a text area
//	textAreaData->default_text;
//	textAreaData->rows;
//	textAreaData->cols;
LPane* UFormElementFactory::MakeTextArea(
	CHTMLView* inHTMLView,
	CNSContext*	inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	CWhiteScroller * theScroller = NULL;
	CFormBigText * theTextView = NULL;
	FontInfo fontInfo;
	if (!HasFormWidget(formElem))				// If there is no form, create it
		{	
		lo_FormElementTextareaData * textAreaData = (lo_FormElementTextareaData *) formElem->element_data;
	
		// Create the view
		LCommander::SetDefaultCommander(inHTMLView);
		LView::SetDefaultView(inHTMLView);
	
		// since I can't find an API to switch the word wrap property after the element
		// has been initialized, we need to create two separate PPob's. Read in the
		// appropriate one depending on if the text area requires wrapping or not.
		ResIDT elementToRead = 0;
		switch(textAreaData->auto_wrap) {
			case TEXTAREA_WRAP_SOFT:
			case TEXTAREA_WRAP_HARD:
				elementToRead = formBigText;
				break;
				
			case TEXTAREA_WRAP_OFF:
				elementToRead = formBigTextScroll;
				break;
				
		} // case of wrap property
				
		theScroller = (CWhiteScroller *)UReanimator::ReadObjects('PPob', elementToRead);
		ThrowIfNil_(theScroller);
		theScroller->FinishCreate();
		theScroller->PutInside(inHTMLView);
	
		theTextView = (CFormBigText*)theScroller->FindPaneByID(formBigTextID);
		Assert_(theTextView != NULL);

		LModelObject* theSuper = inHTMLView->GetFormElemBaseModel();
		theTextView->InitFormElement(*inNSContext, formElem);

		// Add the undoer for text actions.  This will be on a per-text field basis
		//	- Let WASTE handle undo
		//	LUndoer* theUndoer = new LUndoer;
		//	theTextView->AddAttachment(theUndoer);
		
		// Resize to proper size
		theTextView->FocusDraw();
		::GetFontInfo(&fontInfo);
//		short wantedWidth = textAreaData->cols * fontInfo.widMax + 8;
		short wantedWidth = textAreaData->cols * ::CharWidth('M') + 8;
		short wantedHeight = textAreaData->rows * (fontInfo.ascent + fontInfo.descent + fontInfo.leading) + 8;
		
		// make the image big so there is something to scroll, if necessary....
		if ( textAreaData->auto_wrap == TEXTAREA_WRAP_OFF )
			theTextView->ResizeImageTo(2000, 0, false);
		
		theScroller->ResizeFrameTo(wantedWidth + 16 + BigTextLeftIndent + BigTextRightIndent,
								   wantedHeight + 16 + BigTextTopIndent + BigTextBottomIndent,
								   FALSE);
								   
	// Set the default values.
		ResetFormElement(formElem, FALSE, SetFE_Data(formElem, theScroller, theTextView, theTextView));

		theScroller->Show();
		}
	else
		{
		theScroller = (CWhiteScroller*)FEDATAPANE;
		if (!theScroller)
			return NULL;
		theTextView = (CFormBigText*)theScroller->FindPaneByID(formBigTextID);
		theTextView->FocusDraw();
		
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		theTextView->SetLayoutElement(formElem);
		::GetFontInfo(&fontInfo);
		}		
	
	SDimension16 size;
	theScroller->GetFrameSize(size);
	width = size.width + textLeftExtra + textRightExtra;;
	height = size.height + textTopExtra + textBottomExtra;;
	baseline = fontInfo.ascent + 2;
	
	return theScroller;
}

// Creates an HTML area
//	htmlAreaData->default_text;
//	htmlAreaData->rows;
//	htmlAreaData->cols;
LPane* UFormElementFactory::MakeHTMLArea(
	CHTMLView* inHTMLView,
	CNSContext*	inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	CHyperScroller * theScroller = NULL;
	CFormHTMLArea * theHTMLAreaView = NULL;
	FontInfo fontInfo;
	if (!HasFormWidget(formElem))				// If there is no form, create it
	{	
		lo_FormElementHTMLareaData * htmlAreaData = (lo_FormElementHTMLareaData *) formElem->element_data;
	
		// Create the view
		LCommander::SetDefaultCommander(inHTMLView);
		LView::SetDefaultView(inHTMLView);
	
		theScroller = (CHyperScroller *)UReanimator::ReadObjects('PPob', formHTMLArea);
		ThrowIfNil_(theScroller);
		theScroller->FinishCreate();
		theScroller->PutInside(inHTMLView);
	
		theHTMLAreaView = (CFormHTMLArea*)theScroller->FindPaneByID(formHTMLAreaID);
		ThrowIfNil_(theHTMLAreaView);

		URL_Struct* url = 0;	
		url = NET_CreateURLStruct ("about:editfilenew", NET_NORMAL_RELOAD );

		CBrowserContext *nscontext = new CBrowserContext();
		ThrowIfNil_(nscontext);
		theHTMLAreaView->SetContext(nscontext);
		
		MWContext *context = *nscontext;
		context->is_editor = true;

		CURLDispatcher::DispatchURL( url, nscontext, false, false, CEditorWindow::res_ID );
		
		LModelObject* theSuper = inHTMLView->GetFormElemBaseModel();
		theHTMLAreaView->InitFormElement(*inNSContext, formElem);

		// Resize to proper size
		theHTMLAreaView->FocusDraw();
		::GetFontInfo(&fontInfo);
		short wantedWidth = htmlAreaData->cols * ::CharWidth('M') + 8;
		short wantedHeight = htmlAreaData->rows * (fontInfo.ascent + fontInfo.descent + fontInfo.leading) + 8;
		
		theScroller->ResizeFrameTo(wantedWidth + 16 + BigTextLeftIndent + BigTextRightIndent,
								   wantedHeight + 16 + BigTextTopIndent + BigTextBottomIndent,
								   FALSE);
								   
	// Set the default values.
		ResetFormElement(formElem, FALSE, SetFE_Data(formElem, theScroller, theHTMLAreaView, theHTMLAreaView));

		theScroller->Show();
	}
	else
	{
		theScroller = (CHyperScroller*)FEDATAPANE;
		if (!theScroller)
			return NULL;
		theHTMLAreaView = (CFormHTMLArea*)theScroller->FindPaneByID(formHTMLAreaID);
		theHTMLAreaView->FocusDraw();
		
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		theHTMLAreaView->SetLayoutElement(formElem);
	}		
	
	SDimension16 size;
	theScroller->GetFrameSize(size);
	width = size.width + textLeftExtra + textRightExtra;;
	height = size.height + textTopExtra + textBottomExtra;;
	baseline = fontInfo.ascent + 2;
	
	return theScroller;
}

// Creates a button
//	buttonData->name; is buttons name
// Buttons store the form element as the refCon, so that they can broadcast
// to the window.
LPane* UFormElementFactory::MakeButton(
	CHTMLView* inHTMLView,
	CNSContext* inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	LControl*		theControl = nil;
	LPane*			thePane = nil;
	LFormElement*	theHost = nil;
	lo_FormElementMinimalData * buttonData= (lo_FormElementMinimalData *)formElem->element_data;
	FontInfo info;
	
	if (!HasFormWidget(formElem))
	{
		// FIX ME?
		LCommander::SetDefaultCommander(LWindow::FetchWindowObject(inHTMLView->GetMacPort()));
		LPane::SetDefaultView(inHTMLView);
		switch (buttonData->type)	{
			case FORM_TYPE_SUBMIT:
				{
					XP_Bool	useGrayscaleFormControls;
					int		prefResult = PREF_GetBoolPref("browser.mac.use_grayscale_form_controls", &useGrayscaleFormControls);
					
					if (prefResult == PREF_NOERROR && useGrayscaleFormControls)
					{
						StModifyPPob modifier(formGASubmitButton, 0x46, inNSContext->GetWinCSID(), TRUE);
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formGASubmitButton));
					}
					else
					{
						StModifyPPob modifier(formSubmitButton, 0x46, inNSContext->GetWinCSID(), TRUE);
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formSubmitButton));
					}
				}
				break;
			case FORM_TYPE_RESET:
				{
					XP_Bool	useGrayscaleFormControls;
					int		prefResult = PREF_GetBoolPref("browser.mac.use_grayscale_form_controls", &useGrayscaleFormControls);
					
					if (prefResult == PREF_NOERROR && useGrayscaleFormControls)
					{
						StModifyPPob modifier(formGAResetButton, 0x46, inNSContext->GetWinCSID(), TRUE);
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formGAResetButton));
					}
					else
					{
						StModifyPPob modifier(formResetButton, 0x46, inNSContext->GetWinCSID(), TRUE);
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formResetButton));
					}
				}
				break;
			case FORM_TYPE_BUTTON:
				{
					XP_Bool	useGrayscaleFormControls;
					int		prefResult = PREF_GetBoolPref("browser.mac.use_grayscale_form_controls", &useGrayscaleFormControls);
					
					if (prefResult == PREF_NOERROR && useGrayscaleFormControls)
					{
						StModifyPPob modifier(formGAPlainButton, 0x46, inNSContext->GetWinCSID(), TRUE);
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formGAPlainButton));
					}
					else
					{
						StModifyPPob modifier(formPlainButton, 0x46, inNSContext->GetWinCSID(), TRUE);
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formPlainButton));
					}
				}
				break;
			default:
				Throw_(0);
		}
		
		ThrowIfNil_(theHost = dynamic_cast<LFormElement*>(thePane));
		ThrowIfNil_(theControl = dynamic_cast<LControl*>(thePane));

		thePane->FinishCreate();
		theHost->InitFormElement(*inNSContext, formElem);

		ResetFormElement(formElem, FALSE, SetFE_Data(formElem,thePane,theHost, NULL));
	// Set up buttons as broadcasters. Store the LO_FormElementStruct * in refCon
		theControl->AddListener(inHTMLView);
	}
	else
	{
		thePane = FEDATAPANE;
		ThrowIfNil_(theHost = dynamic_cast<LFormElement*>(thePane));
		ThrowIfNil_(theControl = dynamic_cast<LControl*>(thePane));
		
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		theHost->SetLayoutElement(formElem);
	}
	
	thePane->SetUserCon((long)formElem); // Always do this
	
	char * name;
	PA_LOCK(name, char*, buttonData->value);
	CStr255 pName(name);
	thePane->SetDescriptor(pName);
	PA_UNLOCK(buttonData->value);
	::GetFontInfo(&info);
	
// Get width/height/baseline
	SDimension16 size;
	thePane->GetFrameSize(size);
	width = size.width + buttomLeftExtra + buttonRightExtra;
	height = size.height + buttonTopExtra + buttonBottomExtra;
	baseline = info.ascent + 2 + buttonTopExtra;
	return thePane;
}

LPane* UFormElementFactory::MakeFilePicker(
	CHTMLView* inHTMLView,
	CNSContext* inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	CFormFile * filePicker = NULL;
	lo_FormElementTextData * pickerData= (lo_FormElementTextData *)formElem->element_data;
	
	if (!HasFormWidget(formElem))
	{
		// FIX ME?
		LCommander::SetDefaultCommander(LWindow::FetchWindowObject(inHTMLView->GetMacPort()));
		LPane::SetDefaultView(inHTMLView);
		filePicker = (CFormFile *)UReanimator::ReadObjects('PPob', formFilePicker);
		ThrowIfNil_(filePicker);

		filePicker->InitFormElement(*inNSContext, formElem);

		filePicker->FinishCreate();
		ResetFormElement(formElem, FALSE, SetFE_Data(formElem,filePicker,filePicker, filePicker->fEditField));
	}
	else
	{
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		filePicker = (CFormFile *)FEDATAPANE;
		if (!filePicker)
			return NULL;
		filePicker->SetLayoutElement(formElem);
	}

	
/*
	char * name;
	PA_LOCK(name, char*, pickerData->default_text);
	CStr255 pName(name);
	filePicker->SetDescriptor(pName);
	PA_UNLOCK(pickerData->default_text);
	::GetFontInfo(&info);
*/
	
// Get width/height/baseline
	filePicker->GetFontInfo(width, height, baseline);
	width = width + buttomLeftExtra + buttonRightExtra;
	height = height + buttonTopExtra + buttonBottomExtra;
	return filePicker;
}

// Creates buttons/checkboxes
// Boolean toggleData->default_toggle is default on/off
LPane* UFormElementFactory::MakeToggle(
	CHTMLView* inHTMLView,
	CNSContext* inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
		
	LControl*	toggle = nil;
	LPane*		thePane;
	LFormElement *host;
	lo_FormElementToggleData * toggleData = (lo_FormElementToggleData *)formElem->element_data;
	if (!HasFormWidget(formElem))
	{
		// FIX ME?
		LCommander::SetDefaultCommander(LWindow::FetchWindowObject(inHTMLView->GetMacPort()));
		LPane::SetDefaultView(inHTMLView);
		switch (toggleData->type)
		{
			case FORM_TYPE_RADIO:
				{
					XP_Bool	useGrayscaleFormControls;
					int		prefResult = PREF_GetBoolPref("browser.mac.use_grayscale_form_controls", &useGrayscaleFormControls);
					
					if (prefResult == PREF_NOERROR && useGrayscaleFormControls)
					{
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formGARadio));
					}
					else
					{
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formRadio));
					}
				}
				break;
			case FORM_TYPE_CHECKBOX:
				{
					XP_Bool	useGrayscaleFormControls;
					int		prefResult = PREF_GetBoolPref("browser.mac.use_grayscale_form_controls", &useGrayscaleFormControls);
					
					if (prefResult == PREF_NOERROR && useGrayscaleFormControls)
					{
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formGACheckbox));
					}
					else
					{
						ThrowIfNil_(thePane = (LPane *)UReanimator::ReadObjects('PPob', formCheckbox));
					}
				}
				break;
			default:
				Throw_(0);
		}

		ThrowIfNil_(host	= dynamic_cast<LFormElement*>(thePane));
		ThrowIfNil_(toggle	= dynamic_cast<LControl*>(thePane));

		host->InitFormElement(*inNSContext, formElem);
			
		thePane->FinishCreate();
		thePane->Enable();

		ResetFormElement(formElem, FALSE, SetFE_Data(formElem,toggle, host, NULL ));
			
		// Set up toggles as broadcasters. Store the LO_FormElementStruct * in refCon
//		toggle->AddListener(this);

	}
	else
	{
		LFormElement*	formWidget;
		
		lo_FormElementToggleData* toggleData = (lo_FormElementToggleData *)formElem->element_data;
		toggle = dynamic_cast<LControl*>(FEDATAPANE);
		if (!toggle)
			return NULL;
		
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		if (toggleData->type == FORM_TYPE_RADIO)				// fun with C++ casting
		{
			formWidget = dynamic_cast<LFormElement*>(toggle);
			ThrowIfNil_(formWidget);
		}
		else if (toggleData->type == FORM_TYPE_CHECKBOX)
		{
			formWidget = dynamic_cast<LFormElement*>(toggle);
			ThrowIfNil_(formWidget);
		}
		else
		{
			Assert_(false);
		}
		
		formWidget->SetLayoutElement(formElem);
	}
	
	toggle->SetUserCon((long)formElem);

	short extra;	// extra space to be left after the control
	switch (toggleData->type)	{
		case FORM_TYPE_RADIO:
			extra = 3;
			break;
		case FORM_TYPE_CHECKBOX:
			extra = 5;
			break;
	}
// width/height/baseline
	SDimension16 size;
	toggle->GetFrameSize(size);
	width = size.width + extra;
	height = size.height;
	baseline = size.height - 2;
	return toggle;
}

LPane* UFormElementFactory::MakePopup (
	CHTMLView* inHTMLView,
	CNSContext* inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	if (!HasFormWidget(formElem))
	{
		lo_FormElementSelectData * selections = (lo_FormElementSelectData *)formElem->element_data;
		// Create the popup
		LCommander::SetDefaultCommander(inHTMLView);
		LPane::SetDefaultView(inHTMLView);
		
		LPane* thePane = (LPane*)(UReanimator::ReadObjects ('PPob', formGAPopup));

		CGAPopupMenu* popupCtrl = dynamic_cast<CGAPopupMenu*>(thePane);
		ThrowIfNil_(popupCtrl);
		
		popupCtrl->SetTextTraits(CPrefs::GetButtonFontTextResIDs(inNSContext->GetWinCSID()));

		popupCtrl->FinishCreate();

		// pointer from host widget to layout
		popupCtrl->SetUserCon ((Int32)selections);
		// tell popup what data to display
		FormsPopup * popupText = new FormsPopup (popupCtrl, selections);
		ThrowIfNil_(popupText);
		
		popupText->InitFormElement(*inNSContext, formElem);
		
		popupCtrl->AddAttachment (popupText);
		// reset values
		ResetFormElement(formElem, FALSE, SetFE_Data(formElem,popupCtrl,popupText, NULL ));			// Reset the selection
#if 1	/*	POPUP MENU VERTICAL POSITION FIX */
		FontInfo fontInfo;
		GetFontInfo (&fontInfo);
#endif
		// Resize the popup to max width
		short widgetBaseline;
		Point popupSize = popupText->CalcTargetFrame (widgetBaseline);
		popupCtrl->ResizeFrameTo (popupSize.h, popupSize.v, false);
		width = popupSize.h;
		height = popupSize.v + 1; // add 1 for "leading" between lines in forms
#if 1	/*	POPUP MENU VERTICAL POSITION FIX */
		baseline = fontInfo.ascent + 1;
#else
		baseline = widgetBaseline;
#endif
		
		popupCtrl->Show();
		popupCtrl->Enable();

		return popupCtrl;
	}
	else
	{
		LPane*			widget 		= dynamic_cast<LPane*>(FEDATAPANE);
		FormsPopup* 	popupText 	= dynamic_cast<FormsPopup*>(FEDATAHOST);
		SDimension16 	size;
		
		ThrowIfNil_(widget);
		ThrowIfNil_(popupText);
		
#if 1	/*	POPUP MENU VERTICAL POSITION FIX */
		FontInfo fontInfo;
		GetFontInfo (&fontInfo);
#endif
		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		if (popupText != NULL)
			popupText->SetLayoutElement(formElem);
			
		if (widget != NULL)
			widget->GetFrameSize(size);
		width = size.width;
		height = size.height + 1; // add 1 for "leading" between lines in forms
#if 1	/*	POPUP MENU VERTICAL POSITION FIX */
		baseline = fontInfo.ascent + 1;
#else
		baseline = 2;	// ERROR. Does not get the proper baseline
#endif
		return widget;
	}
}

LPane* UFormElementFactory::MakeList(
	CHTMLView* inHTMLView,
	CNSContext* inNSContext,
	Int32 &width,
	Int32 &height,
	Int32& baseline,
	LO_FormElementStruct *formElem)
{
	if (!formElem->element_data)
		return nil;
	CFormList * myList = NULL; // (CFormList*)FEDATAPANE;
	if (!HasFormWidget(formElem))
	{
		lo_FormElementSelectData* selectData = (lo_FormElementSelectData *)formElem->element_data;
		// Create the form
		LCommander::SetDefaultCommander( inHTMLView );
		LPane::SetDefaultView( inHTMLView );
		StModifyPPob modifier( formScrollingList, 0x3C, inNSContext->GetWinCSID(), false );
		myList = (CFormList *)UReanimator::ReadObjects( 'PPob', formScrollingList );
		ThrowIfNil_( myList );

		myList->InitFormElement( *inNSContext, formElem );
		myList->FinishCreate();

		// Initialize the list items from the form data
		myList->SetSelectMultiple( selectData->multiple );
		myList->SyncRows( selectData->option_cnt );
		
		ResetFormElement( formElem, FALSE, 	SetFE_Data( formElem, myList, myList, myList) );			// Reset the selection
		myList->ShrinkToFit( selectData->size );
	}
	else
	{
		myList = (CFormList*)FEDATAPANE;
		if (!myList)
			return NULL;
		myList->FocusDraw();

		//
		// We've already created a widget for the
		// form element, but layout has gone and
		// reallocated the form element, so we must
		// update our reference to it.
		//
		myList->SetLayoutElement(formElem);
	}
	
	FontInfo		info;
	SDimension16	size;

	::GetFontInfo( &info );
	myList->GetFrameSize( size );

	// +6 to account for the focus ring
	width	= size.width + 6;
	height 	= size.height + 6;

	baseline = info.ascent;
	return myList;
}

// FreeFormElement is called when eric finally wants to free the FE form data
void UFormElementFactory::FreeFormElement(
	LO_FormElementData *formElem)
{
	XP_TRACE(("FreeFormElement, data %d \n", (Int32)formElem->ele_minimal.FE_Data));
	switch (formElem->type) {
		case FORM_TYPE_TEXT:
		case FORM_TYPE_PASSWORD:
		case FORM_TYPE_ISINDEX:		// Sets the original text
		case FORM_TYPE_FILE:
			if (formElem->ele_text.current_text)
				PA_FREE(formElem->ele_text.current_text);
			formElem->ele_text.current_text = NULL;
			FormFEData * FEData = (FormFEData *)formElem->ele_text.FE_Data;
			if (FEData)
			{	// don't forget to null out pointer to FEData in fHost
				if (FEData->fHost)
					FEData->fHost->SetFEData(NULL);
				delete FEData;
			}
			formElem->ele_text.FE_Data = NULL;
			break;
		case FORM_TYPE_TEXTAREA:
#ifdef ENDER
		case FORM_TYPE_HTMLAREA:
#endif /*ENDER*/
			if (formElem->ele_textarea.current_text)		// Free up the old memory
				PA_FREE(formElem->ele_textarea.current_text);
			formElem->ele_textarea.current_text = NULL;
			FEData = (FormFEData *)formElem->ele_textarea.FE_Data;
			if (FEData)
			{	// don't forget to null out pointer to FEData in fHost
				if (FEData->fHost)
					FEData->fHost->SetFEData(NULL);
				delete FEData;
			}
			formElem->ele_textarea.FE_Data = NULL;
			break;
		case FORM_TYPE_RADIO:
		case FORM_TYPE_CHECKBOX:
			FEData = (FormFEData *)formElem->ele_toggle.FE_Data;
			if (FEData)
			{	// don't forget to null out pointer to FEData in fHost
				if (FEData->fHost)
					FEData->fHost->SetFEData(NULL);
				delete FEData;
			}
			formElem->ele_toggle.FE_Data = NULL;
			break;
		case FORM_TYPE_SELECT_ONE:
		case FORM_TYPE_SELECT_MULT:
			FEData = (FormFEData *)formElem->ele_select.FE_Data;
			if ( FEData )
			{	// don't forget to null out pointer to FEData in fHost
				if (FEData->fHost)
					FEData->fHost->SetFEData(NULL);
				delete FEData;
			}
			formElem->ele_select.FE_Data = NULL;
			break;
		case FORM_TYPE_SUBMIT:
		case FORM_TYPE_RESET:
		case FORM_TYPE_BUTTON:
		case FORM_TYPE_READONLY:
			FEData = (FormFEData *)formElem->ele_minimal.FE_Data;
			if (FEData)
			{	// don't forget to null out pointer to FEData in fHost
				if (FEData->fHost)
					FEData->fHost->SetFEData(NULL);
				delete FEData;
			}
			formElem->ele_minimal.FE_Data = NULL;
			break;
		case FORM_TYPE_HIDDEN:
		case FORM_TYPE_IMAGE:
		case FORM_TYPE_JOT:
		case FORM_TYPE_KEYGEN:
		case FORM_TYPE_OBJECT:
			// no FE_Data for these guys
			break;
	};
}

/*-----------------------------------------------------------------------------
	Forms. 
	lo_ele.h
	
	Just like in other web clients, size is the size in characters the field should be, 
	max_size is the maximum length in characters that can be entered.
	default_text is the text the user wants to have appear in the textfield
	on creation and reset.
	
	The layout needs the front end to fill in the width and height fields in the
	LO_FormElementStruct.  It also needs the baseline field filled in.
	This may be difficult to impossible on some front ends.  baseline is
	the y position from 0 to (height - 1) that should line up with regular text on
	the same line as this form element. If necessary, just punt to
	baseline = (height - 1).
-----------------------------------------------------------------------------*/

// Took out XP_TRACEFORM call, so why use this function?

void UFormElementFactory::DisplayFormElement(
	CNSContext* inNSContext,
	LO_FormElementStruct* formElem )
{
	Rect	oldElementFrame, newElementFrame;
	Int32	extraX, extraY;
	LPane	*pane;

	pane = 0;

	Try_	// Because stupid LNTextEdit does a throw when you resize it too large
	{		
		// � the x,y passed to us originally may have been adjusted by layout
		//		so we have to update the widget. We don't actually need to draw it.
		//		It should only be adjusted once I think.
		if ( !XP_CheckElementSpan( *inNSContext,
			formElem->y + formElem->y_offset, formElem->height ) )
			return;
	
		if (!formElem->element_data)
			return; // jrm 97/03/21

		switch ( formElem->element_data->type )
		{
			case FORM_TYPE_SUBMIT:
			case FORM_TYPE_RESET:
			case FORM_TYPE_BUTTON:
			case FORM_TYPE_FILE:
				if (GetFEData (formElem)) {
					pane = GetFEDataPane(formElem);
					extraX = buttomLeftExtra;	// [sic]
					extraY = buttonTopExtra;
				}
				break;

			case FORM_TYPE_TEXT:
			case FORM_TYPE_PASSWORD:
			case FORM_TYPE_ISINDEX:
			case FORM_TYPE_READONLY:
			case FORM_TYPE_TEXTAREA:
#ifdef ENDER
			case FORM_TYPE_HTMLAREA:
#endif /*ENDER*/
				if (GetFEData (formElem)) {
					pane = GetFEDataPane(formElem);
					extraX = textLeftExtra;
					extraY = textTopExtra;
				}
			break;

			case FORM_TYPE_RADIO:
			case FORM_TYPE_CHECKBOX:
			case FORM_TYPE_SELECT_ONE:
			case FORM_TYPE_SELECT_MULT:
				if (GetFEData (formElem)) {
					pane = GetFEDataPane(formElem);
					extraX = 0;
					extraY = 0;
				}
				break;

			case FORM_TYPE_OBJECT:
				break;

			case FORM_TYPE_HIDDEN:
			case FORM_TYPE_KEYGEN:
			case FORM_TYPE_IMAGE:
			case FORM_TYPE_JOT:
			default:
				Assert_(FALSE);
				break;
		}

		if (pane) {
			if (pane->IsVisible() && formElem->ele_attrmask & LO_ELE_INVISIBLE) {
				pane->Hide();
			}
			pane->CalcPortFrameRect (oldElementFrame);
			pane->PlaceInSuperImageAt (formElem->x + formElem->x_offset + extraX,
									  formElem->y + formElem->y_offset + extraY, TRUE);
			pane->CalcPortFrameRect (newElementFrame);
			// if the element had to be moved, invalidate a slightly larger area than its current frame
			// (see notes below)
			if (oldElementFrame.top != newElementFrame.top || oldElementFrame.left != newElementFrame.left) {
				newElementFrame.left--;
				newElementFrame.top--;
				newElementFrame.right++;
				newElementFrame.bottom++;
				pane->InvalPortRect (&newElementFrame);
			}
			if (!pane->IsVisible() && !(formElem->ele_attrmask & LO_ELE_INVISIBLE)) {
				pane->Show();
				pane->Enable();
			}
			pane->Draw(nil);
		}
	}
	Catch_(inErr)
	{}
	EndCatch_

	/*	Why we invalidate a slightly larger rectangle than that occupied by the form element
		pane itself: when generating a page from HTML, the compositor creates a unique layer
		for each form element.  This layer is strictly rectangular and slightly larger than
		the form element pane, and is used as a mask to punch out a hole in its background.
		If the new form element happens to be located in a table cell using a background
		color different from that used by the document, the element will have a frame
		of the document background color around it after the document has been created.
		That frame is temporary; it is erased when the document is redrawn, because on
		subsequent draw operations, the element frame is no longer used to punch a hole
		in the cell background.  The cell background is rendered in the same pass as the
		document background, before PowerPlant panes (already created form elements) are
		drawn.  Invalidating a slightly larger framerect when the form element pane
		is first moved onto the document forces the important part of that redraw to happen
		immediately, erasing the inappropriately colored frame around the pane.
	*/
}

void UFormElementFactory::ResetFormElement(
	LO_FormElementStruct *formElem,
	Boolean redraw,
	Boolean fromDefaults)
{
	ResetFormElementData(formElem, redraw, fromDefaults, true);
}

void UFormElementFactory::SetFormElementToggle(
	LO_FormElementStruct* formElem,
	Boolean value )
{
#ifdef DEBUG
	if ( !formElem->element_data || formElem->element_data->type != FORM_TYPE_RADIO )
		XP_ABORT(("no form radio"));
#endif

	if ( !GetFEData( formElem ) )
		return;
	LStdControl* control = (LStdControl*)FEDATAPANE;
	if (!control)
		return;
	
//	control->SetValue( value );
//	control->Refresh();
	
	// MGY: In order to eliminate flickering, I changed the unenlightened call
	// to Refresh above to be a little more enlightened. TODO: find out why
	// it's here at all and remove it if no one can give a satifactory reason.
	// Before removing it, test both GA and non-GA toggles on a variety of backgfounds
	// to verify that it indeed did not perform any useful service.

#if 0
	Int32 oldValue = control->GetValue();
	if (value != oldValue)
	{
		control->SetValue(value);
		control->Refresh();
	}	
#endif
	
	// MGY: Do it this way instead
	
	control->SetValue(value);
}

void UFormElementFactory::FormTextIsSubmit(
	LO_FormElementStruct * formElem)
{
	if ( !GetFEData( formElem ) )
		return;
	if (!formElem->element_data)
		return;
	lo_FormElementTextData * textData = (lo_FormElementTextData *) formElem->element_data;
	CFormLittleText* editField = (CFormLittleText*)FEDATAPANE;
	if (editField)
		editField->SetBroadcast(TRUE);
}
//
// We should move this to some utility class
//
// To Do: Figure out is this ok for 
// 	TEXTAREA_WRAP_OFF, TEXTAREA_WRAP_SOFT and TEXTAREA_WRAP_HARD
//
static char* ConvertDataInTextEnginToUTF8(CWASTEEdit* engine)
{
	Handle		h = engine->GetTextHandle();
	Int32		offset, theTextSize = engine->GetTextLength();
	ScriptCode	script;
	INTL_Encoding_ID	encoding;
	WERunInfo	runInfo;
	Int32		gatherSize = theTextSize * 3 + 1;
	char*		utf8gather = (char*)XP_ALLOC(gatherSize);	
	
	if(utf8gather) {
		StHandleLocker lock(h);
		*utf8gather = '\0';	// null terminated the beginning

		for (offset = 0; offset < theTextSize; ) {
			engine->GetRunInfo(offset, &runInfo);
			script = FontToScript(runInfo.runStyle.tsFont);
			encoding = ScriptToEncoding(script);
			
			Int32 runLength = runInfo.runEnd - runInfo.runStart;
			char * utftemp = (char*)INTL_ConvertLineWithoutAutoDetect (encoding, CS_UTF8, 
				(unsigned char *)((*h) + offset), runLength);
			if(utftemp) {
				XP_STRCAT(utf8gather, utftemp);			
				XP_FREE(utftemp);
			}
			offset += runLength;
		}

		Int32 reallocatedLen = strlen(utf8gather) + 1;
		if(reallocatedLen < (gatherSize - 8))
		{	
			// Let's try to allocate a smalle buffer and return that instead if possible
			if(char* reallocated = (char*)XP_ALLOC(reallocatedLen))
			{
				XP_STRCPY(reallocated,utf8gather);
				XP_FREE(utf8gather);		
				return reallocated;
			}
		}
		return utf8gather; // otherwise, just return the one we allocated.
	}
	
	return "";
}


void UFormElementFactory::GetFormElementValue(
	LO_FormElementStruct *formElem,
	Boolean hide)
{
	if (!formElem->element_data)
		return;
	Try_
	{
		switch (formElem->element_data->type)
		{
			case FORM_TYPE_TEXT:
			case FORM_TYPE_PASSWORD:
			case FORM_TYPE_ISINDEX:		// Sets the original text
				{
				if ( !GetFEData( formElem ) )
					return;
				lo_FormElementTextData* textData =  (lo_FormElementTextData*)formElem->element_data;
				if (textData->current_text)
					PA_FREE(textData->current_text);
				textData->current_text = NULL;
				
				LEditField * editField = (LEditField*)FEDATAPANE;
				if (!editField)
					return;
				CStr255 text;
				editField->GetDescriptor(text);
				textData->current_text = PA_ALLOC(text.Length() +1);
				char * textPtr;
				PA_LOCK(textPtr, char*, textData->current_text);
				BlockMoveData(&(text[1]), textPtr, text.Length());
				textPtr[text.Length()] = 0;
				PA_UNLOCK(textData->current_text);
				}
				break;
				
			case FORM_TYPE_TEXTAREA:
				{
				if ( !GetFEData( formElem ) )
					return;
				lo_FormElementTextareaData * textAreaData = (lo_FormElementTextareaData *) formElem->element_data;
				if (textAreaData->current_text)		// Free up the old memory
					PA_FREE(textAreaData->current_text);
				textAreaData->current_text = NULL;
				// find the views
				LView * scroller = (LView*)FEDATAPANE;
				if (!scroller)
					return;
				CSimpleTextView*  textEdit = (CSimpleTextView*)scroller->FindPaneByID(formBigTextID);
				// Copy the text over
				Handle h = NULL;
				Boolean disposeHandle;		// Depending if we are making a copy of the text handle
				if(CS_UTF8 == formElem->text_attr->charset)
				{
					disposeHandle = FALSE;
					textAreaData->current_text = 
						(PA_Block) ConvertDataInTextEnginToUTF8(dynamic_cast<CWASTEEdit *>(textEdit));			
					
				} else {
					Int32 theTextSize;
					switch(textAreaData->auto_wrap)
						{
						case TEXTAREA_WRAP_OFF:
						case TEXTAREA_WRAP_SOFT:
							{
							disposeHandle = FALSE;
							h = textEdit->GetTextHandle();
							theTextSize = textEdit->GetTextLength();
							}
						break;
						case TEXTAREA_WRAP_HARD:
							{
							
							disposeHandle = TRUE;
							h = TextHandleToHardLineBreaks( *textEdit );
							theTextSize = ::GetHandleSize(h);
														
							}
						break;		
						}
						
					ThrowIfNULL_(h);

					// note that in the hard wrapped case we're adding an extra null byte
					// on the end.  this wont hurt.
					char* newTextPtr = (char*)XP_ALLOC(theTextSize + 1);
					::BlockMoveData(*h, newTextPtr, theTextSize);
					newTextPtr[theTextSize] = 0;

					textAreaData->current_text = (PA_Block)newTextPtr;
				}

				if (disposeHandle)
					DisposeHandle(h);
				}
				break;
				
#ifdef ENDER
			case FORM_TYPE_HTMLAREA:
				{
				if ( !GetFEData( formElem ) )
					return;
				lo_FormElementHTMLareaData * htmlAreaData = (lo_FormElementHTMLareaData *) formElem->element_data;
				if (htmlAreaData->current_text)		// Free up the old memory
					PA_FREE(htmlAreaData->current_text);
				htmlAreaData->current_text = NULL;
				// find the views
				LView * scroller = (LView*)FEDATAPANE;
				if (!scroller)
					return;
				CFormHTMLArea*  htmlEdit = (CFormHTMLArea*)scroller->FindPaneByID(formHTMLAreaID);
				Assert_(htmlEdit != NULL);
				// Copy the text over
				char* newTextPtr = 0;
				EDT_SaveToBuffer(*(((CHTMLView*)htmlEdit)->GetContext()), (XP_HUGE_CHAR_PTR*) &newTextPtr);
				htmlAreaData->current_text = (PA_Block)newTextPtr;
				}
				break;
#endif /*ENDER*/

			case FORM_TYPE_RADIO:
			case FORM_TYPE_CHECKBOX:	// Sets the original check state
				if ( !GetFEData( formElem ) )
					return;
				lo_FormElementToggleData * toggleData = (lo_FormElementToggleData*)formElem->element_data;
				LStdControl * toggle = (LStdControl *)FEDATAPANE;
				if (toggle)
					toggleData->toggled = toggle->GetValue();
				break;
			case FORM_TYPE_FILE:
				lo_FormElementTextData * pickData = (lo_FormElementTextData*)formElem->element_data;
				CFormFile * formFile = (CFormFile *)FEDATAPANE;
				FSSpec spec;
				if (formFile && formFile->GetFileSpec(spec))
				{
					char * fileURL = CFileMgr::EncodedPathNameFromFSSpec( spec, TRUE );
					if (fileURL)
					{
						PA_Block newValueBlock = PA_ALLOC(XP_STRLEN(fileURL) + 1);
						char * value;
						PA_LOCK(value, char*, newValueBlock);
						::BlockMoveData(fileURL, value, XP_STRLEN(fileURL) + 1);
						PA_UNLOCK(newValueBlock);
						pickData->current_text = newValueBlock;
						XP_FREE(fileURL);
					}
					else
						pickData->current_text = NULL;
				}
				else
					pickData->current_text = NULL;
				break;
			case FORM_TYPE_HIDDEN:
			case FORM_TYPE_KEYGEN:
			case FORM_TYPE_SUBMIT:
			case FORM_TYPE_RESET:
			case FORM_TYPE_BUTTON:
			case FORM_TYPE_READONLY:
				break;
			case FORM_TYPE_SELECT_ONE:
			{	// Resets the selects
				if ( !GetFEData( formElem ) )
					return;
				lo_FormElementSelectData_struct* selections = &formElem->element_data->ele_select;
				LButton* widget = (LButton*)FEDATAPANE;
				int value = widget ? widget->GetValue() : 0;
				lo_FormElementOptionData_struct* options = (lo_FormElementOptionData_struct*)selections->options;
				for ( int i = 0; i < selections->option_cnt; i++ )
					options[ i ].selected = FALSE;
				if ( value )
					options[ value - 1].selected = TRUE;
			}
			break;
			case FORM_TYPE_SELECT_MULT:
			{
				lo_FormElementSelectData* selectData = (lo_FormElementSelectData *)formElem->element_data;
				CFormList* myList = (CFormList*)FEDATAPANE;
				if (!myList)
					return;
				lo_FormElementOptionData* mother = (lo_FormElementOptionData*)selectData->options;
				for ( int i = 0 ; i < selectData->option_cnt; i++ )
					mother[ i ].selected = myList->IsSelected( i );
			}
			break;
			case FORM_TYPE_IMAGE:
			case FORM_TYPE_JOT:
			case FORM_TYPE_OBJECT:
				break;
		}
	}
	Catch_(inErr){}
	EndCatch_
	if (hide)
		HideFormElement(formElem);
}

void UFormElementFactory::HideFormElement(
	LO_FormElementStruct* formElem)
{
	if (!formElem->element_data)
		return;
	switch ( formElem->element_data->type )
	{
		case FORM_TYPE_TEXT:
		case FORM_TYPE_PASSWORD:
		case FORM_TYPE_ISINDEX:
		case FORM_TYPE_TEXTAREA:
#ifdef ENDER
		case FORM_TYPE_HTMLAREA:
#endif /*ENDER*/
		case FORM_TYPE_RADIO:
		case FORM_TYPE_CHECKBOX:
		case FORM_TYPE_SUBMIT:
		case FORM_TYPE_RESET:
		case FORM_TYPE_BUTTON:
		case FORM_TYPE_SELECT_ONE:
		case FORM_TYPE_SELECT_MULT:
		case FORM_TYPE_FILE:
		case FORM_TYPE_READONLY:

			LFormElement*	host = FEDATAHOST;
			
			// Schedule the form element to be destroyed.
			// We don't do it immediately because we may
			// be in the middle of a JavaScript which could
			// then return to some PowerPlant code which
			// still needs access to the form element widget
			//
			// Read the comments for MarkForDeath for more details
			//
			if (host != NULL)  {
				host->MarkForDeath();
			}
				
			FEDATAPANE = NULL;
			FEDATAHOST = NULL;			
			break;
			
			
		case FORM_TYPE_HIDDEN:
		case FORM_TYPE_KEYGEN:
			// no value to set or reset
		break;
		case FORM_TYPE_IMAGE:
		case FORM_TYPE_JOT:
		case FORM_TYPE_OBJECT:
		break;
	}
}
static void ValidateTextByCharset(int csid,	char* text)
{
	if(text && (csid & MULTIBYTE))
	{
		int pos;
		int lastIdx = 0;
		int boundary = strlen(text);
		for(pos = 0 ; pos < boundary ; pos += (INTL_IsLeadByte(csid, text[pos])+1))
			lastIdx = pos;
		if(pos != boundary)
			text[lastIdx] = '\0';
	}
}


static void PutUnicodeIntoTextEngine( CWASTEEdit* engine, INTL_Unicode* unicode, Int32 len)
{
	INTL_CompoundStr* cs = INTL_CompoundStrFromUnicode(unicode, len);
	UFontSwitcher* fs= UFixedFontSwitcher::Instance();
	if(cs)
	{
		// Clear the text before insertining text runs.
		engine->SelectAll();
		engine->Delete();

		INTL_Encoding_ID encoding;
		unsigned char* outtext;
		INTL_CompoundStrIterator iter;
		for(iter = INTL_CompoundStrFirstStr((INTL_CompoundStrIterator)cs, &encoding , &outtext);
				iter != NULL;
					iter = INTL_CompoundStrNextStr(iter, &encoding, &outtext))
		{
			if((outtext) && (*outtext))
			{
				Int32	theTextSize = engine->GetTextLength();
				// Set the Text
				engine->SetSelection(LONG_MAX, LONG_MAX);
				engine->InsertPtr((char*)outtext, XP_STRLEN((char*)outtext), NULL, NULL);
				
				CCharSet charset;	// Don't move this line since the fontname is point to it.
				unsigned char* fontname;
				switch(encoding)
				{
					case CS_SYMBOL:
						fontname = (unsigned char*)"\pSymbol";
						break;
					case CS_DINGBATS:
						fontname = (unsigned char*)"\pZapf Dingbats";
						break;
					default:
						Boolean gotFont = CPrefs::GetFont(encoding, &charset);					
						Assert_(gotFont);
						fontname = &charset.fPropFont[0];//fFixedFont[0];
						break;					
				}
				
				TextStyle	inTextStyle;
				GetFNum(fontname, &inTextStyle.tsFont);
				engine->SetSelection(theTextSize, LONG_MAX);
				engine->SetStyle(weDoFont, &inTextStyle);
			}
		}
		INTL_CompoundStrDestroy(cs);
	}
}


static void PutUTF8IntoTextEngine( CWASTEEdit* engine, char* text, Int32 len)
{
	if(len >0)
	{
		uint32 ubuflen = len+1;
		uint32 ucs2len;
		INTL_Unicode* ucs2buf = NULL;
		ucs2buf = new INTL_Unicode[ubuflen];
		if(ucs2buf && (0 != (ucs2len = UUTF8TextHandler::Instance()->UTF8ToUCS2((unsigned char*) text, len, ucs2buf,  ubuflen) )))
			PutUnicodeIntoTextEngine(engine, ucs2buf, ucs2len);	
		else
			Assert_(FALSE);
			
		if(ucs2buf)
			delete ucs2buf;
	}
}


static void ChangeTextTraitsForSingleScript( CWASTEEdit* engine, unsigned short csid)
{
	TextTraitsH	theTextTraits = UTextTraits::LoadTextTraits(CPrefs::GetTextFieldTextResIDs(csid));
	TextStyle	ts;

	if (theTextTraits != NULL) {
		ts.tsFont = (*theTextTraits)->fontNumber;
		ts.tsSize = (*theTextTraits)->size;
		ts.tsFace = (*theTextTraits)->style;
		ts.tsColor = (*theTextTraits)->color;

		engine->SetStyle(weDoAll, &ts);
	}
}


static void SetupTextInTextEngine( CWASTEEdit* engine, int16 csid, char* text, int32 len)
{
	if(CS_UTF8 == csid)
	{
		PutUTF8IntoTextEngine( engine, text, len );
	} else {
		ChangeTextTraitsForSingleScript(engine, csid );
		if (text != NULL)
			engine->InsertPtr( text, len, NULL, NULL, true );
		else
		{
			engine->SelectAll();
			engine->Delete();
		}
	}
}


void UFormElementFactory::ResetFormElementData(
	LO_FormElementStruct *formElem,
	Boolean redraw,
	Boolean fromDefaults,
	Boolean reflect)
{	
	if (!formElem->element_data)
		return;
	switch (formElem->element_data->type)
	{
		case FORM_TYPE_TEXT:				// Set the text
		case FORM_TYPE_PASSWORD:
		case FORM_TYPE_ISINDEX:
		{
			if ( !GetFEData( formElem ) )
				return;
			lo_FormElementTextData* textData =  & formElem->element_data->ele_text;
			LEditField * editField = (LEditField *)FEDATAPANE;
			if (!editField)
				return;
			char* newText = NULL;
			if ( fromDefaults )
			{
				if ( textData->default_text )		// Eric might be passing us NULL
				{
					newText = (char*)textData->default_text;
					editField->SetDescriptor( CStr255( newText ) );
				}
				else
					editField->SetDescriptor( "\p" );
			}
			else
			{
				newText = (char*)textData->current_text;
				editField->SetDescriptor( CStr255( newText ) );
			}
			if ( redraw )
			{
				editField->Refresh();
				editField->UpdatePort();
			}
		}
		break;
	
		case FORM_TYPE_TEXTAREA:
		{		
			if ( !GetFEData( formElem ) )
				return;
			lo_FormElementTextareaData* textAreaData = (lo_FormElementTextareaData*) formElem->element_data;
			LView* scroller = (LView*)FEDATAPANE;
			if (!scroller)
				return;
			CSimpleTextView* textEdit = (CSimpleTextView*)scroller->FindPaneByID(formBigTextID);
			CWASTEEdit* engine = dynamic_cast<CWASTEEdit *>(textEdit);
			char* default_text;
			if ( fromDefaults )
			{
				if ( textAreaData->default_text )
				{
					default_text = (char*)textAreaData->default_text;
					ValidateTextByCharset(formElem->text_attr->charset, default_text);
					Try_
					{
						SetupTextInTextEngine(engine, formElem->text_attr->charset, default_text, XP_STRLEN((char*)default_text));
					}
					Catch_( inErr )
					{
					}
					EndCatch_
				}
				else			// No default value
				{
					Try_
					{
						SetupTextInTextEngine(engine, formElem->text_attr->charset, NULL, 0);
					}
					Catch_( inErr )
					{
					}
				}
			}
			else
			{
				default_text = (char*)textAreaData->current_text;
				ValidateTextByCharset(formElem->text_attr->charset, default_text);
				Try_
				{
				
					// don't want ending linefeed
					Int32 length = XP_STRLEN((char*)default_text)-1;
					if( length )
						SetupTextInTextEngine(engine, formElem->text_attr->charset, default_text,  length);
				}
				Catch_( inErr ) 
				{
				}
			}
			if ( redraw )
				textEdit->Refresh();
		}
		break;

#ifdef ENDER
		case FORM_TYPE_HTMLAREA:
		{		
			if ( !GetFEData( formElem ) )
				return;
			lo_FormElementHTMLareaData* htmlAreaData = (lo_FormElementHTMLareaData*) formElem->element_data;
			LView* scroller = (LView*)FEDATAPANE;
			if (!scroller)
				return;
			CFormHTMLArea* htmlEdit = (CFormHTMLArea*)scroller->FindPaneByID(formHTMLAreaID);
			if (!htmlEdit)
				return;
			MWContext *pContext = *(((CHTMLView*)htmlEdit)->GetContext());
			if ( fromDefaults )
			{
				EDT_SetDefaultText( pContext,(char*)(htmlAreaData->default_text) );
			}
			else
			{
				// moose: this is probably all wrong... revisit.
				EDT_ReadFromBuffer( pContext, (XP_HUGE_CHAR_PTR)htmlAreaData->current_text );
			}
			if ( redraw )
				htmlEdit->Refresh();
		}
		break;
#endif /*ENDER*/

		case FORM_TYPE_RADIO:		// Set the toggle to on or off
		case FORM_TYPE_CHECKBOX:
		{
			if ( !GetFEData( formElem ) )
				return;
			lo_FormElementToggleData* toggleData = (lo_FormElementToggleData*)formElem->element_data;
			LStdControl* toggle = (LStdControl*)FEDATAPANE;
			if (!toggle)
				return;
			if ( fromDefaults )
			{			
				if ( toggle->GetValue() == toggleData->default_toggle )	// Do not change unless we have to
					break;
				toggle->SetValue( toggleData->default_toggle );
			}
			else // use the stored value
			{			
				if ( toggle->GetValue() == toggleData->toggled )	
					break;
				toggle->SetValue( toggleData->toggled );
			}
			if ( redraw )
				toggle->Refresh();
		}
		break;

		case FORM_TYPE_FILE:
		{
			lo_FormElementTextData * pickData = (lo_FormElementTextData*)formElem->element_data;
			CFormFile * formFile = (CFormFile *)FEDATAPANE;
			if (!formFile)
				return;
			formFile->SetVisibleChars(pickData->size);
			if (fromDefaults)
				;	// Do nothing
			else
			{
				char * default_text;
				FSSpec spec;
				OSErr err;
				PA_LOCK(default_text, char*, pickData->current_text);
				if (default_text)
					err = CFileMgr::FSSpecFromLocalUnixPath(default_text, &spec);
				else
					err = fnfErr;
				PA_UNLOCK(pickData->current_text);				
				if (err == noErr)
					formFile->SetFileSpec(spec);
			}
			if (redraw)
				formFile->Refresh();
		}
		break;
		
		case FORM_TYPE_HIDDEN:
		case FORM_TYPE_KEYGEN:
		case FORM_TYPE_SUBMIT:
		case FORM_TYPE_RESET:
		case FORM_TYPE_BUTTON:
		// no value to set or reset
		break;
		
		case FORM_TYPE_SELECT_ONE:
		{
			if ( !GetFEData( formElem ) )
				return;
			lo_FormElementSelectData* selectData = (lo_FormElementSelectData*)formElem->element_data;
			int offset = CalcCurrentSelected( selectData, fromDefaults );
			
			LButton*		widget = (LButton*)FEDATAPANE;
			FormsPopup*		popup  = (FormsPopup*)FEDATAHOST;
			
			if (popup)
				popup->DirtyMenu();
			if (!widget)
				return;
			widget->SetValue( offset + 1 );
			if ( redraw )
				widget->Refresh();
		}
		break;

		case FORM_TYPE_SELECT_MULT:		// Select/unselect the cells
		{
			if ( !GetFEData( formElem ) )
				return;
			lo_FormElementOptionData*	mother;
			lo_FormElementSelectData*	selectData = (lo_FormElementSelectData *)formElem->element_data;
			CFormList*					myList 	 = (CFormList*)FEDATAPANE;
			
			if (!myList)
				return;
			myList->SelectNone();		
			myList->FocusDraw();
				
			::LSetDrawingMode( FALSE, myList->GetMacListH() );
		
			mother = (lo_FormElementOptionData*)selectData->options;
			myList->SyncRows( selectData->option_cnt );
			for ( int i = 0; i < selectData->option_cnt; i++ )
			{
				char* itemName = NULL;
				Boolean select;
				
				itemName = (char*)mother[ i ].text_value;
				myList->AddItem( itemName, i );
				 
				if ( fromDefaults )
					select = mother[ i ].def_selected;
				else
					select = mother[ i ].selected;
					
				if (select)
					myList->SetSelect( i, select );
			}

			::LSetDrawingMode( TRUE, myList->GetMacListH() );
			// ��this forces an update of the scrollbar, if there is
			//		one
			::LUpdate( NULL, myList->GetMacListH() );
			myList->ShrinkToFit( selectData->size );
			Cell selection = { myList->GetValue(), 0 };		// cells are v, h
			myList->MakeCellVisible ( selection );
			if ( redraw )
				myList->Refresh();
		}
		break;

		case FORM_TYPE_IMAGE:
		case FORM_TYPE_JOT:
		case FORM_TYPE_OBJECT:
		break;
	}

	
// immediately reflect our state
	if (reflect)
		FEDATAHOST->ReflectData();
}
