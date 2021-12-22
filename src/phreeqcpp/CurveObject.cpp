// CurveObject.cpp: implementation of the CurveObject class.
//
//////////////////////////////////////////////////////////////////////
#ifdef MULTICHART
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif
#include "CurveObject.h"

#if defined(PHREEQCI_GUI)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CurveObject::CurveObject()
	//
	// default constructor for cxxExchComp
	//
{
	x.clear();
	y.clear();
	this->id = "";
	this->symbol = "";
	this->color = "";
	this->y_axis = 1;
	this->line_w = 1.0;
	this->symbol_size = 6.0;
}


CurveObject::~CurveObject()
{
}

#endif // MULTICHART
