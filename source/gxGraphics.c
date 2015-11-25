/****************************************************//*													*//*	File:		gxGraphics.c						*//*													*//*	Program:	Imageer								*//*													*//*	By:			Jason Hodges-Harris					*//*													*//*	Created:	26/10/95  00:00:00 AM				*//*													*//*	Version:	1.0.0d3								*//*													*//*	Copyright:	� 1995-96 Apple Computer, Inc.,		*/ /*					all rights reserved.			*/		/*													*//****************************************************//**** Macintosh Toolbox Headers *****/#ifndef __GXENVIRONMENT__#include <GXEnvironment.h>#endif#ifndef __GXGRAPHICS__#include <GXGraphics.h>#endif#ifndef __GXTYPES__#include <GXTypes.h>#endif#ifndef __QDOFFSCREEN__#include <QDOffscreen.h>#endif#ifndef __TYPES__#include <Types.h>#endif/****   Application headers and prototypes   ****/#ifndef __IMAGEERAPPHEADER__#include "Imageer.app.h"#endif#ifndef __IMAGEERPROTOSHEADER__#include "Imageer.protos.h"#endif/****	Function to convert an existing Color QuickDraw image to a QuickDraw GX object	****/#pragma segment gxGrafixOSErr ConvPixMapToGXShape(ImageDocHndl theDocHndl, WindowPtr theWindow){	ImageDocPtr 		theDocPtr;	PixMapHandle		thePixMapHndl = nil;	gxShapeSpoolUPP		theGXSpoolUPP;	GDHandle			theGDevHndl = nil;	GWorldPtr			oldPort;	Rect				theImagePortRect = (*theDocHndl)->theImageWorld->portRect;	OSErr				error = noErr;	Point				scale = {1,1};		GetGWorld(&oldPort,&theGDevHndl);	thePixMapHndl = GetGWorldPixMap((*theDocHndl)->theImageWorld);	if (PixMap32Bit(thePixMapHndl))		SaveSetMMUMode(true);	if (!LockPixels(thePixMapHndl))		error = kFailedLockPixels;	if (!error)	{		HLock((Handle)theDocHndl);		theDocPtr = *theDocHndl;		// Convert QuickDraw GWorld to GX based shape object		theGXSpoolUPP = NewgxShapeSpoolProc(QDShapeSpooler);		SetGWorld(theDocPtr->theImageWorld,nil);		GXInstallQDTranslator((GrafPtr)theDocPtr->theImageWorld, gxDefaultOptionsTranslation,							  &theImagePortRect,							  &theImagePortRect,							  scale, theGXSpoolUPP, (void*)theDocPtr);		CopyBits((BitMap*)(*thePixMapHndl),				 (BitMap*)(*thePixMapHndl),				 &theImagePortRect,				 &theImagePortRect,srcCopy,nil);		GXRemoveQDTranslator((GrafPtr)theDocPtr->theImageWorld,nil);		DisposeRoutineDescriptor((UniversalProcPtr)theGXSpoolUPP);	}	UnlockPixels(thePixMapHndl);			// unlock GWorld	SaveSetMMUMode(false);	SetGWorld((CGrafPtr)theWindow,theGDevHndl);	InitTransferMatrix (theDocHndl,gxCopyMode,gxCopyMode,gxCopyMode);		// init transfer matrix for shape	GXSetShapeInk(theDocPtr->theGXImageShape, theDocPtr->theInkShape);	HUnlock((Handle)theDocHndl);	InvalRect(&theWindow->portRect);		// invalidate window port rect	SetGWorld(oldPort,theGDevHndl);	return error;}/****	spooler function for Color QuickDraw to QuickDRaw GX translator *****/#pragma segment gxGrafixOSErr QDShapeSpooler (gxShape theShape, void *theDocHndlRef){	ImageDocPtr theDocPtr = (ImageDocPtr)theDocHndlRef;		theDocPtr->theGXImageShape = GXCopyToShape(nil, theShape);	GXSetShapeViewPorts(theDocPtr->theGXImageShape,1,&(theDocPtr->theGxChildView));	return	(GXGetGraphicsError(nil));}/****	Create GX View Ports for a given window	****/#pragma segment gxGrafixOSErr	CreateGXviewPorts(ImageDocHndl theDocHndl, WindowPtr theWindow){	gxRectangle		theViewRect;	gxShape			theContentShape;	Rect			thePortRect;	OSErr			error = noErr;		// Create GX parent viewport	thePortRect = (*theDocHndl)->theImageWorld->portRect;	CreateGXRect(&theViewRect, thePortRect.top, thePortRect.left,				thePortRect.bottom, thePortRect.right);	theContentShape = GXNewRectangle(&theViewRect);	(**theDocHndl).theGXview = GXNewWindowViewPort(theWindow);	GXSetViewPortClip((**theDocHndl).theGXview, theContentShape);	GXDisposeShape (theContentShape);	// Create GX child viewport	thePortRect = theWindow->portRect;	CreateGXRect(&theViewRect, thePortRect.top, thePortRect.left,				thePortRect.bottom- kScrollBarWidth, thePortRect.right- kScrollBarWidth);	(**theDocHndl).theGxChildView = 		GXNewViewPort(GXGetViewPortViewGroup((**theDocHndl).theGXview));	theContentShape = GXNewRectangle(&theViewRect);	GXSetViewPortClip((**theDocHndl).theGxChildView, theContentShape);	GXSetViewPortMapping((**theDocHndl).theGxChildView, nil);	GXSetViewPortParent((**theDocHndl).theGxChildView, (**theDocHndl).theGXview);	GXDisposeShape (theContentShape);	return error;}/****	Update QDGX Object displayed in child viewport	****/#pragma segment gxGrafixvoid	UpdateGXObjectDisplay(ImageDocHndl	theWindDocHndl, WindowPtr theWindow){	gxShape			theContentShape;	gxRectangle		theViewRect;	Rect			thePortRect;	short			theXvalue,					theYvalue;	theXvalue = GetControlValue((**theWindDocHndl).theHScrollBar);	theYvalue = GetControlValue((**theWindDocHndl).theVScrollBar);	GXMoveShapeTo((**theWindDocHndl).theGXImageShape,ff(-theXvalue), ff(-theYvalue));	thePortRect = theWindow->portRect;	CreateGXRect(&theViewRect, thePortRect.top, thePortRect.left,				thePortRect.bottom- kScrollBarWidth, thePortRect.right- kScrollBarWidth);	theContentShape = GXNewRectangle(&theViewRect);	GXSetViewPortClip((**theWindDocHndl).theGxChildView, theContentShape);	GXDisposeShape (theContentShape);	GXDrawShape((**theWindDocHndl).theGXImageShape);}/****	Initialise transfer matrix	****/#pragma segment gxGrafixvoid	InitTransferMatrix (ImageDocHndl theDocHndl,							gxComponentMode comp1TransferMode,							gxComponentMode comp2TransferMode,							gxComponentMode comp3TransferMode){	gxTransferMode	theTransferMode;	short counter;	theTransferMode.space = gxRGBSpace;	theTransferMode.set = nil;	theTransferMode.profile = nil;	// initialise source matrix tranfer mode	theTransferMode.sourceMatrix[0][0] = ff(1.0);	for (counter = 1;counter<4;counter++)		theTransferMode.sourceMatrix[0][counter] = 0;	theTransferMode.sourceMatrix[1][0] = 0;	theTransferMode.sourceMatrix[1][1] = ff(1.0);	for (counter = 2;counter<4;counter++)		theTransferMode.sourceMatrix[1][counter] = 0;	for (counter = 0;counter<2;counter++)		theTransferMode.sourceMatrix[2][counter] = 0;	theTransferMode.sourceMatrix[2][2] = ff(1.0);	theTransferMode.sourceMatrix[2][3] = 0;	for (counter = 0;counter<3;counter++)		theTransferMode.sourceMatrix[3][counter] = 0;	theTransferMode.sourceMatrix[3][3] = ff(1.0);	for (counter = 0;counter<4;counter++)		theTransferMode.sourceMatrix[4][counter] = 0;	// initialise device matrix tranfer mode	theTransferMode.deviceMatrix[0][0] = ff(1.0);	for (counter = 1;counter<4;counter++)		theTransferMode.deviceMatrix[0][counter] = 0;	theTransferMode.deviceMatrix[1][0] = 0;	theTransferMode.deviceMatrix[1][1] = ff(1.0);	for (counter = 2;counter<4;counter++)		theTransferMode.deviceMatrix[1][counter] = 0;	for (counter = 0;counter<2;counter++)		theTransferMode.deviceMatrix[2][counter] = 0;	theTransferMode.deviceMatrix[2][2] = ff(1.0);	theTransferMode.deviceMatrix[2][3] = 0;	for (counter = 0;counter<3;counter++)		theTransferMode.deviceMatrix[3][counter] = 0;	theTransferMode.deviceMatrix[3][3] = ff(1.0);	for (counter = 0;counter<4;counter++)		theTransferMode.deviceMatrix[4][counter] = 0;	// initialise result matrix tranfer mode	theTransferMode.resultMatrix[0][0] = ff(1.0);	for (counter = 1;counter<4;counter++)		theTransferMode.resultMatrix[0][counter] = 0;	theTransferMode.resultMatrix[1][0] = 0;	theTransferMode.resultMatrix[1][1] = ff(1.0);	for (counter = 2;counter<4;counter++)		theTransferMode.resultMatrix[1][counter] = 0;	for (counter = 0;counter<2;counter++)		theTransferMode.resultMatrix[2][counter] = 0;	theTransferMode.resultMatrix[2][2] = ff(1.0);	theTransferMode.resultMatrix[2][3] = 0;	for (counter = 0;counter<3;counter++)		theTransferMode.resultMatrix[3][counter] = 0;	theTransferMode.resultMatrix[3][3] = ff(1.0);	for (counter = 0;counter<4;counter++)		theTransferMode.resultMatrix[4][counter] = 0;	theTransferMode.flags = 0;	// Red Component	theTransferMode.component[0].mode				= comp1TransferMode;	theTransferMode.component[0].flags				= 0;	theTransferMode.component[0].sourceMinimum		= 0;	theTransferMode.component[0].deviceMinimum		= 0;	theTransferMode.component[0].clampMinimum		= 0;	theTransferMode.component[0].clampMaximum		= gxColorValue1;	theTransferMode.component[0].sourceMaximum		= gxColorValue1;	theTransferMode.component[0].deviceMaximum		= gxColorValue1;	// Green Component	theTransferMode.component[1].mode				= comp2TransferMode;	theTransferMode.component[1].flags				= 0;	theTransferMode.component[1].sourceMinimum		= 0;	theTransferMode.component[1].deviceMinimum		= 0;	theTransferMode.component[1].clampMinimum		= 0;	theTransferMode.component[1].clampMaximum		= gxColorValue1;	theTransferMode.component[1].sourceMaximum		= gxColorValue1;	theTransferMode.component[1].deviceMaximum		= gxColorValue1;	// Blue Component	theTransferMode.component[2].mode				= comp3TransferMode;	theTransferMode.component[2].flags				= 0;	theTransferMode.component[2].sourceMinimum		= 0;	theTransferMode.component[2].deviceMinimum		= 0;	theTransferMode.component[2].clampMinimum		= 0;	theTransferMode.component[2].clampMaximum		= gxColorValue1;	theTransferMode.component[2].sourceMaximum		= gxColorValue1;	theTransferMode.component[2].deviceMaximum		= gxColorValue1;	(**theDocHndl).theInkShape						= GXNewInk();	GXSetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);}/****	gxShape rotation function	****/#pragma segment gxGrafixOSErr RotateGxShape(ImageDocHndl theDocHndl, WindowPtr theWindow, Fixed angle){	GDHandle			theGDevHndl = nil;	GWorldPtr			oldPort;	gxTransform			theShapeTransform,						theTempTransform;	gxMapping			theTransMapping;	gxPoint				theImageCentre;	OSErr				error = noErr;		GetGWorld(&oldPort,&theGDevHndl);	theShapeTransform = GXGetShapeTransform((**theDocHndl).theGXImageShape);	theTempTransform = GXNewTransform();	GXGetShapeCenter((**theDocHndl).theGXImageShape,0,&theImageCentre);	GXRotateTransform(theTempTransform,angle,theImageCentre.x,theImageCentre.y);	GXGetTransformMapping(theTempTransform,&theTransMapping);	GXMapTransform(theShapeTransform,&theTransMapping);	GXDisposeTransform(theTempTransform);	GXSetShapeTransform((**theDocHndl).theGXImageShape, theShapeTransform);	GXSetShapeAttributes((**theDocHndl).theGXImageShape, gxNoAttributes);	SetGWorld((CGrafPtr)theWindow,theGDevHndl);	InvalRect(&theWindow->portRect);		// invalidate window port rect	SetGWorld(oldPort,theGDevHndl);	return error;}/****	gxShape Mirror image transform	****/#pragma segment gxGrafixOSErr MirrorGxShape(ImageDocHndl theDocHndl, WindowPtr theWindow, Boolean isXaxis){	GDHandle			theGDevHndl = nil;	GWorldPtr			oldPort;	gxTransform			theShapeTransform;	gxMapping			theTransMapping;	gxRectangle			theShapeBounds;	gxPoint				theImageCentre;	OSErr				error = noErr;		GetGWorld(&oldPort,&theGDevHndl);	theShapeTransform = GXGetShapeTransform((**theDocHndl).theGXImageShape);	GXGetShapeCenter((**theDocHndl).theGXImageShape,0,&theImageCentre);	/****	Identity transform matrix elements	****/	theTransMapping.map[1][0] = 0;	theTransMapping.map[2][0] = 0;	theTransMapping.map[0][1] = 0;	theTransMapping.map[2][1] = 0;	theTransMapping.map[0][2] = 0;	theTransMapping.map[1][2] = 0;	theTransMapping.map[2][2] = fract1;	if (isXaxis)		// mirror shape along X axis	{		theTransMapping.map[0][0] = ff(1);		theTransMapping.map[1][1] = ff(-1);	}	else				// mirror shape along Y axis	{		theTransMapping.map[0][0] = ff(-1);		theTransMapping.map[1][1] = ff(1);	}	GXMapTransform(theShapeTransform,&theTransMapping);	GXSetShapeTransform((**theDocHndl).theGXImageShape, theShapeTransform);	GXSetShapeAttributes((**theDocHndl).theGXImageShape, gxMapTransformShape);	GXGetShapeBounds((**theDocHndl).theGXImageShape, 0, &theShapeBounds);	if (isXaxis)		GXMoveShape((**theDocHndl).theGXImageShape,0, theShapeBounds.bottom - theShapeBounds.top);	else		GXMoveShape((**theDocHndl).theGXImageShape,theShapeBounds.right - theShapeBounds.left, 0);		GXSetShapeAttributes((**theDocHndl).theGXImageShape, gxNoAttributes);	SetGWorld((CGrafPtr)theWindow,theGDevHndl);	InvalRect(&theWindow->portRect);		// invalidate window port rect	SetGWorld(oldPort,theGDevHndl);	return error;}/****	QDGX Ink object inverter	****/#pragma segment gxGrafixOSErr	GxShapeInkInvert (ImageDocHndl theDocHndl, WindowPtr theWindow){	GDHandle			theGDevHndl = nil;	GWorldPtr			oldPort;	gxTransferMode		theTransferMode;	OSErr				error = noErr;	short				counter;		GetGWorld(&oldPort,&theGDevHndl);	GXGetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);	if (&theTransferMode == nil)	{		DisplayAlert(rGenAlert,rQDGXmessages,iInkGetFail);		error = kGXFailGetInk;	}	else	{		if (theTransferMode.sourceMatrix[4][0] == ff(1.0))		{			//	Reset to color identity matrix			for (counter = 0;counter<4;counter++)			{				theTransferMode.sourceMatrix[4][counter] = ff(0.0);				theTransferMode.sourceMatrix[counter][counter] = ff(1.0);			}		}		else		{			//	Set to color inversion matrix			for (counter = 0;counter<4;counter++)			{				theTransferMode.sourceMatrix[4][counter] = ff(1.0);				theTransferMode.sourceMatrix[counter][counter] = ff(-1.0);			}		}		GXSetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);		GXSetShapeInk((**theDocHndl).theGXImageShape, (**theDocHndl).theInkShape);		SetGWorld((CGrafPtr)theWindow,theGDevHndl);		InvalRect(&theWindow->portRect);		// invalidate window port rect		SetGWorld(oldPort,theGDevHndl);		}	return error;}/****	Switch ink object source color component source transfer value	Removes color components ****/#pragma segment gxGrafixOSErr	RGBColorComponent (ImageDocHndl theDocHndl, WindowPtr theWindow, short colorComponent){	GDHandle			theGDevHndl = nil;	GWorldPtr			oldPort;	gxTransferMode		theTransferMode;	OSErr				error = noErr;	short				counter;		GetGWorld(&oldPort,&theGDevHndl);	GXGetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);	if (&theTransferMode == nil)	{		DisplayAlert(rGenAlert,rQDGXmessages,iInkGetFail);		error = kGXFailGetInk;	}	else	{		theTransferMode.space = gxRGBSpace;		theTransferMode.set = nil;		theTransferMode.profile = nil;				// initialise source matrix tranfer mode		theTransferMode.sourceMatrix[0][0] = ff(1.0);		for (counter = 1;counter<4;counter++)			theTransferMode.sourceMatrix[0][counter] = 0;		theTransferMode.sourceMatrix[1][0] = 0;		theTransferMode.sourceMatrix[1][1] = ff(1.0);		for (counter = 2;counter<4;counter++)			theTransferMode.sourceMatrix[1][counter] = 0;		for (counter = 0;counter<2;counter++)			theTransferMode.sourceMatrix[2][counter] = 0;		theTransferMode.sourceMatrix[2][2] = ff(1.0);		theTransferMode.sourceMatrix[2][3] = 0;		for (counter = 0;counter<3;counter++)			theTransferMode.sourceMatrix[3][counter] = 0;		theTransferMode.sourceMatrix[3][3] = ff(1.0);		for (counter = 0;counter<4;counter++)			theTransferMode.sourceMatrix[4][counter] = 0;		// initialise device matrix tranfer mode		theTransferMode.deviceMatrix[0][0] = ff(1.0);		for (counter = 1;counter<4;counter++)			theTransferMode.deviceMatrix[0][counter] = 0;		theTransferMode.deviceMatrix[1][0] = 0;		theTransferMode.deviceMatrix[1][1] = ff(1.0);		for (counter = 2;counter<4;counter++)			theTransferMode.deviceMatrix[1][counter] = 0;		for (counter = 0;counter<2;counter++)			theTransferMode.deviceMatrix[2][counter] = 0;		theTransferMode.deviceMatrix[2][2] = ff(1.0);		theTransferMode.deviceMatrix[2][3] = 0;		for (counter = 0;counter<3;counter++)			theTransferMode.deviceMatrix[3][counter] = 0;		theTransferMode.deviceMatrix[3][3] = ff(1.0);		for (counter = 0;counter<4;counter++)			theTransferMode.deviceMatrix[4][counter] = 0;			// initialise result matrix tranfer mode		theTransferMode.resultMatrix[0][0] = ff(1.0);		for (counter = 1;counter<4;counter++)			theTransferMode.resultMatrix[0][counter] = 0;		theTransferMode.resultMatrix[1][0] = 0;		theTransferMode.resultMatrix[1][1] = ff(1.0);		for (counter = 2;counter<4;counter++)			theTransferMode.resultMatrix[1][counter] = 0;		for (counter = 0;counter<2;counter++)			theTransferMode.resultMatrix[2][counter] = 0;		theTransferMode.resultMatrix[2][2] = ff(1.0);		theTransferMode.resultMatrix[2][3] = 0;		for (counter = 0;counter<3;counter++)			theTransferMode.resultMatrix[3][counter] = 0;		theTransferMode.resultMatrix[3][3] = ff(1.0);		for (counter = 0;counter<4;counter++)			theTransferMode.resultMatrix[4][counter] = 0;			theTransferMode.flags = 0;		// Red Component		theTransferMode.component[0].mode				= gxCopyMode;		theTransferMode.component[0].flags				= 0;		theTransferMode.component[0].sourceMinimum		= 0;		theTransferMode.component[0].deviceMinimum		= 0;		theTransferMode.component[0].clampMinimum		= 0;		theTransferMode.component[0].sourceMaximum		= gxColorValue1;		theTransferMode.component[0].deviceMaximum		= gxColorValue1;		// Green Component		theTransferMode.component[1].mode				=  gxCopyMode;		theTransferMode.component[1].flags				= 0;		theTransferMode.component[1].sourceMinimum		= 0;		theTransferMode.component[1].deviceMinimum		= 0;		theTransferMode.component[1].clampMinimum		= 0;		theTransferMode.component[1].sourceMaximum		= gxColorValue1;		theTransferMode.component[1].deviceMaximum		= gxColorValue1;		// Blue Component		theTransferMode.component[2].mode				= gxCopyMode;		theTransferMode.component[2].flags				= 0;		theTransferMode.component[2].sourceMinimum		= 0;		theTransferMode.component[2].deviceMinimum		= 0;		theTransferMode.component[2].clampMinimum		= 0;		theTransferMode.component[2].sourceMaximum		= gxColorValue1;		theTransferMode.component[2].deviceMaximum		= gxColorValue1;		switch (colorComponent)		{			case kRedComp:				theTransferMode.component[0].clampMaximum = 					(theTransferMode.component[0].clampMaximum == 0)	? gxColorValue1 : 0;			break;			case kGreenComp:				theTransferMode.component[1].clampMaximum = 					(theTransferMode.component[1].clampMaximum == 0)	? gxColorValue1 : 0;			break;			case kBlueComp:				theTransferMode.component[2].clampMaximum = 					(theTransferMode.component[2].clampMaximum == 0)	? gxColorValue1 : 0;			break;		}		GXSetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);		GXSetShapeInk((**theDocHndl).theGXImageShape, (**theDocHndl).theInkShape);		SetGWorld((CGrafPtr)theWindow,theGDevHndl);		InvalRect(&theWindow->portRect);		// invalidate window port rect		SetGWorld(oldPort,theGDevHndl);	}	return error;}/****	QDGX Ink object inverter	****/#pragma segment gxGrafixOSErr	GxShapeInkBrightness (ImageDocHndl theDocHndl, WindowPtr theWindow, Boolean isBrighter){	GDHandle			theGDevHndl = nil;	GWorldPtr			oldPort;	gxTransferMode		theTransferMode;	OSErr				error = noErr;	short				counter;		GetGWorld(&oldPort,&theGDevHndl);	GXGetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);	if (&theTransferMode == nil)	{		DisplayAlert(rGenAlert,rQDGXmessages,iInkGetFail);		error = kGXFailGetInk;	}	else	{		theTransferMode.space = gxRGBSpace;		theTransferMode.set = nil;		theTransferMode.profile = nil;				if (isBrighter)		{			for (counter = 0;counter<4;counter++)				theTransferMode.sourceMatrix[counter][counter] += fl(0.1);		}		else		{			for (counter = 0;counter<4;counter++)				theTransferMode.sourceMatrix[counter][counter] -= fl(0.1);		}		GXSetInkTransfer((**theDocHndl).theInkShape, &theTransferMode);		GXSetShapeInk((**theDocHndl).theGXImageShape, (**theDocHndl).theInkShape);		SetGWorld((CGrafPtr)theWindow,theGDevHndl);		InvalRect(&theWindow->portRect);		SetGWorld(oldPort,theGDevHndl);		}	return error;}/****	Create QDGX type rect	****/#pragma segment gxGrafixvoid	CreateGXRect(gxRectangle* theGXRect, short top, short left, short bottom, short right){	theGXRect->top = ff(top);	theGXRect->left = ff(left);	theGXRect->bottom = ff(bottom);	theGXRect->right = ff(right);}/****	get copy of QDGX transform matrix and save for Undo Operation	****/#pragma segment gxGrafixOSErr	SaveTransformCopy(ImageDocHndl theDocHndl, Boolean isUndoOp){	gxTransferMode		theTransform;	gxTransform			theShapeTransform;	OSErr				error = noErr;		if (isUndoOp)	{		switch ((*theDocHndl)->theGXUndoType)		{			case kGXinkTransform:				GXGetInkTransfer((**theDocHndl).theInkShape, &(*theDocHndl)->theUndoInkTransform);			break;			case kGXshapeTransform:				theShapeTransform = GXGetShapeTransform((*theDocHndl)->theGXImageShape);				(*theDocHndl)->theUndoShapeTransform = GXCopyToTransform(nil, theShapeTransform);			break;		}	}	(*theDocHndl)->hasUndoTemp = true;	(*theDocHndl)->theUndoState = kCanUndo;	SetUndoItemText((*theDocHndl)->theUndoState);	return error;}/****	restore QDGX transform matrix for undo Operation	****/#pragma segment gxGrafixOSErr	RestoreOldTransformCopy(ImageDocHndl theDocHndl, Boolean isUndoOp){	GrafPtr				oldPort,						theWindow;	gxTransferMode		theTransform;	gxTransform			theShapeTransform;	OSErr				error = noErr;	if (isUndoOp)	{		switch ((*theDocHndl)->theGXUndoType)		{			case kGXinkTransform:				GXSetInkTransfer((**theDocHndl).theInkShape, &(*theDocHndl)->theUndoInkTransform);				GXSetShapeInk((**theDocHndl).theGXImageShape, (**theDocHndl).theInkShape);			break;			case kGXshapeTransform:				theShapeTransform = GXGetShapeTransform((*theDocHndl)->theGXImageShape);				GXCopyToTransform(theShapeTransform, (*theDocHndl)->theUndoShapeTransform);				GXDisposeTransform((*theDocHndl)->theUndoShapeTransform);			break;		}	}	(*theDocHndl)->theUndoState = kCannotUndo;	SetUndoItemText((*theDocHndl)->theUndoState);	GetPort(&oldPort);	theWindow = FrontWindow();	SetPort(theWindow);	InvalRect(&theWindow->portRect);	SetPort(oldPort);	return error;}