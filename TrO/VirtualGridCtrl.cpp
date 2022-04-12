// VirtualGridCtrl.cpp : implementation file
//


// Written by Yuriy Zabroda <urizabr@hotmail.com>
// Copyright (c) 2002-2005. All Rights Reserved.
// Based on Delphi TGridView component written by Roman Mochalov, (c) 1997-2002.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.

//  --- Last modified: June 24, 2005.  ---

#include "pch.h"
//#include "pch.h"

#include <cmath>

#include "VirtualGridCtrl.h"
#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef max
#undef max
#endif
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
/////////////////////////////////////////////////////////////////////////////
// CVirtualGridCtrl

IMPLEMENT_DYNCREATE(CVirtualGridCtrl, CWnd);


//
CVirtualGridCtrl::CVirtualGridCtrl() : 	m_bShowHeader(TRUE),
					m_bGridLines(TRUE),
					m_nGridLineWidth(1),	//  Don't change
					m_bEndEllipsis(TRUE),
					m_nImageLeftIndent(2),
					m_nImageTopIndent(1),
					m_nTextLeftIndent(6),
					m_nTextRightIndent(6),
					m_nTextTopIndent(2),
					m_bShowFocusRect(TRUE),
					m_bRightClickSelect(TRUE),
					m_bAllowSelect(TRUE),
					m_nCursorKeys(gkArrows | gkMouse | gkMouseWheel),
					m_bColumnsSizing(TRUE),
					m_bColumnClick(TRUE),
					m_checkStyle(cs3D),
					m_nCheckWidth(16),
					m_nCheckHeight(16),
					m_nCheckLeftIndent(0),
					m_nCheckTopIndent(0),
					m_bColResizing(FALSE),
					m_nSortLeftIndent(10),
					m_nSortTopIndent(0),
					m_bCancelOnExit(TRUE),
					m_bColumnsFullDrag(FALSE),
					m_bEditing(FALSE),
					m_nGridStyles(gsHorzLine | gsVertLine),
					m_bRowSelect(FALSE),
					m_bAlwaysEdit(FALSE),
					m_pEdit(0),
					m_bCellSelected(FALSE),
					m_bImageHighlight(0),
					m_bAllowEdit(FALSE),
					m_bReadOnly(FALSE),
					m_bTitleTips(TRUE),	//  Show title tips by default
					m_bHeaderClicking(FALSE),
					m_bHeaderClickState(FALSE),
					m_pHeaderClickSection(0),
					m_bCheckBoxes(FALSE),
					m_bDoubleBuffered(TRUE),
					m_bAlwaysSelected(FALSE),
					m_bHideSelection(FALSE),
					m_pImageList(0)
{
	HINSTANCE hInstance;
	hInstance = AfxGetInstanceHandle();
	WNDCLASS wc;

	if (!(::GetClassInfo(hInstance, VIRTUAL_GRID_CLASS_NAME, &wc))) {
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wc.hIcon = 0;
		wc.hInstance = hInstance;
		wc.lpfnWndProc = ::DefWindowProc;
		wc.lpszClassName = VIRTUAL_GRID_CLASS_NAME;
		wc.lpszMenuName = 0;
		wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	
		if (!AfxRegisterClass(&wc)) {
			AfxThrowResourceException();
			return;
		}
	}

	m_pHorzScrollBar = CreateScrollBar(SB_HORZ);
	m_pVertScrollBar = CreateScrollBar(SB_VERT);
	m_pHeader = CreateHeader();
	m_pColumns = CreateColumns();
	m_pRows = CreateRows();
	m_pFixed = CreateFixed();

	m_gridColor = ::GetSysColor(COLOR_WINDOW);	//  ???

	m_editCell.m_iColumn = -1;
	m_editCell.m_iRow = -1;

	m_hCheckBuffer = CreateBitmap(16, 16, 1, 1, 0);

	CDC ScreenDC;
	VERIFY(ScreenDC.CreateIC(_T("DISPLAY"), NULL, NULL, NULL));
	const int nLogDPIX = ScreenDC.GetDeviceCaps(LOGPIXELSX),
		nLogDPIY = ScreenDC.GetDeviceCaps(LOGPIXELSY);

	if (nLogDPIX == 96 && nLogDPIY == 96)
	{
		;// 96 DPI, Small Fonts
	}
	else
	{
		if (nLogDPIX == 120 && nLogDPIY == 120)
		{
			// 120 DPI, Large Fonts
			this->SetRowsHeight(25);
		}
		else
		{
			// Otherwise, Custom Font Size
			this->SetRowsHeight(25);
		}
	}
}



//  ===================================================================
CVirtualGridCtrl::~CVirtualGridCtrl()
{
	BOOL bSuccess;

	if (m_pEdit)
		delete m_pEdit;

	if (m_bTitleTips && ::IsWindow(m_titleTip.GetSafeHwnd())) {
		bSuccess = m_titleTip.DestroyWindow();
		ASSERT(bSuccess);
	}

	bSuccess = m_font.DeleteObject();
	bSuccess = m_bitmapPattern.DeleteObject();
	bSuccess = ::DeleteObject(m_hCheckBuffer);

	if (m_pFixed)
		delete m_pFixed;
	if (m_pRows)
		delete m_pRows;
	if (m_pColumns)
		delete m_pColumns;
	if (m_pHeader)
		delete m_pHeader;
	if (m_pVertScrollBar)
		delete m_pVertScrollBar;
	if (m_pHorzScrollBar)
		delete m_pHorzScrollBar;
}



//
void CVirtualGridCtrl::PreSubclassWindow() 
{
	BOOL bSuccess;
	CRect rect;
	CDC* pDC;
	int nRetVal;

	if (::IsWindow(m_hWnd)) {
		bSuccess = ModifyStyleEx(0, WS_EX_CLIENTEDGE);
		ASSERT(bSuccess);
	}

	//  Vipe your eyes, Chris :-)
	EnableScrollBarCtrl(SB_BOTH);
	EnableScrollBarCtrl(SB_BOTH, FALSE);

	CWnd* pParentWnd = GetParent();
	
   
	if (pParentWnd) {
		CFont* pFont;
		LOGFONT lf;
		int nRetVal;

		pFont = pParentWnd->GetFont();
		if (pFont != NULL)
		{
			nRetVal = pFont->GetLogFont(&lf);
			bSuccess = m_font.CreateFontIndirect(&lf);
		}
		else
		{
			lf.lfHeight=11;   
			lf.lfWidth=0;   
			lf.lfEscapement=0;   
			lf.lfOrientation=0;   
			lf.lfWeight=FW_NORMAL;   
			lf.lfItalic=0;   
			lf.lfUnderline=0;   
			lf.lfStrikeOut=0;   
			lf.lfCharSet=ANSI_CHARSET;   
			lf.lfOutPrecision=OUT_DEFAULT_PRECIS;   
			lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;   
			lf.lfQuality=PROOF_QUALITY;   
			lf.lfPitchAndFamily=VARIABLE_PITCH | FF_ROMAN;   
			wcscpy(lf.lfFaceName, _T("MS Sans Serif"));
			bSuccess = m_font.CreateFontIndirect(&lf);
		}

		m_pHeader->m_pFont = &m_font;
		m_pFixed->m_pFont = &m_font;
		SetFont(&m_font);
	}

	m_titleTip.SetParentWnd(this);


	pDC = GetDC();
	bSuccess = m_bitmapPattern.CreateCompatibleBitmap(pDC, 2, 2);
	ASSERT(bSuccess);
	nRetVal = ReleaseDC(pDC);
	ASSERT(nRetVal);
}




//
BOOL CVirtualGridCtrl::Create(
			      const RECT& rect,
			      CWnd* pParentWnd,
			      UINT nID,
			      DWORD dwStyle
			      )
{
	return CWnd::Create(
		VIRTUAL_GRID_CLASS_NAME,
		_T(""),
		dwStyle,
		rect,
		pParentWnd,
		nID
		);
}




//
void CVirtualGridCtrl::Paint(CDC* pDC)
{
	CRect rect;
	CRect fixedRect;
	int nRetVal;

	//  ----------------  Paint the grid header  ---------------------
	rect = GetHeaderRect();
	if (m_bShowHeader && pDC->RectVisible(rect))  {
		//  Fixed part
		PaintHeaders(pDC);
		//  Cut off fixed headers
		fixedRect = GetFixedRect();
		nRetVal = pDC->ExcludeClipRect(fixedRect.left, rect.top, fixedRect.right, rect.bottom);
	}

	//  Regular part
	PaintHeaders(pDC, FALSE);
	
	//  Cut-off the header rectangle
	nRetVal = pDC->ExcludeClipRect(rect);

	//  Fields to the right and to the bottom
	PaintFreeField(pDC);

	//  -----------------  Paint the fixed part of the Grid  --------------------
	rect = GetFixedRect();
	if (m_pFixed->GetCount() && pDC->RectVisible(rect))  {
		//  Cells...
		PaintFixed(pDC);
		//  Mesh
		if (m_bGridLines)
			PaintFixedGrid(pDC);

		//  Cut off fixed part
		nRetVal = pDC->ExcludeClipRect(rect);
	}


	//  -------------  Paint regular cells  ----------------------
	if (m_visSize.m_iColumn > 0 && m_visSize.m_iRow > 0) {
		//  Cut off edit control (if present)
		if (m_bEditing)
			nRetVal = pDC->ExcludeClipRect(GetEditRect(m_editCell));

		//  Cells cells cells
		PaintCells(pDC);

		if (IsFocusAllowed())
			PaintFocus(pDC);
	}

	//  Mesh
	if (m_bGridLines)
		PaintGridLines(pDC);
}




//
void CVirtualGridCtrl::ApplyEdit()
{
	SetEditing(FALSE);
}


//
void CVirtualGridCtrl::CancelEdit()
{
	if (!m_bEditing)
		return;		//  Nothing to cancel

	if (m_bAlwaysEdit)
		UpdateEditContents(FALSE);
	else
		HideEdit();
}



//
void CVirtualGridCtrl::DefaultDrawCell(CDC* pDC, const CGridCell& cell, LPRECT lpRect)
{
	const UINT ds[] = {ILD_NORMAL, ILD_SELECTED};
	UINT ids;
	CRect defRect(*lpRect);
	CRect rect(*lpRect);
	GridCheckKind checkKind;
	CRect cellRect;
	int nImage;
	int nRetVal;
	int nWidth, nHeight;
	CPoint point;
	BOOL bRowHighlight, bCellHighlight, bSh, bImageHighlight, bSuccess;
	LOGBRUSH lb;
	CPoint textIndent;
	TCHAR szBuffer[MAX_CELL_TEXT];
	HBITMAP hBitmapCheck;
	int nAlign;

	//  Offset borders to avoid filling grid lines.
	if (m_bGridLines)  {
		if (m_nGridStyles & gsVertLine)
			rect.right -= m_nGridLineWidth;
		if (m_nGridStyles & gsHorzLine)
			rect.bottom -= m_nGridLineWidth;
	}

	//  Get type of cell's "check box" and image index.
	checkKind = GetCheckKind(cell);
	nImage = GetCellImage(cell);

	//  Find out the indication of image highlight
	bRowHighlight = m_bRowSelect && (cell.m_iColumn == m_pFixed->m_nCount)
		&& (cell.m_iRow == m_cellFocused.m_iRow);
	bCellHighlight = !m_bRowSelect && (cell == m_cellFocused);
	
	nRetVal = pDC->GetCurrentBrush()->GetLogBrush(&lb);
	bSh = (lb.lbColor == ::GetSysColor(COLOR_BTNHIGHLIGHT)
		|| lb.lbColor == ::GetSysColor(COLOR_BTNFACE));

	bImageHighlight = (!m_bImageHighlight) && (bRowHighlight || bCellHighlight) && bSh;

	//  -------------  Draw the flag  ----------------
	if (checkKind != gcNone)  {
		CRect checkRect;

		//  Get flag's rectangle
		rect.right = min(rect.left + m_nCheckWidth + GetCheckIndent(cell).x, rect.right);

		//  Is the flag visible
		if (rect.left < defRect.right)  {
			//  Let's paint
			point.x = rect.left + GetCheckIndent(cell).x;
			point.y = rect.top + GetCheckIndent(cell).y;

			//  Flag's size
			nWidth = m_nCheckWidth;
			if (point.x + nWidth > rect.right)
				nWidth = rect.right - point.x;
			nHeight = m_nCheckHeight;
			if (point.y + nHeight > rect.bottom)
				nHeight = rect.bottom - point.y;

			if (checkKind != gcUserDefine)  {
				UINT nState;

				//  Default is check box-like
				if (checkKind == gcRadioButton)
					nState = DFCS_BUTTONRADIO;
				else
					nState = DFCS_BUTTONCHECK;

				switch (GetCheckState(cell))  {
				case 0:		//  Unchecked
					break;
				case 1:		//  Checked
					nState |= DFCS_CHECKED;
					break;
				case 2:
					nState |= DFCS_INACTIVE;
					break;
				default:
					break;
				}


				if (m_checkStyle == csFlat)
					nState |= DFCS_FLAT;

				pDC->FillSolidRect(rect, ::GetSysColor(COLOR_WINDOW));

				checkRect = rect;
				checkRect.DeflateRect(1, 1);
				bSuccess = pDC->DrawFrameControl(
					checkRect,
					DFC_BUTTON,
					nState
					);
				ASSERT(bSuccess);
			}  else  {
				BITMAP bm;

				hBitmapCheck = m_hCheckBuffer;

				GetCheckImage(cell, &hBitmapCheck);
				CBitmap* pCheckBitmap = CBitmap::FromHandle(hBitmapCheck);

				pDC->FillRect(rect, pDC->GetCurrentBrush());

				pCheckBitmap->GetBitmap(&bm);
				if (bm.bmWidth > 0 && bm.bmHeight > 0)  {
					CDC dc;
					CBitmap* pOldBitmap;

					bSuccess = dc.CreateCompatibleDC(pDC);

					pOldBitmap = dc.SelectObject(pCheckBitmap);

					bSuccess = pDC->BitBlt(
						point.x,
						point.y,
						nWidth,
						nHeight,
						&dc,
						0,
						0,
						SRCCOPY
						);

					pOldBitmap = dc.SelectObject(pOldBitmap);
					bSuccess = dc.DeleteDC();
					ASSERT(bSuccess);
				}

			}	//  Check kind is gcUserDefine.

			lpRect->left = rect.right;
		}
	}

	//  ---------  Draw the image (if present) ------------
	if (nImage >= 0)  {
		IMAGEINFO ii;

		bSuccess = m_pImageList->GetImageInfo(nImage, &ii);
		ASSERT(bSuccess);
		//  if (m_bImageHighlight)
		//  ?????

		rect = *lpRect;

		//  Get image rectangle
		rect.right = min(rect.left + (ii.rcImage.right - ii.rcImage.left) + GetCellImageIndent(cell).x, rect.right);

		//  Is image visible?
		if (rect.left < defRect.right)  {
			point.x = rect.left + GetCellImageIndent(cell).x;
			point.y = rect.top + GetCellImageIndent(cell).y;

			//  Image size (for narrow columns cut-off purposes)
			nWidth = ii.rcImage.right - ii.rcImage.left;
			if (point.x + nWidth > rect.right)
				nWidth = rect.right - nWidth;
			nHeight = ii.rcImage.bottom - ii.rcImage.top;
			if (point.y + nHeight > rect.bottom)
				nHeight = rect.bottom - point.y;

			//  Image's style and background colors...
			ids = ds[IsCellFocused(cell) && m_bCellSelected && m_bImageHighlight];

			//  Paint the image
			pDC->FillSolidRect(rect, pDC->GetBkColor());

			m_pImageList->Draw(pDC, nImage, point, ids);
			lpRect->left = rect.right;
		}
	}

	//  Restore background color
	GetCellColors(cell, pDC);

	//  Paint the text if edit control is not visible
	if (!(cell == m_editCell && !IsFocusAllowed()))  {
		CGridColumn* pColumn;

		//  Get text rectangle
		rect = *lpRect;

		//  Is the text visible
		if (rect.left >= defRect.right)
			return;

		//  Get text draw parameters
		textIndent = GetCellTextIndent(cell);
		pColumn = m_pColumns->GetAt(cell.m_iColumn);
		GetCellText(szBuffer, cell, nAlign);

		pDC->FillSolidRect(rect, pDC->GetBkColor());

		PaintText(
			pDC,
			rect,
			textIndent.x,
			textIndent.y,
			nAlign,
			pColumn->m_bWantReturns,
			pColumn->m_bWordWrap,
			szBuffer
			);
	}
}




//
void CVirtualGridCtrl::DefaultDrawHeader(CDC* pDC, CGridHeaderSection* pSection, LPRECT lpRect)
{
	CRect defRect(*lpRect);
	int nIndex;
	CPoint point;
	DRAWTEXTPARAMS dtp;
	UINT nFlags = 0;
	CRect rect(*lpRect);
	GridSortDirection sd;
	CRect textRect;
	int nRetVal;
	CPen* pOldPen;
	BOOL bSuccess;


	//  "FillSolidRect"
	bSuccess = ExtTextOut(pDC->m_hDC, 0, 0, ETO_OPAQUE, lpRect, 0, 0, 0);
	ASSERT(bSuccess);

	//  If the section was pressed then we should offset both the picture and the text
	if (IsHeaderPressed(pSection))
		bSuccess = OffsetRect(lpRect, 1, 1);

	//  Get an index of header image.
	nIndex = GetHeaderImage(pSection);

	//  Paint the image if present
	if (nIndex >= 0) {
		IMAGEINFO ii;
		int nWidth, nHeight;

		//  Get image rectangle.
		bSuccess = m_pHeader->m_pImageList->GetImageInfo(nIndex, &ii);
		rect = *lpRect;
		rect.right = min(rect.left + (ii.rcImage.right - ii.rcImage.left) + 2, rect.right);

		//  If the image is visible then paint it
		if (rect.left < lpRect->right)  {
			point.x = rect.left + 2;
			point.y = rect.top + 1 + (!m_pHeader->m_bFlat);
		}

		nWidth = ii.rcImage.right - ii.rcImage.left;
		if (point.x + nWidth > rect.right)
			nWidth = rect.right - point.x;

		nHeight = ii.rcImage.bottom - ii.rcImage.top;
		if (point.y + nHeight > rect.bottom)
			nHeight = rect.bottom - point.y;

		bSuccess = GetHeader()->m_pImageList->Draw(pDC, nIndex, point, ILD_NORMAL);
		lpRect->left = rect.right;
	}


	//  Is the text visible?
	if (lpRect->left < lpRect->right)  {
		//  Memorize length of header caption.
		int nTextLength = pSection->GetCaption().GetLength();
		TCHAR szCaption[1024];
		wcscpy(szCaption, pSection->GetCaption());

		sd = gsNone;
		if (pSection->GetLevel() == m_pHeader->GetMaxLevel())
			sd = GetSortDirection(pSection);

		dtp.cbSize = sizeof(DRAWTEXTPARAMS);
		dtp.iLeftMargin = 2 + 4 * (nIndex < 0);
		dtp.iRightMargin = 6;

		//
		nFlags = DT_END_ELLIPSIS | DT_NOPREFIX;
		switch (pSection->m_nAlignment)  {
		case LVCFMT_LEFT:
			nFlags |= DT_LEFT;
			break;
		case LVCFMT_RIGHT:
			nFlags |= DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nFlags |= DT_CENTER;
			break;
		default:
			break;
		}

		if (pSection->m_bWordWrap)
			nFlags |= DT_WORDBREAK;

		if (sd != gsNone)  {
			//  ----------  Paint sort direction token.  ------------
			CRect sortRect;
			POINT pt;
			int nWidth = 8;
			int nHeight = 8;

			//  Calculate text width.
			rect = *lpRect;
			if (nTextLength > 0)  {
				nRetVal = DrawTextEx(
					pDC->m_hDC,
					szCaption,
					nTextLength,
					&rect,
					nFlags | DT_CALCRECT,
					&dtp
					);

				rect.top = lpRect->top + ((lpRect->bottom - lpRect->top) - (rect.Height())) / 2;
			}  else  {
				rect.right = rect.left;
				rect.top = lpRect->top + 2 * (!GetHeader()->m_bFlat);
			}

			//  Calculate sort rectangle.
			sortRect.left = min(lpRect->right - nWidth - 10, rect.right + m_nSortLeftIndent);
			sortRect.left = max(lpRect->left + dtp.iLeftMargin, sortRect.left);
			sortRect.right = min(lpRect->right - 6, sortRect.left + nWidth);
			sortRect.top = rect.top + m_nSortTopIndent;
			sortRect.bottom = min(lpRect->bottom, sortRect.top + nHeight);

			//  Calculate new sort token size.
			nWidth = sortRect.Width();
			nHeight = sortRect.Height();

			pt.x = sortRect.left;
			pt.y = sortRect.top;

			PaintSortImage(pDC, &pt, sd);

			//  Adjust text rectangle.
			lpRect->right = sortRect.left - m_nSortLeftIndent;
		}

		//  Draw the text
		if (pSection->m_strCaption.GetLength() && rect.Width())  {
			TCHAR szBuffer[1024];
			wcscpy(szBuffer, pSection->m_strCaption);

			//  First, we should calculate text rectangle
			textRect = *lpRect;
			nRetVal = ::DrawTextEx(
				pDC->m_hDC,
				szBuffer,
				lstrlen(szBuffer),
				textRect,
				nFlags | DT_CALCRECT,
				&dtp
				);

			//  Use vertical alignment for sections without images
			if (nIndex < 0)
				textRect.OffsetRect(0, ((lpRect->bottom - lpRect->top - textRect.Height()) / 2));
			else
				textRect.OffsetRect(0, 2 + 2 * !m_pHeader->m_bFlat);

			//  Adjust left and right
			textRect.left = lpRect->left;
			textRect.right = lpRect->right;

			//  Text out
			pDC->SetBkMode(TRANSPARENT);
			nRetVal = ::DrawTextEx(
				pDC->m_hDC,
				szBuffer,
				lstrlen(szBuffer),
				textRect,
				nFlags,
				&dtp
				);
		}
	}

	//  ----------------------  Paint divider  --------------------------
	*lpRect = defRect;

	CPen penBtnShadow(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));

	//  What to paint: a button or some lines?
	if (m_pHeader->m_bFlat) {
		//  If the colors of header and the table are the same then
		//  we should paint single line
		if (m_pHeader->m_bGridColor)  {
			CPen pen(PS_SOLID, m_nGridLineWidth, ::GetSysColor(COLOR_ACTIVEBORDER));

			pOldPen = pDC->SelectObject(&pen);

			//  Line at the bottom
			point = pDC->MoveTo(rect.left, rect.bottom - 1);
			bSuccess = pDC->LineTo(rect.right - 1, rect.bottom - 1);

			//  Line to the right
			point = pDC->MoveTo(rect.right - 1, rect.top);
			bSuccess = pDC->LineTo(rect.right - 1, rect.bottom);

			pOldPen = pDC->SelectObject(pOldPen);
			bSuccess = pen.DeleteObject();
			ASSERT(bSuccess);
		}  else  {
			CPen penBtnHighlight(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));


			//  Line at the bottom
			pOldPen = pDC->SelectObject(&penBtnShadow);
			point = pDC->MoveTo(rect.left, rect.bottom - 2);
			bSuccess = pDC->LineTo(rect.right - 1, rect.bottom - 2);

			pDC->SelectObject(&penBtnHighlight);
			point = pDC->MoveTo(rect.left, rect.bottom - 1);
			bSuccess = pDC->LineTo(rect.right - 1, rect.bottom - 1);

			//  Line to the right
			pDC->SelectObject(&penBtnShadow);
			point = pDC->MoveTo(rect.right - 2, rect.top);
			bSuccess = pDC->LineTo(rect.right - 2, rect.bottom - 1);

			pDC->SelectObject(&penBtnHighlight);
			point = pDC->MoveTo(rect.right - 1, rect.top);
			bSuccess = pDC->LineTo(rect.right - 1, rect.bottom);

			pOldPen = pDC->SelectObject(pOldPen);
		}
	}  else  {		//  The Grid's Header ain't no flat
		//  Paint the button frame
		if (IsHeaderPressed(pSection))
			bSuccess = pDC->DrawEdge(lpRect, BDR_SUNKENOUTER, BF_RECT | BF_FLAT);
		else
			Paint3DFrame(pDC, lpRect, BF_RECT);

		//  Adjust section's rectangle
		bSuccess = InflateRect(lpRect, -2, -2);
	}

	bSuccess = penBtnShadow.DeleteObject();
	ASSERT(bSuccess);
}




//
void CVirtualGridCtrl::DrawDragRect(CDC* pDC, const CGridCell& cell)
{
	int nInterType;

	if (!IsCellVisible(cell))
		return;		//  Nothing to draw

	//  Cell's rectangle
	CRect rect(GetEditRect(cell));

	//  Colors
	GetCellColors(m_cellFocused, pDC);

	//  --------  Let's paint  ----------
	
	//  Cut off some place for the header and fixed part
	CRect gridRect;
	gridRect = GetGridRect();

	nInterType = pDC->IntersectClipRect(
		GetFixedRect().right,
		gridRect.top,
		gridRect.right,
		gridRect.bottom
		);

	//  Paint the focus
	pDC->DrawFocusRect(rect);
}




//
CGridCell CVirtualGridCtrl::GetCellAt(int x, int y)
{
	CGridCell cell;

	int iColumn = GetColumnAt(x, y);
	int iRow = GetRowAt(x, y);

	if (iColumn >= 0 && iRow >= 0)  {
		cell.m_iColumn = iColumn;
		cell.m_iRow = iRow;
	}  else  {
		cell.m_iColumn = -1;
		cell.m_iRow = -1;
	}

	return cell;
}



//
CRect CVirtualGridCtrl::GetCellRect(const CGridCell& cell)
{
	CRect rect;
	CRect rowRect;

	rect = GetColumnLeftRight(cell.m_iColumn);
	rowRect = GetRowTopBottom(cell.m_iRow);

	rect.top = rowRect.top;
	rect.bottom = rowRect.bottom;

	return rect;
}



//
int CVirtualGridCtrl::GetColumnAt(int x, int y)
{
	CRect rect;

	GetClientRect(rect);

	//  Look through fixed columns...
	int nLeft = rect.left;
	int nRight = 0;
	int i;

	for (i = 0; i < GetFixed()->m_nCount; ++i) 
	{
		nRight = nLeft + m_pColumns->GetAt(i)->GetWidth();

		if (nRight != nLeft && x >= nLeft && x < nRight)
			return i;

		nLeft = nRight;
	}

	//  Look through usual columns
	nLeft += GetGridOrigin().x;
	for (;i < m_pColumns->GetSize(); ++i)
	{
		nRight = nLeft + m_pColumns->GetAt(i)->GetWidth();
		if (nRight != nLeft && x >= nLeft && x < nRight)
			return i;
		nLeft = nRight;
	}

	return -1;
}



//
CRect CVirtualGridCtrl::GetColumnLeftRight(int iColumn)
{
	CRect rect;

	//  Check the column...
	if (!m_pColumns->GetSize())  {
		//  There's no columns at all
		rect.left = GetGridRect().left;
		rect.right = rect.left;
	}  else if (iColumn < 0)  {
		//  The column is leftier than the leftmost
		rect = GetColumnLeftRight(0);
		rect.right = rect.left;
	}  else if (iColumn > m_pColumns->GetUpperBound())  {
		//  The column is rightier than the rightmost
		rect = GetColumnLeftRight(m_pColumns->GetUpperBound());
		rect.left = rect.right;
	}  else  {
		//  Normal column
		GetClientRect(rect);
		rect.left += GetColumnsWidth(0, iColumn - 1);
		if (iColumn >= m_pFixed->GetCount())
			rect.left += GetGridOrigin().x;
		rect.right = rect.left + m_pColumns->GetAt(iColumn)->GetWidth();
	}

	return rect;
}



//
int CVirtualGridCtrl::GetColumnMaxWidth(int iColumn)
{
	int nMaxWidth;
	int i, nWidth;
	CGridCell cell;
	CRect rect;

	//  Check the column...
	if (iColumn < 0 || iColumn > m_pColumns->GetUpperBound())
		return 0;

	//  Is there visible rows?
	if (m_visSize.m_iRow == 0)
		return m_pColumns->GetAt(iColumn)->m_nWidth;

	nMaxWidth = 0;

	for (i = 0; i < m_visSize.m_iRow; ++i) {
		cell.m_iColumn = iColumn;
		cell.m_iRow = m_visOrigin.m_iRow + i;

		//  Find out text rectangle
		rect = GetCellTextBounds(cell);
		nWidth = rect.Width();

		//  Place for "flag"
		if (IsCellHasCheck(cell))
			nWidth += m_nCheckWidth + GetCheckIndent(cell).x;
		//  Place for image
		if (IsCellHasImage(cell))  {
			IMAGEINFO ii;
			BOOL bSuccess;

			bSuccess = m_pImageList->GetImageInfo(0, &ii);
			ASSERT(bSuccess);

			nWidth += (ii.rcImage.right - ii.rcImage.left) + GetCellImageIndent(cell).x;
		}
		
		//  Taking into account the grid lines
		if (m_bGridLines && (gsVertLine & m_nGridStyles))
			nWidth += m_nGridLineWidth;

		if (nMaxWidth < nWidth)
			nMaxWidth = nWidth;
	}

	return nMaxWidth;
}


//
CRect CVirtualGridCtrl::GetColumnRect(int iColumn)
{
	CRect rect;

	rect = GetColumnLeftRight(iColumn);
	rect.top = GetRowTopBottom(0).top;
	rect.bottom = GetRowTopBottom(m_pRows->GetCount() - 1).bottom;

	return rect;
}



//
CRect CVirtualGridCtrl::GetColumnsRect(int nFirstColumn, int nLastColumn)
{
	CRect rect;
	CRect firstRect, secondRect;
	BOOL bSuccess;

	firstRect = GetColumnRect(nFirstColumn);
	secondRect = GetColumnRect(nLastColumn);

	bSuccess = rect.UnionRect(firstRect, secondRect);
	return rect;
}



//
int CVirtualGridCtrl::GetColumnsWidth(int nFirstColumn, int nLastColumn)
{
	int nWidth = 0;
	int i;

	//  Check the indices
	nFirstColumn = max(nFirstColumn, 0);
	nLastColumn = min(m_pColumns->GetUpperBound(), nLastColumn);

	for (i = nFirstColumn; i <= nLastColumn; ++i)
		nWidth += m_pColumns->GetAt(i)->GetWidth();

	return nWidth;
}



//
CRect CVirtualGridCtrl::GetEditRect(const CGridCell& cell)
{
	CRect rect;

	rect = GetCellRect(cell);

	//  Place for check box...
	if (IsCellHasCheck(cell))
		rect.left += m_nCheckWidth + GetCheckIndent(cell).x;

	//  Place for image...
	if (IsCellHasImage(cell))  {
		IMAGEINFO ii;
		BOOL bSuccess;

		bSuccess = m_pImageList->GetImageInfo(0, &ii);
		ASSERT(bSuccess);

		rect.left += (ii.rcImage.right - ii.rcImage.left) + GetCellImageIndent(cell).x;
	}

	//  Taking into account grid lines...
	if (m_bGridLines)  {
		if (gsVertLine & m_nGridStyles)
			rect.right -= m_nGridLineWidth;
		if (gsHorzLine & m_nGridStyles)
			rect.bottom -= m_nGridLineWidth;
	}


	//  Check the right border
	if (rect.left > rect.right)
		rect.left = rect.right;

	return rect;
}


//
int CVirtualGridCtrl::GetFirstImageColumn()
{
	int i;

	for (i = m_pFixed->GetCount(); i < m_pColumns->GetSize(); ++i) {
		if (m_pColumns->GetAt(i)->m_bVisible)
			return i;
	}

	return -1;
}


//
CRect CVirtualGridCtrl::GetFixedRect()
{
	CRect rect;

	rect = GetGridRect();
	rect.right = rect.left + GetColumnsWidth(0, m_pFixed->m_nCount - 1);

	return rect;
}



//
int CVirtualGridCtrl::GetFixedWidth()
{
	return GetFixedRect().Width();
}



//
CRect CVirtualGridCtrl::GetFocusRect()
{
	CRect rect;
	CGridCell cell;
	int nLeft;

	if (m_bRowSelect)
		rect = GetRowRect(m_cellFocused.m_iRow);
	else
		rect = GetCellRect(m_cellFocused);

	cell.m_iColumn = m_cellFocused.m_iColumn;

	if (m_bRowSelect)
		cell.m_iColumn = m_pFixed->GetCount();

	cell.m_iRow = m_cellFocused.m_iRow;

	//  If cell's picture selectable
	if (!m_bImageHighlight)  {
		//  Place for small flag
		if (IsCellHasCheck(cell))
			rect.left += m_nCheckWidth + GetCheckIndent(cell).x;
		
		//  A place for an image.
		if (IsCellHasImage(cell))  {
			IMAGEINFO ii;
			BOOL bSuccess;

			bSuccess = m_pImageList->GetImageInfo(0, &ii);
			ASSERT(bSuccess);

			rect.left += (ii.rcImage.right - ii.rcImage.left) + GetCellImageIndent(cell).x;
		}
	}

	//  Check the right border
	nLeft = GetCellRect(cell).right;
	if (rect.left > nLeft)
		rect.left = nLeft;

	return rect;
}



//
int CVirtualGridCtrl::GetGridHeight()
{
	return GetGridRect().Height();
}



//
CPoint CVirtualGridCtrl::GetGridOrigin()
{
	CPoint point(0, 0);

	point.x = -m_pHorzScrollBar->GetPosition();
	point.y = -m_pVertScrollBar->GetPosition() * m_pRows->GetHeight();

	return point;
}




//
CRect CVirtualGridCtrl::GetGridRect()
{
	CRect rect;

	GetClientRect(rect);
	rect.top = GetHeaderRect().bottom;

	return rect;
}



//
int CVirtualGridCtrl::GetHeaderHeight()
{
	return m_pHeader->GetHeight();
}



//
CRect CVirtualGridCtrl::GetHeaderRect()
{
	CRect rect;

	GetClientRect(rect);
	rect.bottom = rect.top;

	if (m_bShowHeader)
		rect.bottom += GetHeaderHeight();

	return rect;
}



//
CGridHeaderSection* CVirtualGridCtrl::GetHeaderSection(int iColumn, int iLevel)
{
	return m_pHeader->m_sections.GetSection(iColumn, iLevel);
}



//
CGridHeaderSection* CVirtualGridCtrl::GetResizeSectionAt(CPoint point)
{
	CGridHeaderSection* pSection;

	if (m_pHeader->m_sections.FindResizeSection(point, pSection))
		return pSection;

	return 0;
}



//
int CVirtualGridCtrl::GetRowAt(int x, int y)
{
	int nRow;
	int grt, goy;

	if (m_pRows->m_nHeight <= 0)
		return -1;

	grt = GetGridRect().top;
	goy = GetGridOrigin().y;

	if (y - grt - goy < 0)
		return -1;	//  That is the key moment


	nRow = (y - grt - goy) / m_pRows->m_nHeight;

	//  Check the cell
	if (nRow >= 0 && nRow < m_pRows->m_nCount)
		return nRow;

	return -1;
}



//
CRect CVirtualGridCtrl::GetRowRect(int nRow)
{
	CRect rect;
	RECT rc;

	rect = GetRowTopBottom(nRow);
	GetClientRect(&rc);

	rect.left = min(rc.left, GetColumnLeftRight(m_pFixed->GetCount()).left);
	rect.right = GetColumnLeftRight(m_pColumns->GetUpperBound()).right;

	return rect;
}



//
CRect CVirtualGridCtrl::GetRowsRect(int nFirstRow, int nLastRow)
{
	CRect firstRect, lastRect;
	BOOL bSuccess;

	firstRect = GetRowRect(nFirstRow);
	lastRect = GetRowRect(nLastRow);

	bSuccess = firstRect.UnionRect(firstRect, lastRect);
	return firstRect;
}



//
int CVirtualGridCtrl::GetRowsHeight(int nFirstRow, int nLastRow)
{
	if (nFirstRow > nLastRow)
		return 0;

	return (nLastRow - nFirstRow + 1) * m_pRows->m_nHeight;
}



//
CRect CVirtualGridCtrl::GetRowTopBottom(int nRow)
{
	CRect rect;

	rect.top = GetGridRect().top  + GetRowsHeight(0, nRow - 1) + GetGridOrigin().y;
	rect.bottom = rect.top + m_pRows->m_nHeight;

	return rect;
}


//
CGridHeaderSection* CVirtualGridCtrl::GetSectionAt(CPoint point)
{
	CGridHeaderSection* pSection;

	if (m_pHeader->m_sections.FindSection(point, pSection))
		return pSection;

	return 0;
}



//
void CVirtualGridCtrl::PaintCells(CDC* pDC)
{
	int i, j;
	int nLeft, nTop;
	CRect rect;
	int nWidth;

	//  Left and top border of visible cells
	nLeft = GetColumnLeftRight(m_visOrigin.m_iColumn).left;
	nTop = GetRowTopBottom(m_visOrigin.m_iRow).top;

	//  Initialize top border
	rect.bottom = nTop;

	//  Look through the rows
	for (j = 0; j < m_visSize.m_iRow; ++j) {
		//  Offset rectangle in vertical direction
		rect.top = rect.bottom;
		rect.bottom += m_pRows->m_nHeight;

		//  Initialize left border
		rect.right = nLeft;
		
		//  Look though columns
		for (i = 0; i < m_visSize.m_iColumn; ++i) {
			CGridCell cell(m_visOrigin.m_iColumn + i, m_visOrigin.m_iRow + j);

			nWidth = m_pColumns->GetAt(cell.m_iColumn)->GetWidth();

			//  Paint only visible cells
			if (nWidth > 0)  {
				//  Offset rectangle in horizontal direction
				rect.left = rect.right;
				rect.right += nWidth;

				//  Paint the cell
				if (pDC->RectVisible(rect))
					PaintCell(pDC, cell, rect);
			}
		}
	}
}



//
void CVirtualGridCtrl::InvalidateCell(const CGridCell& cell)
{
	if (cell.m_iRow == m_cellFocused.m_iRow || cell.m_iColumn == m_cellFocused.m_iColumn)
		HideFocus();

	InvalidateRect(GetCellRect(cell));

	if (cell.m_iRow == m_cellFocused.m_iRow || cell.m_iColumn == m_cellFocused.m_iColumn)
		ShowFocus();
}



//
void CVirtualGridCtrl::InvalidateCheck(const CGridCell& cell)
{
	InvalidateRect(GetCheckRect(cell));
}


//
void CVirtualGridCtrl::InvalidateColumn(int iColumn)
{
	HideFocus();
	InvalidateRect(GetColumnRect(iColumn));
	ShowFocus();
}


//
void CVirtualGridCtrl::InvalidateColumns(int nFirstColumn, int nLastColumn)
{
	HideFocus();
	InvalidateRect(GetColumnsRect(nFirstColumn , nLastColumn));
	ShowFocus();
}


//
void CVirtualGridCtrl::InvalidateEdit()
{
	if (m_bEditing)
		m_pEdit->Invalidate();
}


//
void CVirtualGridCtrl::InvalidateFixed()
{
	InvalidateRect(GetFixedRect());
}


//
void CVirtualGridCtrl::InvalidateFocus()
{
	CRect rect;
	BOOL bSuccess;

	rect = GetFocusRect();

	//  Adjust focus rectangle (it doesn't suit the picture)
	if (m_bRowSelect)
		bSuccess = rect.UnionRect(rect, GetCellRect(CGridCell(m_pFixed->m_nCount, m_cellFocused.m_iRow)));
	else
		bSuccess = rect.UnionRect(rect, GetCellRect(m_cellFocused));

	InvalidateRect(rect);
}



//
void CVirtualGridCtrl::InvalidateGrid()
{
	InvalidateEdit();
	InvalidateRect(GetGridRect());
}


//
void CVirtualGridCtrl::InvalidateHeader()
{
	if (m_bShowHeader)
		InvalidateRect(GetHeaderRect());
}


//
void CVirtualGridCtrl::InvalidateRow(int iRow)
{
	if (iRow == m_cellFocused.m_iRow)
		HideFocus();

	InvalidateRect(GetRowRect(iRow));

	if (iRow == m_cellFocused.m_iRow)
		ShowFocus();
}


//
void CVirtualGridCtrl::InvalidateRows(int nFirstRow, int nLastRow)
{
	HideFocus();
	InvalidateRect(GetRowsRect(nFirstRow, nLastRow));
	ShowFocus();
}



//
BOOL CVirtualGridCtrl::IsActiveControl()
{
	if (GetParent() && this == GetActiveWindow())
		return TRUE;

	if (m_pEdit && m_pEdit == GetFocus())
		return TRUE;

	if (this == GetFocus())
		return TRUE;

	return FALSE;
}


//
BOOL CVirtualGridCtrl::IsCellAcceptCursor(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	if (!IsCellValid(cell))
		return FALSE;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_CELLACCEPTCURSOR;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	//  Default result
	dispInfo.item.bAccept = (cell.m_iColumn >= m_pFixed->GetCount()) && ((*m_pColumns)[cell.m_iColumn]->GetTabStop());

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.bAccept;
}



//
BOOL CVirtualGridCtrl::IsCellHasCheck(const CGridCell& cell)
{
	return (m_bCheckBoxes && gcNone != GetCheckKind(cell));
}


//
BOOL CVirtualGridCtrl::IsCellHasImage(const CGridCell& cell)
{
	return (m_pImageList && -1 != GetCellImage(cell));
}


//
BOOL CVirtualGridCtrl::IsCellFocused(const CGridCell& cell)
{
	return ((cell.m_iColumn == m_cellFocused.m_iColumn) || m_bRowSelect)
		&& (cell.m_iRow == m_cellFocused.m_iRow)
		&& (cell.m_iColumn >= m_pFixed->m_nCount);
}



//
BOOL CVirtualGridCtrl::IsCellReadOnly(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	if (!IsCellValid(cell))
		return TRUE;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);

	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_GETALLOWEDIT;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	//  We are allowed to edit the cell if the grid is not read only AND cell's column
	//  is not fixed AND the column itself is not read only
	dispInfo.item.bReadOnly = m_bReadOnly || cell.m_iColumn < m_pFixed->GetCount() || m_pColumns->GetAt(cell.m_iColumn)->m_bReadOnly;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	return dispInfo.item.bReadOnly;
}




//
BOOL CVirtualGridCtrl::IsCellValid(
				     const CGridCell& cell,
				     BOOL bCheckPosition,	//  = TRUE
				     BOOL bCheckVisible		//  = TRUE
				     )
{
	BOOL bCorrectColumn;
	BOOL bCorrectRow;
	BOOL bVisible = FALSE;

	bCorrectColumn = cell.m_iColumn >= 0 && cell.m_iColumn < m_pColumns->GetSize();
	bCorrectRow = cell.m_iRow >= 0 && cell.m_iRow < m_pRows->m_nCount;

	bVisible = bCorrectColumn && m_pColumns->GetAt(cell.m_iColumn)->GetVisible()
		&& m_pColumns->GetAt(cell.m_iColumn)->m_nWidth > 0;


	return (!bCheckPosition || (bCorrectColumn && bCorrectRow)) && (!bCheckVisible || bVisible);
}



//
BOOL CVirtualGridCtrl::IsCellVisible(const CGridCell& cell, BOOL bPartialOK)
{
	BOOL bVisible;
	CRect cellRect;
	CRect gridRect;
	CRect rect;

	cellRect = GetCellRect(cell);
	gridRect = GetGridRect();

	if (m_pFixed->GetCount() && cell.m_iColumn >= m_pFixed->GetCount())
		gridRect.left = GetFixedRect().right;

	//  Intersection
	bVisible = rect.IntersectRect(cellRect, gridRect);

	if (!bPartialOK)
		bVisible = (rect == cellRect);

	return bVisible;
}



//
BOOL CVirtualGridCtrl::IsColumnVisible(int iColumn)
{
	CRect columnRect;
	CRect gridRect;

	columnRect = GetColumnRect(iColumn);
	gridRect = GetGridRect();
	return columnRect.IntersectRect(columnRect, gridRect);
}



//
BOOL CVirtualGridCtrl::IsFocusAllowed()
{
	return (m_bRowSelect || (!(m_bEditing || m_bAlwaysEdit))) && m_bAllowSelect;
}



//
BOOL CVirtualGridCtrl::IsHeaderHasImage(CGridHeaderSection* pSection)
{
	if (m_pImageList == 0)
		return FALSE;

	if (GetHeaderImage(pSection) < 0)
		return FALSE;

	return TRUE;
}



//
BOOL CVirtualGridCtrl::IsHeaderPressed(CGridHeaderSection* pSection)
{
	return ((pSection == 0) || (pSection == m_pHeaderClickSection)) && m_bHeaderClickState;
}


//
BOOL CVirtualGridCtrl::IsRowVisible(int nRow)
{
	CRect rowRect;
	CRect gridRect;

	rowRect = GetRowRect(nRow);
	gridRect = GetGridRect();

	return rowRect.IntersectRect(rowRect, gridRect);
}




//
void CVirtualGridCtrl::MakeCellVisible(const CGridCell& cell, BOOL bPartialOK)
{
	int dx, dy, x, y;
	CRect gridRect, rect;

	LRESULT lResult;

	if (IsCellVisible(cell, bPartialOK))
		return;		//  Already done

	dx = 0;
	dy = 0;
	gridRect = GetGridRect();

	//  -----------------  Horizontal scroll  --------------------
	if (!m_bRowSelect)  {
		rect = GetColumnRect(cell.m_iColumn);
		x = gridRect.left + GetFixedWidth();

		if (rect.right > gridRect.right)
			dx = gridRect.right - rect.right;
		if (rect.left < x)
			dx = x - rect.left;
		if (rect.Width() > gridRect.right - x)
			dx = x - rect.left;
	}

	if (dx < 0)  {
		while (dx)  {
			lResult = SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
			++dx;
		}
	}  else if (dx > 0)  {
		while (dx)  {
			lResult = SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
			--dx;
		}
	}

	//  --------------  Vertical scroll  ---------------
	if (m_pRows->m_nHeight > 0)  {
		rect = GetRowRect(cell.m_iRow);
		if (rect.bottom > gridRect.bottom)
			dy = gridRect.bottom - rect.bottom;
		if (rect.top < gridRect.top)
			dy = gridRect.top - rect.top;
		if (rect.Height() > gridRect.Height())
			dy = gridRect.top - rect.top;
		y = dy / m_pRows->m_nHeight;

		if (m_visSize.m_iRow > 1 && fmod((double)dy, (double)m_pRows->m_nHeight) != 0.0)
			--y;
		dy = y;
	}

	if (dy < 0)  {
		while (dy)  {
			lResult = SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			++dy;
		}
	}  else if (dy > 0)  {
		while (dy)  {
			lResult = SendMessage(WM_VSCROLL, SB_LINEUP, 0);
			--dy;
		}
	}
}



//
void CVirtualGridCtrl::SetCursor(CGridCell& cell, BOOL bSelected, BOOL bVisible)
{
	//  Check selection...
	UpdateSelection(cell, bSelected);

	//  Is there something changed?
	if (m_cellFocused != cell || m_bCellSelected != bSelected)  {
		//  The cell is changing...
		Changing(cell, bSelected);

		//  Set active cell
		if (m_cellFocused != cell)  {
			//  Application.CancelHint

			SetEditing(FALSE);

			//  Change cell
			HideCursor();

			m_cellFocused = cell;
			m_bCellSelected = bSelected;
			if (bVisible)
				MakeCellVisible(cell, FALSE);
			ShowCursor();
		}  else if (m_bCellSelected != bSelected) {
			if (m_bEditing)
				ShowEdit();

			if (!m_bEditing)  {
				HideCursor();
				m_bCellSelected = bSelected;
				if (bVisible)
					MakeCellVisible(cell, FALSE);
				ShowCursor();
			}
		}

		Change(m_cellFocused, m_bCellSelected);
	}  else  {
		if (bVisible)
			MakeCellVisible(cell, FALSE);
	}
}


//
void CVirtualGridCtrl::UndoEdit()
{
	if (m_pEdit && EditCanUndo(m_editCell))
		m_pEdit->Undo();
}



//
void CVirtualGridCtrl::UpdateCursor()
{
	CGridCell cell;
	BOOL bIsValidCell, bDummy;

	cell = m_cellFocused;

	bIsValidCell = IsCellValid(cell) && IsCellAcceptCursor(cell);

	//  If current cell is not acceptable then look for accessible cell around.
	if (!bIsValidCell)  {
		UpdateSelection(cell, bDummy);
		if (cell == m_cellFocused)
			cell = GetCursorCell(cell, goFirst);
	}

	//  Adjust cell selection
	SetCursor(cell, m_bCellSelected, !bIsValidCell);
}


//
void CVirtualGridCtrl::UpdateColors()
{
	CDC* pDC = GetDC();
	int nRetVal;

	m_pHeader->GridColorChanged(pDC->GetBkColor());
	m_pFixed->GridColorChanged(pDC->GetBkColor());

	nRetVal = ReleaseDC(pDC);
	ASSERT(nRetVal);
}



//
void CVirtualGridCtrl::UpdateEdit(BOOL bActivate)
{
	if (EditCanShow(m_cellFocused))  {
		if (m_pEdit == 0)  {
			ValidateEdit();
			DoUpdateEdit();
		}  else if (!(m_editCell == m_cellFocused && m_bEditing))  {
			bActivate = bActivate || m_bEditing || m_bAlwaysEdit;
			HideEdit();
			ValidateEdit();
			DoUpdateEdit();
		}

		if (bActivate)
			m_pEdit->Show();
	}  else  {
		HideEdit();
	}
}



//
void CVirtualGridCtrl::UpdateEditContents(BOOL bSaveText)
{
	CString strEditText;

	if (!m_bEditing)
		return;		//  Nothing to update

	m_pEdit->GetWindowText(strEditText);

	//  To completely update the edit control, we should switch it off
	m_pEdit->ShowWindow(SW_HIDE);

	//  Update and show again
	UpdateEdit();

	//  Recover the text
	if (bSaveText)
		m_pEdit->SetWindowText(strEditText);
}



//
void CVirtualGridCtrl::UpdateEditText()
{
	BOOL bEditFocused;
	CString strEditText;

	if (m_bReadOnly || m_pEdit == 0 || IsCellReadOnly(m_editCell))
		return;		//  Nothing to update

	bEditFocused = GetEditing();

	try {
		m_pEdit->GetWindowText(strEditText);
		SetEditText(m_editCell, strEditText);

		m_pEdit->SetWindowText(strEditText);
	}
	catch (...) {
		//  Don't let the scroller move
		MakeCellVisible(m_cellFocused, FALSE);

		//  If the edit line is visible then we should set the focus on it
		//  otherwise it would be switched off after error dialog box appearing
		if (bEditFocused)
			m_pEdit->SetFocus();
	}
}


//
void CVirtualGridCtrl::UpdateFixed()
{
	m_pFixed->SetCount(m_pFixed->m_nCount);
}



//
void CVirtualGridCtrl::UpdateFocus()
{
	//  ????????
	//  If the grid is already active then set the focus once more to
	//  avoid problems with MDI forms
	if (IsActiveControl())  {
		SetFocus();
	}  else  {
		//  Can we set the focus at all?
		if (IsWindowVisible())  {	//   && m_bTabStop  ?????
			SetFocus();
			if (m_bAlwaysEdit && m_pEdit)
				UpdateEdit();
		}
	}
}



//
void CVirtualGridCtrl::UpdateFonts()
{
	m_pHeader->GridFontChanged(&m_font);
	m_pFixed->GridFontChanged(&m_font);
}



//
void CVirtualGridCtrl::UpdateHeader()
{
	if (m_pHeader->m_bAutoSynchronize || m_pHeader->m_bSynchronized)
		m_pHeader->SynchronizeSections();
	else
		m_pHeader->UpdateSections();
}



//
void CVirtualGridCtrl::UpdateRows()
{
	m_pRows->SetHeight(m_pRows->m_nHeight);
}




//
void CVirtualGridCtrl::UpdateScrollBars()
{
	CRect rcGrid;
	int nMax, nPageStep, nLineStep;

	SetRedraw(FALSE);

	rcGrid = GetGridRect();

	//  ------------  Vertical scrolling parameters...  ---------------
	if (m_pRows->m_nCount > 0 && m_pRows->m_nHeight > 0)  {
		nMax = m_pRows->m_nCount - 1;
		nPageStep = rcGrid.Height() / m_pRows->m_nHeight - 1;
		nLineStep = 1;
	}  else  {
		nMax = 0;
		nPageStep = 0;
		nLineStep = 0;
	}

	m_pVertScrollBar->SetLineSize(m_pRows->GetHeight());
	m_pVertScrollBar->SetParams(0, nMax, nPageStep, nLineStep);


	//  --------  Horizontal scrolling parameters...  -------
	if (m_pColumns->GetSize())  {
		nMax = GetColumnsWidth(0, m_pColumns->GetUpperBound()) - GetFixedWidth();
		nPageStep = rcGrid.Width() - GetFixedWidth();
		nLineStep = 8;
	}  else  {
		nMax = 0;
		nPageStep = 0;
		nLineStep = 0;
	}

	m_pHorzScrollBar->SetLineSize(1);
	m_pHorzScrollBar->SetParams(0, nMax, nPageStep, nLineStep);

	SetRedraw();
}




//
void CVirtualGridCtrl::UpdateScrollPos()
{
	m_pVertScrollBar->SetPosition(m_visOrigin.m_iRow);
	m_pHorzScrollBar->SetPosition(GetColumnsWidth(m_pFixed->GetCount(), m_visOrigin.m_iColumn - 1));
}




//
void CVirtualGridCtrl::UpdateSelection(CGridCell& cell, BOOL& bSelected)
{
	//  Check the selection flag
	bSelected = bSelected || m_bAlwaysSelected;
	bSelected = bSelected && m_pRows->m_nCount > 0 && m_pColumns->GetSize();

	//  Check cell borders
	if (cell.m_iColumn < m_pFixed->GetCount())
		cell.m_iColumn = m_pFixed->GetCount();
	if (cell.m_iColumn < 0)
		cell.m_iColumn = 0;
	if (cell.m_iColumn > m_pColumns->GetUpperBound())
		cell.m_iColumn = m_pColumns->GetUpperBound();

	if (cell.m_iRow < 0)
		cell.m_iRow = 0;
	if (cell.m_iRow > m_pRows->GetCount() - 1)
		cell.m_iRow = m_pRows->GetCount() - 1;

	//  Check the focus...
	cell = GetCursorCell(cell, goSelect);
}



//
void CVirtualGridCtrl::UpdateText()
{
	UpdateEditText();
}



//
void CVirtualGridCtrl::UpdateVisOriginSize()
{
	CRect rcClient;
	CRect r;
	int i;
	int x, nHeight;

	GetClientRect(rcClient);

	//  -------------------  Columns part  ---------------------------
	if (m_pColumns->GetSize())  {
		//  Look for the first non-fixed column
		x = rcClient.left + GetFixedWidth() - m_pHorzScrollBar->GetPosition();
		r = GetFixedRect();
		i = m_pFixed->GetCount();

		while (i < m_pColumns->GetUpperBound())  {
			x += m_pColumns->GetAt(i)->GetWidth();

			if (x >= r.right)
				break;

			++i;
		}

		m_visOrigin.m_iColumn = i;

		//  Calculate amount of visible columns
		r = GetGridRect();

		while (i < m_pColumns->GetUpperBound())  {
			if (x >= r.right)
				break;
			++i;
			x += m_pColumns->GetAt(i)->GetWidth();
		}

		m_visSize.m_iColumn = i - m_visOrigin.m_iColumn + 1;
	}  else  {
		m_visOrigin.m_iColumn = 0;
		m_visSize.m_iColumn = 0;
	}

	//  -----------------------  Rows part  ---------------------------
	if (m_pRows->GetCount() && m_pRows->m_nHeight > 0)  {
		div_t myDiv;

		m_visOrigin.m_iRow = m_pVertScrollBar->GetPosition();

		//  Calculate amount of visible (even partially) rows
		nHeight = GetGridHeight();
		myDiv = div(nHeight, m_pRows->m_nHeight);

		m_visSize.m_iRow = myDiv.quot + (myDiv.rem > 0);

		if (m_visSize.m_iRow + m_visOrigin.m_iRow > m_pRows->GetCount())
			m_visSize.m_iRow = m_pRows->m_nCount - m_visOrigin.m_iRow;
	}  else  {
		m_visOrigin.m_iRow = 0;
		m_visSize.m_iRow = 0;
	}
}



//
CString CVirtualGridCtrl::GetCell(int iColumn, int nRow)
{
	CGridCell cell(iColumn, nRow);
	TCHAR szBuffer[MAX_CELL_TEXT];
	int nAlign;

	GetCellText(szBuffer, cell, nAlign);
	return CString(szBuffer);
}



//
BOOL CVirtualGridCtrl::GetChecked(int iColumn, int nRow)
{
	int nState;

	nState = GetCheckState(CGridCell(iColumn, nRow));

	return (nState == 1 || nState == 2);	//  ????
}



//
int CVirtualGridCtrl::GetCol()
{
	return m_cellFocused.m_iColumn;
}



//
CGridColumn* CVirtualGridCtrl::GetEditColumn()
{
	if (m_editCell.m_iColumn < 0 || m_editCell.m_iColumn > m_pColumns->GetUpperBound())
		return 0;

	return (*m_pColumns)[m_editCell.m_iColumn];
}



//
BOOL CVirtualGridCtrl::GetEditDropDown()
{
	return m_pEdit && m_pEdit->m_bDropListVisible;
}


//
BOOL CVirtualGridCtrl::GetEditing()
{
	return m_bEditing && m_pEdit;
}


//
CGridHeader* CVirtualGridCtrl::GetHeader()
{
	return m_pHeader;
}



//
int CVirtualGridCtrl::GetLeftCol()
{
	return m_visOrigin.m_iColumn;
}



//
int CVirtualGridCtrl::GetRow()
{
	return m_cellFocused.m_iRow;
}


//
CGridRows* CVirtualGridCtrl::GetRows()
{
	return m_pRows;
}



//
int CVirtualGridCtrl::GetTopRow()
{
	return m_visOrigin.m_iRow;
}


//
int CVirtualGridCtrl::GetVisibleColCount()
{
	return m_visSize.m_iColumn;
}


//
int CVirtualGridCtrl::GetVisibleRowCount()
{
	return m_visSize.m_iRow;
}



//
void CVirtualGridCtrl::ColumnsChange()
{
	UpdateFixed();
	UpdateHeader();

	if (!m_pHeader->m_bAutoSynchronize)
		m_pHeader->SetSynchronized(FALSE);

	UpdateScrollBars();
	UpdateVisOriginSize();
	UpdateCursor();
	UpdateEdit(m_bEditing);
	Invalidate();
	ChangeColumns();
}



//
void CVirtualGridCtrl::FixedChange()
{
	UpdateRows();
	UpdateScrollBars();
	UpdateVisOriginSize();
	UpdateCursor();
	UpdateEdit(m_bEditing);
	Invalidate();
	ChangeFixed();
}



//
void CVirtualGridCtrl::HeaderChange()
{
	UpdateScrollBars();
	UpdateVisOriginSize();
	UpdateEdit(m_bEditing);
	Invalidate();
}



//
void CVirtualGridCtrl::HorzScrollChange()
{
	UpdateVisOriginSize();
	UpdateEdit(m_bEditing);
}


//
void CVirtualGridCtrl::ImagesChange()
{
	InvalidateGrid();
	UpdateRows();
}



//
void CVirtualGridCtrl::RowsChange()
{
	UpdateScrollBars();
	UpdateVisOriginSize();
	UpdateCursor();
	UpdateEdit(m_bEditing);
	Invalidate();
	ChangeRows();
}



//
void CVirtualGridCtrl::SetAllowEdit(BOOL bAllowEdit)
{
	if (m_bAllowEdit == bAllowEdit)
		return;		//  Nothing to change

	m_bAllowEdit = bAllowEdit;
	if (m_bAllowEdit)  {
		SetRowSelect(FALSE);
	}  else  {
		m_bAlwaysEdit = FALSE;
		HideEdit();
	}

	//  Event
	ChangeEditMode();
}



//
void CVirtualGridCtrl::SetAllowSelect(BOOL bAllowSelect)
{
	if (m_bAllowSelect == bAllowSelect)
		return;		//  Nothing to change

	m_bAllowSelect = bAllowSelect;
	SetRowSelect(m_bRowSelect || !m_bAllowSelect);
	InvalidateFocus();
}



//
void CVirtualGridCtrl::SetAlwaysEdit(BOOL bAlwaysEdit)
{
	if (m_bAlwaysEdit == bAlwaysEdit)
		return;		//  Nothing to change
	
	m_bAlwaysEdit = bAlwaysEdit;
	if (m_bAlwaysEdit)  {
		SetAllowEdit();
		SetEditing();
	}  else  {
		HideEdit();
	}
}



//
void CVirtualGridCtrl::SetAlwaysSelected(BOOL bAlwaysSelected)
{
	if (m_bAlwaysSelected == bAlwaysSelected)
		return;		//  Nothing to change
	
	m_bAlwaysSelected = bAlwaysSelected;
	m_bCellSelected = m_bAlwaysSelected || m_bAlwaysSelected;
	InvalidateFocus();
}



//
void CVirtualGridCtrl::SetCell(int iColumn, int nRow, CString& strText)
{
	SetEditText(CGridCell(iColumn, nRow), strText);
}



//
void CVirtualGridCtrl::SetCellFocused(CGridCell& cell)
{
	SetCursor(cell, m_bCellSelected, TRUE);
}




void CVirtualGridCtrl::SetCellFocused(int iColumn, int iRow)
{
	CGridCell curCell(iColumn, iRow);
	SetCursor(curCell, m_bCellSelected, TRUE);
}



//
void CVirtualGridCtrl::SetCellSelected(BOOL bCellSelected)
{
	SetCursor(m_cellFocused, bCellSelected, TRUE);
}



//
void CVirtualGridCtrl::SetCheckBoxes(BOOL bCheckBoxes)
{
	if (m_bCheckBoxes == bCheckBoxes)
		return;		//  Nothing to change

	m_bCheckBoxes = bCheckBoxes;
	UpdateRows();
	UpdateEdit(m_bEditing);
	InvalidateGrid();
}


//
void CVirtualGridCtrl::SetCheckLeftIndent(int nLeftIndent)
{

}



//
void CVirtualGridCtrl::SetCheckStyle(GridCheckStyle checkStyle)
{
	if (m_checkStyle == checkStyle)
		return;		//  Nothing to change

	m_checkStyle = checkStyle;

	if (m_bCheckBoxes)
		InvalidateGrid();
}



//
void CVirtualGridCtrl::SetCheckTopIndent(int nTopIndent)
{

}


//
void CVirtualGridCtrl::SetCol(int nCol)
{
	SetCellFocused(CGridCell(nCol, m_cellFocused.m_iRow));
}


//
void CVirtualGridCtrl::SetColumns(const CGridColumns& columns)
{
	int i;
	CGridColumn* pColumn;

	for (i = 0; i < m_pColumns->m_columns.GetSize(); ++i)
		delete m_pColumns->GetAt(i);

	m_pColumns->m_columns.RemoveAll();

	for (i = 0; i < columns.m_columns.GetSize(); ++i) {
		pColumn = new CGridColumn(*columns.m_columns[i]);
		pColumn->m_pColumns = &(*m_pColumns);
		m_pColumns->m_columns.Add(pColumn);
	}

	ColumnsChange();
}



//
void CVirtualGridCtrl::SetCursorKeys(UINT nCursorKeys)
{
	//  Check incompatible flags
	if (gkMouseMove & nCursorKeys)
		nCursorKeys = nCursorKeys | gkMouse;
	if (!(gkMouse & nCursorKeys))
		nCursorKeys = nCursorKeys & ~gkMouseMove;

	m_nCursorKeys = nCursorKeys;
}



//
void CVirtualGridCtrl::SetEditDropDown(BOOL bEditDropDown)
{
	//  Set cell into editing mode
	SetEditing();

	//  Show drop down list
	if (m_pEdit)
		m_pEdit->SetDropListVisible(TRUE);
}


//
void CVirtualGridCtrl::SetEditing(BOOL bEditing)
{
	m_titleTip.Hide();

	if (bEditing && m_bAllowEdit)  {
		if (AcquireFocus())
			ShowEdit();
	}  else if (!bEditing && m_bEditing)  {
		UpdateEditText();
		if (!m_bAlwaysEdit)
			HideEdit();
	}

	//  An event...
	ChangeEditing();
}



//
void CVirtualGridCtrl::SetEndEllipsis(BOOL bEndEllipsis)
{
	if (m_bEndEllipsis == bEndEllipsis)
		return;		//  Nothing to change

	m_bEndEllipsis = bEndEllipsis;
	InvalidateGrid();
}




void CVirtualGridCtrl::SetFixed(CGridFixed* pFixed)
{

}



void CVirtualGridCtrl::SetGridColor(COLORREF gridColor)
{

}



//
void CVirtualGridCtrl::SetGridLines(BOOL bGridLines)
{
	if (m_bGridLines == bGridLines)
		return;		//  Nothing to set

	m_bGridLines = bGridLines;
	UpdateRows();
	UpdateEdit(m_bEditing);
	Invalidate();
}




//
void CVirtualGridCtrl::SetGridStyle(UINT nGridStyles)
{
	if (m_nGridStyles == nGridStyles)
		return;		//  Nothing to set

	m_nGridStyles = nGridStyles;
	UpdateRows();
	UpdateEdit(m_bEditing);
	Invalidate();
}


//
void CVirtualGridCtrl::SetHeader(CGridHeader* pHeader)
{

}



void CVirtualGridCtrl::SetHideSelection(BOOL bHideSelection)
{

}

void CVirtualGridCtrl::SetImageIndexDef(int nIndex)
{

}

void CVirtualGridCtrl::SetImageHighlight(BOOL bImageHighlight)
{

}


void CVirtualGridCtrl::SetImageLeftIndent(int nLeftIndent)
{

}


//
void CVirtualGridCtrl::SetImageList(CImageList* pImageList)
{
	m_pImageList = pImageList;
}


//
void CVirtualGridCtrl::SetImageTopIndent(int nTopIndent)
{
}



//
void CVirtualGridCtrl::SetLeftCol(int nLeftCol)
{
	SetVisOrigin(CGridCell(nLeftCol, m_visOrigin.m_iRow));
}



//
void CVirtualGridCtrl::SetReadOnly(BOOL bReadOnly)
{
	if (m_bReadOnly == bReadOnly)
		return;		//  Nothing to change

	m_bReadOnly = bReadOnly;
	UpdateEditContents();
}



void CVirtualGridCtrl::SetRow(int nRow)
{
}

void CVirtualGridCtrl::SetRows(CGridRows* pRows)
{

}



//
void CVirtualGridCtrl::SetRowSelect(BOOL bRowSelect)
{
	if (m_bRowSelect == bRowSelect)
		return;		//  Nothing to change

	m_bRowSelect = bRowSelect;
	SetAllowEdit(!bRowSelect);

	SetAllowSelect(GetAllowSelect() || !bRowSelect);
	InvalidateGrid();
}

void CVirtualGridCtrl::SetShowCellTips(BOOL bShowCellTips)
{

}

void CVirtualGridCtrl::SetShowFocusRect(BOOL bShowFocusRect)
{

}

void CVirtualGridCtrl::SetShowHeader(BOOL bShowHeader)
{

}

void CVirtualGridCtrl::SetSortLeftIndent(int nLeftIndent)
{

}


void CVirtualGridCtrl::SetSortTopIndent(int nTopIndent)
{

}

void CVirtualGridCtrl::SetTextLeftIndent(int nLeftIndent)
{

}

void CVirtualGridCtrl::SetTextRightIndent(int nRightIndent)
{

}

void CVirtualGridCtrl::SetTextTopIndent(int nTopIndent)
{

}



//
void CVirtualGridCtrl::SetTopRow(int nTopRow)
{
	SetVisOrigin(CGridCell(m_visOrigin.m_iColumn, nTopRow));
}




//
void CVirtualGridCtrl::SetVisOrigin(const CGridCell& cell)
{
	if (m_visOrigin.m_iColumn != cell.m_iColumn || m_visOrigin.m_iRow != cell.m_iRow)  {
		m_visOrigin = cell;

		//  Update scroll bars...
		UpdateScrollPos();
		UpdateVisOriginSize();

		//  Repaint...
		Invalidate();
	}
}



//
void CVirtualGridCtrl::VertScrollChange()
{
	UpdateVisOriginSize();
	UpdateEdit(m_bEditing);
}



//
BOOL CVirtualGridCtrl::AcquireFocus()
{
	UpdateFocus();

	return IsActiveControl();
}



//
void CVirtualGridCtrl::CellClick(const CGridCell& cell, UINT nFlags, int x, int y)
{
}


//
void CVirtualGridCtrl::CellTips(const CGridCell& cell, BOOL& bAllowTips)
{
}



//
void CVirtualGridCtrl::Change(CGridCell& cell, BOOL& bSelected)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	if (!IsCellValid(cell))
		return;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_CHANGE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.bSelected = bSelected;
	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	bSelected = dispInfo.item.bSelected;
}



//
void CVirtualGridCtrl::ChangeColumns()
{
}


//
void CVirtualGridCtrl::ChangeEditing()
{
}


//
void CVirtualGridCtrl::ChangeEditMode()
{
}


//
void CVirtualGridCtrl::ChangeFixed()
{
}



//
void CVirtualGridCtrl::ChangeRows()
{
}



//  Selected cell is about to change.
void CVirtualGridCtrl::Changing(CGridCell& cell, BOOL& bSelected)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	if (!IsCellValid(cell))
		return;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_CHANGING;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.bSelected = bSelected;
	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	bSelected = dispInfo.item.bSelected;
}



//
void CVirtualGridCtrl::CheckClick(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_CHECKCLICK;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}



//
void CVirtualGridCtrl::ColumnAutoSize(int iColumn, int& nWidth)
{
	VG_DISPINFO dispInfo;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_COLUMNAUTOSIZE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = iColumn;
	dispInfo.item.nWidth = nWidth;

	pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	nWidth = dispInfo.item.nWidth;
}




//
void CVirtualGridCtrl::ColumnResize(int iColumn, int& nWidth)
{
	VG_DISPINFO dispInfo;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_COLUMNRESIZE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = iColumn;
	dispInfo.item.nWidth = nWidth;

	pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	nWidth = dispInfo.item.nWidth;
}




//
void CVirtualGridCtrl::ColumnResizing(int iColumn, int& nWidth)
{
	VG_DISPINFO dispInfo;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_COLUMNRESIZING;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = iColumn;
	dispInfo.item.nWidth = nWidth;

	pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	nWidth = dispInfo.item.nWidth;
}



//
CGridScrollBar* CVirtualGridCtrl::CreateScrollBar(int nBar)
{
	return new CGridScrollBar(this, nBar);
}


//
CGridHeader* CVirtualGridCtrl::CreateHeader()
{
	return new CGridHeader(this);
}


//
CGridColumns* CVirtualGridCtrl::CreateColumns()
{
	return new CGridColumns(this);
}


//
CGridRows* CVirtualGridCtrl::CreateRows()
{
	return new CGridRows(this);
}


//
CGridFixed* CVirtualGridCtrl::CreateFixed()
{
	return new CGridFixed(this);
}



//
CGridEdit* CVirtualGridCtrl::CreateEdit(CRuntimeClass* pEditClass)
{
	CGridEdit* pEdit;

	if (pEditClass == 0)
		pEditClass = RUNTIME_CLASS(CGridEdit);
	pEdit = (CGridEdit*)pEditClass->CreateObject();

	return pEdit;
}



//
void CVirtualGridCtrl::EditButtonPress(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_EDITBUTTONPRESS;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}




//
BOOL CVirtualGridCtrl::EditCanAcceptKey(const CGridCell& cell, UINT nChar)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_EDITACCEPTKEY;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.nChar = nChar;
	dispInfo.item.bAccept = IsCellValid(cell);
	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.bAccept;
}


//  --
void CVirtualGridCtrl::EditCanceled(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_EDITCANCELED;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}



//  --
BOOL CVirtualGridCtrl::EditCanModify(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_EDITCANMODIFY;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.bReadOnly = IsCellReadOnly(cell);

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return !dispInfo.item.bReadOnly;
}



//
BOOL CVirtualGridCtrl::EditCanShow(const CGridCell& cell)
{
	//  Is there any cells?
	if ((0 == m_pColumns->GetSize() - m_pFixed->m_nCount) || !m_pRows->GetCount())
		return FALSE;

	// 02.07.2018 NICK. добавил && !IsCellReadOnly(cell), чтоб не по€вл€лс€ эдитбокс в €чейках, доступных только дл€ чтени€
	return m_bAllowEdit && (m_bAlwaysEdit || IsActiveControl());// && !IsCellReadOnly(cell);
}



//
BOOL CVirtualGridCtrl::EditCanUndo(const CGridCell& cell)
{
	return EditCanModify(cell);
}



//
void CVirtualGridCtrl::EditChange(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_EDITCHANGE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}




//
void CVirtualGridCtrl::EditCloseUp(const CGridCell& cell, int iItem, BOOL& bAccept)
{
	VG_DISPINFO dispInfo;
	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_EDITCLOSEUP;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.iItem = iItem;
	dispInfo.item.bAccept = bAccept;

	pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	bAccept = dispInfo.item.bAccept;	
}




//
void CVirtualGridCtrl::GetCellColors(const CGridCell& cell, CDC* pDC)
{
	CWnd* pWnd;
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	//  --------  Fixed part  -----------
	if (cell.m_iColumn < m_pFixed->GetCount()) {
		pDC->SetBkColor(m_pFixed->m_color);
		pDC->SelectObject(m_pFixed->m_pFont);
	}  else  {
		//  ------  Regular cell  ----------
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
		pDC->SelectObject(&m_font);

		if (!IsWindowEnabled())
			pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));

		//  Selected cell
		if (IsWindowEnabled() && IsFocusAllowed() && m_bCellSelected && IsCellFocused(cell))  {
			//  Is the grid focused?
			if (this == GetFocus())  {
				pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
				pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			}  else if (!m_bHideSelection)  {
				pDC->SetBkColor(::GetSysColor(COLOR_BTNFACE));
			}
		}
	}

	if (IsCellFocused(cell))
		return;

	pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETDISPINFO;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.mask = LVIF_COLOR;
	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.hDC = pDC->m_hDC;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}




//
int CVirtualGridCtrl::GetCellImage(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETDISPINFO;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.mask = LVIF_IMAGE;
	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.iImage = -1;	//  Default: NO IMAGE
	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.iImage;
}



//  ----
CPoint CVirtualGridCtrl::GetCellImageIndent(const CGridCell& cell)
{
	CPoint point(m_nImageLeftIndent, m_nImageTopIndent);
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	//  Take into account 3D effect...
	if (m_pFixed->GetCount() > 0 && !m_pFixed->GetFlat())
		++point.y;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETCELLIMAGEINDENT;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.ptIndent = point;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return point;
}


//  -----
CRect CVirtualGridCtrl::GetCellImageRect(const CGridCell& cell)
{
	CRect rect(0, 0, 0, 0);
	IMAGEINFO ii;
	BOOL bSuccess;

	if (m_pImageList == 0)
		return rect;

	bSuccess = m_pImageList->GetImageInfo(0, &ii);
	ASSERT(bSuccess);

	//  Is there any pictures at all?
	if (!IsCellHasImage(cell))
		return rect;

	rect = GetCellRect(cell);

	//  Take into account the check
	if (IsCellHasCheck(cell))
		rect.left += (m_nCheckWidth + GetCheckIndent(cell).x);

	//  Image rectangle...
	rect.left += GetCellImageIndent(cell).x;
	rect.top += GetCellImageIndent(cell).y;
	rect.right = min(rect.left + (ii.rcImage.right - ii.rcImage.left), rect.right);
	rect.bottom = rect.top + (ii.rcImage.bottom - ii.rcImage.top);

	return rect;
}





//
void CVirtualGridCtrl::GetCellText(LPTSTR lpBuffer, const CGridCell& cell, int& nAlign)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETDISPINFO;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.mask = LVIF_TEXT;
	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.pszText = _T("");
	dispInfo.item.nAlign = m_pColumns->GetAt(cell.m_iColumn)->GetAlignment();

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	if (lResult)
		return;

	if (dispInfo.item.pszText && lstrlen(dispInfo.item.pszText))
		wcscpy(lpBuffer, dispInfo.item.pszText);
	else
		lpBuffer[0] = '\0';

	nAlign = dispInfo.item.nAlign;
}




//
CRect CVirtualGridCtrl::GetCellTextBounds(const CGridCell& cell)
{
	CDC* pDC;
	CPoint point;
	TCHAR szBuffer[MAX_CELL_TEXT];
	CGridColumn* pColumn;
	int nRetVal;
	int nAlign;

	//  Check cell's column
	if (cell.m_iColumn < 0 || cell.m_iColumn > m_pColumns->GetUpperBound())
		return CRect(0, 0, 0, 0);

	pDC = GetDC();
	//  Determine the colors...
	if (cell.m_iRow >= 0 && cell.m_iRow < m_pRows->GetCount()) {
		GetCellColors(cell, pDC);
		point = GetCellTextIndent(cell);
		GetCellText(szBuffer, cell, nAlign);
	}

	//  Paint parameters
	CRect rect(0, 0, 0, 0);

	pColumn = (*m_pColumns)[cell.m_iColumn];

	if (pColumn->GetWordWrap()) {
		rect = GetEditRect(cell);
		rect.OffsetRect(-rect.left, -rect.top);
		rect.bottom = rect.top;
	}

	//  Calculate text rectangle
	rect = GetTextRect(
		pDC,
		rect,
		point.x,
		point.y,
		pColumn->GetAlignment(),
		pColumn->m_bWantReturns,
		pColumn->m_bWordWrap,
		szBuffer
		);


	//  Set the upper-left corner to 0, 0
	rect.OffsetRect(-rect.left, -rect.top);

	nRetVal = ReleaseDC(pDC);
	ASSERT(nRetVal);

	return rect;
}



//
CPoint CVirtualGridCtrl::GetCellTextIndent(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETCELLTEXTINDENT;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	//  Default values
	dispInfo.item.ptIndent.x = m_nTextLeftIndent;
	dispInfo.item.ptIndent.y = m_nTextTopIndent;

	//  Taking into account images and 3D-effect
	if (IsCellHasCheck(cell) || IsCellHasImage(cell))  {
		dispInfo.item.ptIndent.x = 2;

		if (m_pFixed->GetCount() > 0 && !m_pFixed->m_bFlat)
			++dispInfo.item.ptIndent.y;
	}

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.ptIndent;
}





//
int CVirtualGridCtrl::GetCheckAlignment(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETCHECKALIGNMENT;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.nAlign = LVCFMT_LEFT;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.nAlign;
}




//
void CVirtualGridCtrl::GetCheckImage(const CGridCell& cell, HBITMAP* pCheckImage)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETCHECKIMAGE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.hBitmap = *pCheckImage;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	*pCheckImage = dispInfo.item.hBitmap;
}



//
CPoint CVirtualGridCtrl::GetCheckIndent(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETCHECKINDENT;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.ptIndent.x = m_nCheckLeftIndent;
	dispInfo.item.ptIndent.y = m_nCheckTopIndent;

	//  Taking into account 3D effect...
	if (m_pFixed->GetCount() > 0 && !m_pFixed->GetFlat())
		++dispInfo.item.ptIndent.y;

	//  Taking into account check alignment...
	if (LVCFMT_CENTER == GetCheckAlignment(cell))
		dispInfo.item.ptIndent.x = ((*m_pColumns)[cell.m_iColumn]->GetWidth() - m_nCheckWidth) / 2 - 1;

	//  Send notification.
	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.ptIndent;
}


//  --
GridCheckKind CVirtualGridCtrl::GetCheckKind(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	if (!m_bCheckBoxes || (cell.m_iColumn < 0) || (cell.m_iColumn > m_pColumns->GetUpperBound()))
		return gcNone;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_GETCHECKKIND;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.checkKind = (*m_pColumns)[cell.m_iColumn]->m_checkKind;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.checkKind;
}



//  ---
CRect CVirtualGridCtrl::GetCheckRect(const CGridCell& cell)
{
	CRect rect;

	//  Is there check box?
	if (!IsCellHasCheck(cell))
		return CRect(0, 0, 0, 0);

	rect = GetCellRect(cell);

	//  Check box rectangle...
	rect.left += GetCheckIndent(cell).x;
	rect.right = min(rect.left + m_nCheckWidth, rect.right);
	rect.top += GetCheckIndent(cell).y;
	rect.bottom = rect.top + m_nCheckHeight;

	return rect;
}


//  --
int CVirtualGridCtrl::GetCheckState(const CGridCell& cell)
{
	LRESULT lResult;
	VG_DISPINFO dispInfo;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_GETCHECKSTATE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.nCheckState = 0;	//  Unchecked is default
	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.nCheckState;
}


//  --
CPoint CVirtualGridCtrl::GetClientOrigin()
{
	CRect rect;

	GetClientRect(rect);
	return rect.TopLeft();
}



//  --
CRuntimeClass* CVirtualGridCtrl::GetColumnClass()
{
	return RUNTIME_CLASS(CGridColumn);
}


//  --
CGridCell CVirtualGridCtrl::GetCursorCell(CGridCell& cell, GridCursorOffset offset)
{
	switch (offset)  {
	case goLeft:
		return MoveLeft(cell, 1);
	case goRight:
		return MoveRight(cell, 1);
	case goUp:
		return MoveUp(cell, 1);
	case goDown:
		return MoveDown(cell, 1);
	case goPageUp:
		return MoveUp(cell, m_visSize.m_iRow - 1);
	case goPageDown:
		return MoveDown(cell, m_visSize.m_iRow - 1);
	case goHome:
		return MoveHome(cell);
	case goEnd:
		return MoveEnd(cell);
	case goGridHome:
		return MoveGridHome(cell);
	case goGridEnd:
		return MoveGridEnd(cell);
	case goSelect:
		return Select(cell);
	case goFirst:
		return First();
	case goNext:
		return Next(cell);
	case goPrev:
		return Prev(cell);
	default:
		break;
	}

	return cell;
}



//  --
CRuntimeClass* CVirtualGridCtrl::GetEditClass(const CGridCell& cell)
{
	return RUNTIME_CLASS(CGridEdit);
}



//  --
void CVirtualGridCtrl::GetEditList(const CGridCell& cell, CStringArray& items)
{
	LRESULT lResult;
	VG_DISPINFO dispInfo;
	CGridColumn* pColumn;
	INT_PTR nIndex;

	pColumn = m_pColumns->GetAt(cell.m_iColumn);

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETEDITLIST;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.pickList.RemoveAll();	//  !!!
	dispInfo.item.pickList.Append(pColumn->m_pickList);

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	items.RemoveAll();	//  Just reinsurance...
	nIndex = items.Append(dispInfo.item.pickList);
}




//  --
void CVirtualGridCtrl::GetEditListBounds(const CGridCell& cell, LPRECT lpRect)
{
	//  NYI
}



//  --
GridEditStyle CVirtualGridCtrl::GetEditStyle(const CGridCell& cell)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	if (cell.m_iColumn < 0 || cell.m_iColumn > m_pColumns->GetUpperBound())
		return geSimple;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_GETEDITSTYLE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	dispInfo.item.style = (*m_pColumns)[cell.m_iColumn]->GetEditStyle();

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.style;
}


//  --
CString CVirtualGridCtrl::GetEditText(const CGridCell& cell)
{
	TCHAR szBuffer[MAX_CELL_TEXT];
	int nAlign;

	GetCellText(szBuffer, cell, nAlign);

	//  NYI
	return CString(szBuffer);
}




//
int CVirtualGridCtrl::GetHeaderImage(CGridHeaderSection* pSection)
{
	LRESULT lResult;
	VG_DISPINFO dispInfo;

	if (m_pHeader->m_pImageList == 0)
		return -1;

	if (!m_pHeader->m_pImageList->GetImageCount())
		return -1;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_GETHEADERIMAGE;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.pSection = pSection;
	//  Default header image is its column index by default
	dispInfo.item.iImage = pSection->GetColumnIndex();

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	//  Just reinsurance...
	if (dispInfo.item.iImage >= m_pHeader->m_pImageList->GetImageCount())
		dispInfo.item.iImage = -1;

	return dispInfo.item.iImage;
}


//  --
void CVirtualGridCtrl::GetHeaderColors(CGridHeaderSection* pSection, CDC* pDC)
{
	CFont* pOldFont;
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	//  Setting standard header color and font.
	pDC->SetBkColor(m_pHeader->m_color);
	pOldFont = pDC->SelectObject(m_pHeader->m_pFont);

	dispInfo.hdr.code = VGN_GETHEADERCOLORS;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.pSection = pSection;
	dispInfo.item.hDC = pDC->m_hDC;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}



//  --
GridSortDirection CVirtualGridCtrl::GetSortDirection(CGridHeaderSection* pSection)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd->GetSafeHwnd());

	dispInfo.hdr.code = VGN_GETSORTDIRECTION;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.pSection = pSection;
	dispInfo.item.sortDirection = gsNone;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	return dispInfo.item.sortDirection;
}




//
CRect CVirtualGridCtrl::GetTextRect(
				    CDC* pDC,
				    LPRECT lpRect,
				    int nLeftIndent,
				    int nTopIndent,
				    int nAlignment,
				    BOOL bWantReturns,
				    BOOL bWordWrap,
				    LPTSTR lpText
				    )
{
	CRect rect(*lpRect);
	DRAWTEXTPARAMS dtp;
	UINT nFlags;
	int nRetVal;
	int nWidth, nHeight;

	//  Let's check how to paint text: using DrawTextEx or TextOut
	if (bWantReturns || bWordWrap || m_bEndEllipsis)  {
		dtp.cbSize = sizeof(DRAWTEXTPARAMS);
		dtp.iLeftMargin = nLeftIndent;
		dtp.iRightMargin = m_nTextRightIndent;

		//  Text attributes
		nFlags = DT_NOPREFIX;

		//  Horizontal alignment...
		switch (nAlignment)  {
		case LVCFMT_LEFT:
			nFlags |= DT_LEFT;
			break;
		case LVCFMT_RIGHT:
			nFlags |= DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nFlags |= DT_CENTER;
			break;
		default:
			break;
		}

		//  Vertical alignment...
		if (!(bWantReturns || bWordWrap))
			//  Automatic alignment
			nFlags |= (DT_SINGLELINE | DT_VCENTER);

		//  Word wrap...
		if (bWordWrap)
			nFlags |= DT_WORDBREAK;

		//  Text rectangle
		rect = *lpRect;

		//  Calculate text rectangle
		nRetVal = DrawTextEx(pDC->m_hDC, lpText, lstrlen(lpText), rect, nFlags | DT_CALCRECT, &dtp);

		nWidth = max(rect.Width(), lpRect->right - lpRect->left);
		nHeight = max(rect.Height(), lpRect->bottom - lpRect->top);
	}  else  {
		CSize mySize;

		mySize = pDC->GetTextExtent(lpText);
		nWidth = max(rect.Width(), nLeftIndent + mySize.cx + m_nTextRightIndent);
		nHeight = max(rect.Height(), mySize.cy);
	}

	//  Mould the rectangle
	switch (nAlignment)  {
	case LVCFMT_CENTER:
		rect.left = lpRect->left - (nWidth - (lpRect->right - lpRect->left)) / 2;
		rect.right = rect.left + nWidth;
		break;
	case LVCFMT_RIGHT:
		rect.right = lpRect->right;
		rect.left = rect.right - nWidth;
		break;
	default:
		rect.left = lpRect->left;
		rect.right = rect.left + nWidth;
		break;
	}

	rect.top = lpRect->top;
	rect.bottom = rect.top + nHeight;

	return rect;
}



//  ---------  Header click handler  ---------
void CVirtualGridCtrl::HeaderClick(CGridHeaderSection* pSection)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_HEADERCLICK;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.pSection = pSection;
	
	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
}


//  ---
void CVirtualGridCtrl::HeaderClicking(CGridHeaderSection* pSection, BOOL& bAllowClick)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	//  By default only bottom-level sections is allowed to click
	bAllowClick = m_bColumnClick && pSection->GetAllowClick()
		&& !pSection->GetSections()->GetSize();

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = VGN_HEADERCLICKING;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.pSection = pSection;
	dispInfo.item.bAccept = bAllowClick;	//  !!!!

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	bAllowClick = dispInfo.item.bAccept;
}


//  ---
void CVirtualGridCtrl::HideCursor()
{
	if (IsFocusAllowed())
		InvalidateFocus();
	else
		HideEdit();
}


//  --
void CVirtualGridCtrl::HideFocus()
{
	CDC* pDC;
	int nRetVal;

	pDC = GetDC();

	if (IsFocusAllowed())
		PaintFocus(pDC);

	nRetVal = ReleaseDC(pDC);
	ASSERT(nRetVal);
}



//  ---
void CVirtualGridCtrl::PaintCell(CDC* pDC, const CGridCell& cell, LPRECT lpRect)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;

	GetCellColors(cell, pDC);

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_DRAWCELL;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.bDefDraw = TRUE;	//  !!!!  DEFAULT  !!!!!
	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;

	dispInfo.item.rect.left = lpRect->left;
	dispInfo.item.rect.top = lpRect->top;
	dispInfo.item.rect.right = lpRect->right;
	dispInfo.item.rect.bottom = lpRect->bottom;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);

	if (dispInfo.item.bDefDraw)
		DefaultDrawCell(pDC, cell, lpRect);

	//  !!!!!!!!  Restore  !!!!!!!!!!!
	pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
}




//  ---
void CVirtualGridCtrl::PaintDotGridLines(CDC* pDC, LPPOINT lpPoints, int nCount)
{
	int i = 0;
	CRect rect;

	PreparePatternBitmap(pDC, ::GetSysColor(COLOR_ACTIVEBORDER), FALSE);

	__try {
		//  -----------  Paint the lines  -----------
		while (i < nCount) {
			//  Line position
			rect.left = lpPoints[i].x;
			rect.top = lpPoints[i].y;
			++i;
			rect.right = lpPoints[i].x;
			rect.bottom = lpPoints[i].y;
			++i;

			//  Check the rectangle
			if ((rect.left == rect.right) && (rect.top != rect.bottom))
				++rect.right;
			else if ((rect.left != rect.right) && (rect.top == rect.bottom))
				++rect.bottom;

			pDC->FillRect(rect, pDC->GetCurrentBrush());
		}
	}
	__finally {
		PreparePatternBitmap(pDC, ::GetSysColor(COLOR_ACTIVEBORDER), TRUE);
	}
}


//  -----
void CVirtualGridCtrl::PaintFixed(CDC* pDC)
{
	int i, j;
	int nWidth;
	CRect rect;
	CGridCell cell;

	//  Top border of rows...
	rect.bottom = GetRowTopBottom(m_visOrigin.m_iRow).top;
	
	//  Look through the lines...
	for (j = 0; j < m_visSize.m_iRow; ++j) {
		rect.top = rect.bottom;
		rect.bottom += m_pRows->m_nHeight;
		rect.right = GetGridRect().left;

		//  Look through the columns...
		for (i = 0; i < m_pFixed->GetCount(); ++i) {
			cell.m_iColumn = i;
			cell.m_iRow = m_visOrigin.m_iRow + j;
			nWidth = (*m_pColumns)[cell.m_iColumn]->GetWidth();

			if (nWidth > 0)  {
				rect.left = rect.right;
				rect.right += nWidth;

				if (pDC->RectVisible(rect))
					PaintCell(pDC, cell, rect);
			}
		}
	}

	//  --------------  Line to the right  -----------------
	if (m_pFixed->m_bFlat)  {
		CPen penBtnShadow(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
		BOOL bSuccess;

		rect = GetFixedRect();

		//  If the colors of fixed part and the table are the same then we will
		//  paint the single line
		if (m_pFixed->m_bGridColor)  {
			if (gsDotLines & m_nGridStyles)  {
				rect.left = rect.right - 1;
				PaintDotGridLines(pDC, (LPPOINT)&rect, 2);
			}  else  {
				CPen* pOldPen;
				CPoint point;

				pOldPen = pDC->SelectObject(&penBtnShadow);

				point = pDC->MoveTo(rect.right - 1, rect.bottom - 1);
				bSuccess = pDC->LineTo(rect.right - 1, rect.top - 1);
				ASSERT(bSuccess);

				pOldPen = pDC->SelectObject(&penBtnShadow);
			}
		}  else  {
			//  ----------  Otherwise we will paint double line  -------------
			CPen penBtnHighlight(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
			CPen* pOldPen;
			CPoint point;

			pOldPen = pDC->SelectObject(&penBtnShadow);
			point = pDC->MoveTo(rect.right - 2, rect.top - 1);
			bSuccess = pDC->LineTo(rect.right - 2, rect.bottom);
			ASSERT(bSuccess);

			pDC->SelectObject(&penBtnHighlight);
			point = pDC->MoveTo(rect.right - 1, rect.bottom - 1);
			bSuccess = pDC->LineTo(rect.right - 1, rect.top - 1);
			ASSERT(bSuccess);
		}

		bSuccess = penBtnShadow.DeleteObject();
		ASSERT(bSuccess);
	}
}



//  ---
void CVirtualGridCtrl::PaintFixedGrid(CDC* pDC)
{
	int nStrokeCount;
	int nPointCount;
	int nLeft, nTop, nRight, nBottom;
	int x, y;
	int i, nIndex, c;
	LPPOINT lpPoints;
	LPDWORD lpPolyPoints;
	CRect rect;
	BOOL bSuccess;

	if (m_pFixed->m_bFlat)  {
		CPen penBtnShadow(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));

		nStrokeCount = 0;

		if (gsHorzLine & m_nGridStyles)  {
			nStrokeCount = m_visSize.m_iRow;
			if (gsListViewLike & m_nGridStyles)
				nStrokeCount = GetGridHeight() / m_pRows->m_nHeight;
		}

		if (gsVertLine & m_nGridStyles)
			nStrokeCount += m_pFixed->GetCount();

		//  Is there a mesh?
		if (nStrokeCount > 0)  {
			//  Calculate amount of mesh points
			nPointCount = nStrokeCount * 2;

			//  Get the memory for lines...
			lpPoints = new POINT[nPointCount];
			lpPolyPoints = new DWORD[nStrokeCount];

			for (i = 0; i < nStrokeCount; ++i)
				lpPolyPoints[i] = 2;

			rect = GetFixedRect();

			//  --------  Vertical line points...  ----
			if (gsVertLine & m_nGridStyles)  {
				nTop = rect.top;
				nBottom = rect.bottom;

				if (!(gsFullVertLine & m_nGridStyles) && !(gsListViewLike & m_nGridStyles))
					nBottom = GetRowTopBottom(m_visOrigin.m_iRow + m_visSize.m_iRow).top;

				x = rect.left;

				for (i = 0; i < m_pFixed->GetCount(); ++i) {
					x += m_pColumns->GetAt(i)->GetWidth();
					nIndex = i * 2;
					lpPoints[nIndex].x = x - 2;
					lpPoints[nIndex].y = nTop;
					lpPoints[nIndex + 1].x = x - 2;
					lpPoints[nIndex + 1].y = nBottom;
				}
			}

			//  ----------  Horizontal line points...  -----------
			if (gsHorzLine & m_nGridStyles)  {
				nLeft = rect.left;
				nRight = rect.right;
				y = GetRowTopBottom(m_visOrigin.m_iRow).top;
				c = m_visSize.m_iRow;

				if (gsListViewLike & m_nGridStyles)
					c = GetGridHeight() / m_pRows->m_nHeight;

				for (i = 0; i < c; ++i) {
					y += m_pRows->m_nHeight;
					nIndex = m_pFixed->GetCount() * (0 < (gsVertLine & m_nGridStyles)) * 2 + i * 2;
					lpPoints[nIndex].x = nLeft;
					lpPoints[nIndex].y = y - 2;
					lpPoints[nIndex + 1].x = nRight;
					lpPoints[nIndex + 1].y = y - 2;
				}
			}

			//  Double or single lines?
			if (m_pFixed->m_bGridColor) {
				//  Single line...
				//  We must offset lines (they actually has been calculated for first double line)
				ShiftGridPoints(nPointCount, lpPoints, 1, 1);

				if (gsDotLines & m_nGridStyles)  {
					PaintDotGridLines(pDC, lpPoints, nPointCount);
				}  else  {
					CPen* pOldPen;

					pOldPen = pDC->SelectObject(&penBtnShadow);
					bSuccess = pDC->PolyPolyline(lpPoints, lpPolyPoints, nStrokeCount);
					ASSERT(bSuccess);

					pOldPen = pDC->SelectObject(pOldPen);
				}

			}  else  {
				//  Double line...
				CPen penBtnHighlight(PS_SOLID, 1, COLOR_BTNHIGHLIGHT);
				CPen* pOldPen;

				//  First, paint dark lines...
				pOldPen = pDC->SelectObject(&penBtnShadow);
				bSuccess = pDC->PolyPolyline(lpPoints, lpPolyPoints, nStrokeCount);
				ASSERT(bSuccess);

				//  Offset the lines...
				ShiftGridPoints(nPointCount, lpPoints, 1, 1);

				//  Second, paint light lines...
				pDC->SelectObject(&penBtnHighlight);
				bSuccess = pDC->PolyPolyline(lpPoints, lpPolyPoints, nStrokeCount);
				ASSERT(bSuccess);

				pOldPen = pDC->SelectObject(pOldPen);

			}

			delete[] lpPoints;
			delete[] lpPolyPoints;
		}
	}  else if ((gsHorzLine & m_nGridStyles) && (gsVertLine & m_nGridStyles))  {
		//  Paint ALL 3D fixed cells
		int nWidth;

		rect = GetFixedRect();

		if (!(gsListViewLike & m_nGridStyles))
			rect.bottom = rect.top + m_visSize.m_iRow * m_pRows->m_nHeight;

		//  Paint 3D cells
		CRect cellRect(rect);

		cellRect.bottom = cellRect.top;
		//  Rows...
		while (cellRect.bottom < rect.bottom)  {
			cellRect.top = cellRect.bottom;
			cellRect.bottom += m_pRows->m_nHeight;
			cellRect.right = GetFixedRect().left;

			//  Columns...
			for (i = 0; i < m_pFixed->GetCount(); ++i) {
				nWidth = m_pColumns->GetAt(i)->GetWidth();
				if (nWidth > 0)  {
					cellRect.left = cellRect.right;
					cellRect.right += nWidth;
					if (pDC->RectVisible(cellRect))
						Paint3DFrame(pDC, cellRect, BF_RECT);
				}
			}
		}

		//  Paint remainder...
		if (!(gsListViewLike & m_nGridStyles))  {
			rect.top = rect.bottom;
			rect.bottom = GetFixedRect().bottom;
			if (gsFullVertLine & m_nGridStyles)	//  Vertical lines only
				PaintVert3DLines(pDC, rect, FALSE);
			else
				PaintBottom3DMargin(pDC, rect);		//  Remainder to the bottom
		}
	}  else if ((gsHorzLine & m_nGridStyles) && !(gsVertLine & m_nGridStyles))  {
		//  We need to paint only 3D horizontal lines
		rect = GetFixedRect();
		if (!(gsListViewLike & m_nGridStyles))
			rect.bottom = rect.top + m_visSize.m_iRow * m_pRows->m_nHeight;

		//  Horizontal 3D lines
		PaintHorz3DLines(pDC, rect);

		//  Remainder...
		if (!(gsListViewLike & m_nGridStyles))  {
			rect.top = rect.bottom;
			rect.bottom = GetFixedRect().bottom;
			PaintBottom3DMargin(pDC, rect);
		}
	}  else if (!(gsHorzLine & m_nGridStyles) && (gsVertLine & m_nGridStyles))  {
		//  We need to paint only vertical 3D lines
		rect = GetFixedRect();
		PaintVert3DLines(pDC, rect, FALSE);
	}  else  {
		//  Just 3d frame around fixed part
		rect = GetFixedRect();
		PaintBottom3DMargin(pDC, rect);
	}
}



//  ---
void CVirtualGridCtrl::PaintFreeField(CDC* pDC)
{
	int x, y;
	CRect rect;

	//  Free field to the right...
	x = GetColumnRect(m_visOrigin.m_iColumn + m_visSize.m_iColumn).left;
	rect = GetGridRect();

	if (x < rect.right)  {
		rect.left = x;
		pDC->FillSolidRect(rect, ::GetSysColor(COLOR_WINDOW));
	}

	//  Free field at the bottom...
	y = GetRowRect(m_visOrigin.m_iRow + m_visSize.m_iRow).top;
	rect = GetGridRect();

	if (y < rect.bottom)  {
		rect.left = GetFixedRect().right;
		rect.top = y;

		pDC->FillSolidRect(rect, ::GetSysColor(COLOR_WINDOW));

		//  Free field under the fixed part
		rect.right = rect.left;
		rect.left = GetFixedRect().left;

		if (rect.Width() && rect.Height())
			pDC->FillSolidRect(rect, m_pFixed->m_color);
	}

	//  ...Resize line
	if (m_bColResizing && m_nColResizeCount > 0 && !m_bColumnsFullDrag)
		PaintResizeLine();
}


//  ----------------------------------------------------------------------------
void CVirtualGridCtrl::PaintFocus(CDC* pDC)
{
	//  Is focus visible
	if (m_bShowFocusRect && (this == GetFocus()) && m_visSize.m_iRow > 0)  {
		CRect rect;
		CRect gridRect;
		int nRetVal;

		//  Focus rectangle
		rect = GetFocusRect();

		//  Taking into account the grid lines...
		if (m_bGridLines)  {
			if (gsVertLine & m_nGridStyles)
				rect.right -= m_nGridLineWidth;
			if (gsHorzLine & m_nGridStyles)
				rect.bottom -= m_nGridLineWidth;
		}

		//  Let's paint...
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->SetBkMode(OPAQUE);
		pDC->SetROP2(R2_COPYPEN);

		//  Cut off a place for header and fixed part
		gridRect = GetGridRect();
		gridRect.left = GetFixedRect().right;
		nRetVal = pDC->IntersectClipRect(gridRect);
		ASSERT(nRetVal != ERROR);

		pDC->DrawFocusRect(rect);
	}
}


//  -----------------------------------------------------------------------------
void CVirtualGridCtrl::PaintGridLines(CDC* pDC)
{
	int nStrokeCount;
	int nPointCount;
	int i;
	int nLeft, nTop, nRight, nBottom, x, y, c;
	int nIndex;
	CRect rect;
	LPPOINT lpPoints;
	LPDWORD lpPolyPoints;

	nStrokeCount = 0;

	if (gsHorzLine & m_nGridStyles)  {
		nStrokeCount = m_visSize.m_iRow;

		if (gsListViewLike & m_nGridStyles)
			nStrokeCount = GetGridHeight() / m_pRows->m_nHeight;
	}

	if (gsVertLine & m_nGridStyles)
		nStrokeCount += m_visSize.m_iColumn;

	//  Is there a mesh?
	if (nStrokeCount)  {
		//  Let's calculate amount of mesh points
		nPointCount = nStrokeCount * 2;

		//  Allocate memory...
		lpPoints = new POINT[nPointCount];
		lpPolyPoints = new DWORD[nStrokeCount];

		for (i = 0; i < nStrokeCount; ++i)
			lpPolyPoints[i] = 2;

		rect = GetGridRect();

		//  Vertical line points...
		if (gsVertLine & m_nGridStyles)  {
			nTop = rect.top;
			nBottom = rect.bottom;

			if (!(gsFullVertLine & m_nGridStyles) && !(gsListViewLike & m_nGridStyles))
				nBottom = GetRowTopBottom(m_visOrigin.m_iRow + m_visSize.m_iRow).top;

			x = GetColumnLeftRight(m_visOrigin.m_iColumn).left;

			for (i = 0; i < m_visSize.m_iColumn; ++i) {
				x += (*m_pColumns)[m_visOrigin.m_iColumn + i]->GetWidth();
				nIndex = i * 2;

				lpPoints[nIndex].x = x - 1;
				lpPoints[nIndex].y = nTop;
				lpPoints[nIndex + 1].x = x - 1;
				lpPoints[nIndex + 1].y = nBottom;
			}
		}

		//  Horizontal line points...
		if (gsHorzLine & m_nGridStyles)  {
			nLeft = rect.left + GetFixedWidth();
			nRight = rect.right;

			if (!(gsFullHorzLine & m_nGridStyles) && !(gsListViewLike & m_nGridStyles))
				nRight = GetColumnLeftRight(m_visOrigin.m_iColumn + m_visSize.m_iColumn).left;

			y = GetRowTopBottom(m_visOrigin.m_iRow).top;

			c = m_visSize.m_iRow;

			if (gsListViewLike & m_nGridStyles)
				c = GetGridHeight() / m_pRows->m_nHeight;

			for (i = 0; i < c; ++i) {
				y += m_pRows->m_nHeight;
				nIndex = m_visSize.m_iColumn * ((gsVertLine & m_nGridStyles) > 0) * 2 + i * 2;

				lpPoints[nIndex].x = nLeft;
				lpPoints[nIndex].y = y - 1;
				lpPoints[nIndex + 1].x = nRight;
				lpPoints[nIndex + 1].y = y - 1;
			}
		}

		//  ----------  Let's paint  ------------
		if (!(gsDotLines & m_nGridStyles))  {
			CPen penGridLines(PS_SOLID, 1, ::GetSysColor(COLOR_ACTIVEBORDER));
			BOOL bSuccess;
			CPen* pOldPen;

			pOldPen = pDC->SelectObject(&penGridLines);

			bSuccess = pDC->PolyPolyline(lpPoints, lpPolyPoints, nStrokeCount);
			ASSERT(bSuccess);

			pOldPen = pDC->SelectObject(pOldPen);
		}  else  {
			PaintDotGridLines(pDC, lpPoints, nPointCount);
		}

		delete[] lpPoints;
		delete lpPolyPoints;
	}
}



//  ---
void CVirtualGridCtrl::PaintHeader(CDC* pDC, CGridHeaderSection* pSection, LPRECT lpRect)
{
	GetHeaderColors(pSection, pDC);

	//  NYI

	DefaultDrawHeader(pDC, pSection, lpRect);
}




//  ------------------  Paint ALL of the Grid's Header Sectinons  --------------------
void CVirtualGridCtrl::PaintHeaders(CDC* pDC, BOOL bDrawFixed)
{
	CFont* pOldFont;
	CRect clientRect;
	CRect rect;
	CPen* pOldPen;
	BOOL bSuccess;
	CPoint point;

	pOldFont = pDC->SelectObject(m_pHeader->m_pFont);

	GetClientRect(clientRect);

	//  --------  Subheaders  ----------
	PaintHeaderSections(pDC, &m_pHeader->m_sections, bDrawFixed);

	//  Remaining place to the right...
	rect = GetHeaderRect();

	rect.left = clientRect.left + m_pHeader->GetWidth() + GetGridOrigin().x;
	if (rect.left < rect.right)  {
		pDC->FillSolidRect(rect, m_pHeader->m_color);

		if (!m_pHeader->m_bFlat)
			Paint3DFrame(pDC, rect, BF_LEFT | BF_TOP | BF_BOTTOM);
	}

	//  -----------  Grey line below...  -----------
	if (m_pHeader->m_bFlat)  {
		CPen penBtnShadow(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));

		//  Adjust rectangle borders
		if (bDrawFixed)  {
			rect.left = clientRect.left;
			rect.right = GetFixedRect().right;
		}  else  {
			rect.left = GetFixedRect().left;
			rect.right = clientRect.right;
		}

		//  ------------------  Let's paint  --------------------
		if (m_pHeader->m_bGridColor)  {
			CPen pen(PS_SOLID, m_nGridLineWidth, ::GetSysColor(COLOR_ACTIVEBORDER));

			pOldPen = pDC->SelectObject(&pen);

			point = pDC->MoveTo(rect.left, rect.bottom - 1);
			bSuccess = pDC->LineTo(rect.right, rect.bottom - 1);
			bSuccess = pen.DeleteObject();
			ASSERT(bSuccess);
		}  else  {	//  Otherwise paint double line
			CPen penBtnHighlight(PS_SOLID, 1, ::GetSysColor(COLOR_BTNHIGHLIGHT));

			pOldPen = pDC->SelectObject(&penBtnShadow);
			point = pDC->MoveTo(rect.left, rect.bottom - 2);
			bSuccess = pDC->LineTo(rect.right, rect.bottom - 2);

			pDC->SelectObject(&penBtnHighlight);
			point = pDC->MoveTo(rect.left, rect.bottom - 1);
			bSuccess = pDC->LineTo(rect.right, rect.bottom - 1);
		}

	}

	if (m_bColResizing && m_nColResizeCount > 0 && !m_bColumnsFullDrag)
		PaintResizeLine();

	pOldFont = pDC->SelectObject(pOldFont);
}




//  ---
void CVirtualGridCtrl::PaintHeaderSections(CDC* pDC, CGridHeaderSections* pSections, BOOL bAllowFixed)
{
	int i;
	CGridHeaderSection* pSection;
	CRect rect;
	CRect sectionRect;

	for (i = 0; i < pSections->GetSize(); ++i) {
		pSection = pSections->GetSection(i);

		if (bAllowFixed == pSection->GetFixedColumn())  {
			rect = pSection->GetBoundsRect();

			if (rect.Width())  {
				sectionRect = rect;

				if (pSections->m_sections.GetSize())
					sectionRect.bottom = GetHeaderRect().bottom;

				if (pDC->RectVisible(sectionRect))  {
					PaintHeader(pDC, pSection, rect);
					PaintHeaderSections(pDC, &pSection->m_sections, bAllowFixed);
				}
			}
		}  else  {
			PaintHeaderSections(pDC, &pSection->m_sections, bAllowFixed);
		}
	}
}



//  -----------------------------------------------------------------------------
void CVirtualGridCtrl::PaintResizeLine()
{
	CDC* pDC;
	CPen* pOldPen;
	BOOL bSuccess;
	int nOldROP2;
	CPoint point;
	CPen pen(PS_SOLID, 1, RGB(255, 255, 255));

	pDC = GetDC();


	pOldPen = pDC->SelectObject(&pen);
	nOldROP2 = pDC->SetROP2(R2_XORPEN);

	point = pDC->MoveTo(m_nColResizePos, m_colResizeRect.top);
	bSuccess = pDC->LineTo(m_nColResizePos, m_colResizeRect.bottom);
	ASSERT(bSuccess);

	nOldROP2 = pDC->SetROP2(nOldROP2);
	pOldPen = pDC->SelectObject(pOldPen);

	bSuccess = ReleaseDC(pDC);
	ASSERT(bSuccess);
}




//  ----
void CVirtualGridCtrl::PaintText(
				 CDC* pDC,
				 LPRECT lpRect,
				 int nLeftIndent,
				 int nTopIndent,
				 int nAlignment,
				 BOOL bWantReturns,
				 BOOL bWordWrap,
				 LPCTSTR lpszText
				 )
{
	DRAWTEXTPARAMS dtp;
	UINT nFlags;
	int nRetVal;


	//  -----------  Text out  ---------------
	if (bWantReturns || bWordWrap || m_bEndEllipsis)  {
		//  Fill draw text parameters
		dtp.cbSize = sizeof(DRAWTEXTPARAMS);
		dtp.iLeftMargin = nLeftIndent;
		dtp.iRightMargin = m_nTextRightIndent;
		dtp.iTabLength = 0;
		dtp.uiLengthDrawn = 0;

		//  Text attributes
		nFlags = DT_NOPREFIX;

		//  Horizontal alignment
		switch (nAlignment)  {
		case LVCFMT_LEFT:
			nFlags |= DT_LEFT;
			break;
		case LVCFMT_RIGHT:
			nFlags |= DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nFlags |= DT_CENTER;
			break;
		default:
			break;
		}

		//  Vertical alignment
		if (!(bWantReturns || bWordWrap))  {
			//  Auto alignment
			nFlags |= DT_SINGLELINE;

			if (nAlignment == LVCFMT_LEFT)
				nFlags |= DT_END_ELLIPSIS;
		}

		//  Word wrap
		if (bWordWrap)
			nFlags |= DT_WORDBREAK;

		//  Top indent
		lpRect->top += nTopIndent;

		//  Draw the text
		nRetVal = pDC->SetBkMode(TRANSPARENT);

		nRetVal = DrawTextEx(
			pDC->m_hDC,
			const_cast<LPTSTR>(lpszText),
			lstrlen(lpszText),
			lpRect,
			nFlags,
			&dtp
			);
	
	}  else  {
		int dx;
		UINT nAl;
		BOOL bSuccess;

		//  Horizontal alignment
		switch (nAlignment)  {
		case LVCFMT_RIGHT:
			dx = lpRect->right - lpRect->left - m_nTextRightIndent;
			nAl = TA_RIGHT;
			break;
		case LVCFMT_CENTER:
			dx = nLeftIndent + (lpRect->right - lpRect->left) / 2;
			nAl = TA_CENTER;
			break;
		default:
			dx = nLeftIndent;
			nAl = TA_LEFT;
			break;
		}

		//  Standard text out
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextAlign(nAl);

		bSuccess = pDC->ExtTextOut(
			lpRect->left + dx,
			lpRect->top + nTopIndent,
			ETO_CLIPPED,
			lpRect,
			lpszText,
			lstrlen(lpszText),
			0
			);
		ASSERT(bSuccess);
	}
}


//  ---
void CVirtualGridCtrl::PreparePatternBitmap(CDC* pDC, COLORREF fillColor, BOOL bRemove)
{
	BOOL bSuccess;
	COLORREF pattern[4];
	CBrush* pOldBrush;
	DWORD dwBytes;

	if (bRemove)  {
		//  ???
	}

	if ((m_pHorzScrollBar->GetPosition() %  2) == 0) {
		pattern[0] = pDC->GetBkColor();
		pattern[1] = fillColor;
		pattern[2] = fillColor;
		pattern[3] = pDC->GetBkColor();
	}  else  {
		pattern[0] = fillColor;
		pattern[1] = pDC->GetBkColor();
		pattern[2] = fillColor;
		pattern[3] = pDC->GetBkColor();
	}

	dwBytes = m_bitmapPattern.SetBitmapBits(4 * sizeof(COLORREF), pattern);

	CBrush brushDotLine;
	bSuccess = brushDotLine.CreatePatternBrush(&m_bitmapPattern);
	ASSERT(bSuccess);

	pOldBrush = pDC->SelectObject(&brushDotLine);
	if (pOldBrush->GetSafeHandle())  {
		bSuccess = pOldBrush->DeleteObject();
	}
}


//  ---
void CVirtualGridCtrl::ResetClickPos()
{
	m_clickPos.m_iColumn = -1;
	m_clickPos.m_iRow = -1;
}


//  ----
void CVirtualGridCtrl::SetEditText(const CGridCell &cell, CString& strText)
{
	VG_DISPINFO dispInfo;
	LRESULT lResult;
	TCHAR szBuffer[MAX_CELL_TEXT];

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->m_hWnd));

	dispInfo.hdr.code = VGN_SETDISPINFO;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.item.iColumn = cell.m_iColumn;
	dispInfo.item.iRow = cell.m_iRow;
	wcscpy(szBuffer, strText);
	dispInfo.item.pszText = szBuffer;

	lResult = pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo);
	strText = dispInfo.item.pszText;
}


//  ---
void CVirtualGridCtrl::ShowCursor()
{
	if (IsFocusAllowed())
		InvalidateFocus();
	else
		ShowEdit();
}


//  --
void CVirtualGridCtrl::ShowEdit()
{
	UpdateEdit();
}


//  ---
void CVirtualGridCtrl::ShowEditChar(UINT nChar)
{
	SetEditing();

	if (m_pEdit && m_bEditing)
		m_pEdit->PostMessage(WM_CHAR, nChar, 0);
}


//  ---
void CVirtualGridCtrl::ShowFocus()
{
	CDC* pDC;
	int nRetVal;

	pDC = GetDC();

	if (IsFocusAllowed())
		PaintFocus(pDC);

	nRetVal = ReleaseDC(pDC);
	ASSERT(nRetVal);
}



//  -----------------------------------------------------------------------------------
void CVirtualGridCtrl::StartColResize(CGridHeaderSection* pSection, int x, int y)
{
	CRect clientRect;

	GetClientRect(clientRect);
	m_pColResizeSection = pSection;
	m_iColResizeIndex = pSection->GetResizeColumnIndex();

	//  Horizontal borders...
	if (m_iColResizeIndex <= m_pColumns->GetUpperBound())  {
		m_colResizeRect = GetColumnRect(m_iColResizeIndex);
		m_colResizeRect.bottom = clientRect.bottom;	//  ???
		m_nColResizeMinWidth = (*m_pColumns)[m_iColResizeIndex]->GetMinWidth();
		m_nColResizeMaxWidth = (*m_pColumns)[m_iColResizeIndex]->GetMaxWidth();
	}  else  {
		m_colResizeRect = pSection->GetBoundsRect();
		m_colResizeRect.bottom = clientRect.bottom;
		m_nColResizeMinWidth = 0;
		m_nColResizeMaxWidth = 10000;
	}

	//  Vertical borders...
	m_colResizeRect.top = pSection->GetLevel() * m_pHeader->m_nSectionHeight;
	m_colResizeRect.bottom = clientRect.Height();

	//  Resize line...
	m_nColResizePos = m_colResizeRect.right;
	m_nColResizeOffset = m_nColResizePos - x;

	//  Now we can change columns size
	m_nColResizeCount = 0;
	m_bColResizing = TRUE;

	//  Capture mouse
	SetCapture();
}


//  ---
void CVirtualGridCtrl::StartHeaderClick(CGridHeaderSection* pSection, CPoint point)
{
	BOOL bAllowClick;

	bAllowClick = TRUE;
	HeaderClicking(pSection, bAllowClick);

	if (!bAllowClick)
		return;

	//  Memorize parameters...
	m_pHeaderClickSection = pSection;
	m_headerClickRect = pSection->GetBoundsRect();
	m_bHeaderClickState = FALSE;
	m_bHeaderClicking = TRUE;

	SetCapture();

	//  Press the 'button'
	StepHeaderClick(point);
}



//  ---
void CVirtualGridCtrl::StepHeaderClick(CPoint point)
{
	BOOL bHit;

	if (!m_bHeaderClicking)
		return;		//  Nothing to stop

	bHit = m_headerClickRect.PtInRect(point);

	//  Is something changed there?
	if (m_bHeaderClickState != bHit)  {
		m_bHeaderClickState = bHit;
		InvalidateRect(m_headerClickRect);
	}
}



//  ------
void CVirtualGridCtrl::StopHeaderClick(BOOL bAbort)
{
	BOOL bHit;
	BOOL bSuccess;

	if (!m_bHeaderClicking)
		return;

	bHit = m_bHeaderClickState;

	//  Button up
	StepHeaderClick(CPoint(-1, -1));

	m_bHeaderClicking = FALSE;
	bSuccess = ReleaseCapture();
	ASSERT(bSuccess);

	if (!bAbort && bHit)
		HeaderClick(m_pHeaderClickSection);
}





//
void CVirtualGridCtrl::StepColResize(int x, int y)
{
	int nWidth;
	CRect rect;

	//  No resizing at all...
	if (!m_bColResizing)
		return;

	//  Current resize line position
	x += m_nColResizeOffset;

	//  Current width
	nWidth = x - m_colResizeRect.left;

	//  Adjust width according to borders
	if (nWidth < m_nColResizeMinWidth)
		nWidth = m_nColResizeMinWidth;

	if (nWidth > m_nColResizeMaxWidth)
		nWidth = m_nColResizeMaxWidth;

	ColumnResizing(m_iColResizeIndex, nWidth);

	//  Adjust the width again...
	if (nWidth < m_nColResizeMinWidth)
		nWidth = m_nColResizeMinWidth;

	if (nWidth > m_nColResizeMaxWidth)
		nWidth = m_nColResizeMaxWidth;

	//  New line position
	x = m_colResizeRect.left + nWidth;

	// ---  Paint the line  -----
	if (m_nColResizePos != x)  {
		//  Paint over old line
		if (m_nColResizeCount > 0 && !m_bColumnsFullDrag)
			PaintResizeLine();

		++m_nColResizeCount;

		//  A new line postion...
		m_nColResizePos = x;

		//  Set the width
		if (m_bColumnsFullDrag && m_iColResizeIndex < m_pColumns->GetSize())  {
			CRect rect;
			CGridHeaderSection* pSection;

			//  Before changing column width we should calculate and
			//  update changing cells
			GetClientRect(rect);
			rect.left = GetColumnLeftRight(m_iColResizeIndex).left;

			//  If non-fixed column partially overlayed by fixed part
			//  then it's not necessary to repaint it
			if (m_iColResizeIndex >= m_pFixed->GetCount())
				rect.left = max(rect.left, GetFixedRect().right);

			if (nWidth < (*m_pColumns)[m_iColResizeIndex]->GetWidth()) {
				if ((m_pHorzScrollBar->GetRange() > m_pHorzScrollBar->GetPageStep())
					&& (m_pHorzScrollBar->GetPosition() ==
					m_pHorzScrollBar->GetRange() - m_pHorzScrollBar->GetPageStep())) {

					if (m_iColResizeIndex < m_pColumns->GetCount() - 1) {
						CRect rcFixed = GetFixedRect();
						rect.left = rcFixed.right;

						CRect rcLeftRight = GetColumnLeftRight(m_iColResizeIndex + 1);

						rect.right = rcLeftRight.left;
					}
				}
			}

			InvalidateRect(rect);

			//  If the column has multi-level header then we should additionally
			//  update topmost section
			pSection = GetHeaderSection(m_iColResizeIndex, 0);
			if (pSection)  {
				rect = pSection->GetBoundsRect();
				rect.bottom = GetHeaderRect().bottom;
				InvalidateRect(rect);
			}

			(*m_pColumns)[m_iColResizeIndex]->SetWidth(nWidth);

			//  And now we should repaint...
			UpdateWindow();
		}  else  {
			//  Paint the new line
			PaintResizeLine();
		}
	}  else  {
		//  Paint resize line at the first time
		if (m_nColResizeCount == 0 && !m_bColumnsFullDrag)
			PaintResizeLine();
		++m_nColResizeCount;
	}
}



//  --------
void CVirtualGridCtrl::StopColResize(BOOL bAbort)
{
	int nWidth;
	BOOL bSuccess;

	if (!m_bColResizing)
		return;		//  Nothing to stop

	__try {
		//  Free our poor rodent
		bSuccess = ReleaseCapture();
		ASSERT(bSuccess);

		//  Is there at least one resize action
		if (m_nColResizeCount > 0)  {
			//  Paint over the line
			if (!m_bColumnsFullDrag)
				PaintResizeLine();

			if (bAbort)
				return;

			//  --------  Set column size  ------------
			//  New width...
			nWidth = m_nColResizePos - m_colResizeRect.left;

			//  Adjust the width according to borders
			if (nWidth < m_nColResizeMinWidth)
				nWidth = m_nColResizeMinWidth;
			if (nWidth > m_nColResizeMaxWidth)
				nWidth = m_nColResizeMaxWidth;

			//  User's event
			ColumnResize(m_iColResizeIndex, nWidth);

			//  Adjust the width again
			if (nWidth < m_nColResizeMinWidth)
				nWidth = m_nColResizeMinWidth;
			if (nWidth > m_nColResizeMaxWidth)
				nWidth = m_nColResizeMaxWidth;

			//  Set the width
			if (m_iColResizeIndex < m_pColumns->GetSize())
				(*m_pColumns)[m_iColResizeIndex]->SetWidth(nWidth);

			m_pColResizeSection->SetWidth(nWidth);
		}
	}
	__finally {
		m_bColResizing = FALSE;
	}

	HeaderChange();
}



//  -----------------------------------------------------------------------
void CVirtualGridCtrl::Paint3DFrame(CDC* pDC, LPRECT lpRect, UINT nSideFlags)
{
	CRect rect(*lpRect);	//  !!!!!!!!
	BOOL bSuccess;

	bSuccess = pDC->DrawEdge(rect, BDR_RAISEDOUTER, nSideFlags & (~BF_TOPLEFT));
	ASSERT(bSuccess);

	if (nSideFlags & BF_BOTTOM)
		rect.bottom --;
	if (nSideFlags & BF_RIGHT)
		rect.right --;

	bSuccess = pDC->DrawEdge(rect, BDR_RAISEDINNER, nSideFlags & (~BF_BOTTOMRIGHT));
	ASSERT(bSuccess);

	if (nSideFlags & BF_TOP)
		++rect.top;
	if (nSideFlags & BF_LEFT)
		++rect.left;

	bSuccess = pDC->DrawEdge(rect, BDR_RAISEDINNER, nSideFlags & (~BF_TOPLEFT));
	ASSERT(bSuccess);

	if (nSideFlags & BF_BOTTOM)
		rect.bottom --;
	if (nSideFlags & BF_RIGHT)
		rect.right --;

	bSuccess = pDC->DrawEdge(rect, BDR_RAISEDOUTER, nSideFlags & (~BF_BOTTOMRIGHT));
	ASSERT(bSuccess);
}



//  ----------------------------------------------------------------------
void CVirtualGridCtrl::HideEdit()
{
	if (m_pEdit == 0 || !(::IsWindow(m_pEdit->m_hWnd)))
		return;		//  Nothing to hide

	m_editCell.m_iColumn = -1;
	m_editCell.m_iRow = -1;
	m_pEdit->Hide();
}



//  -----
int CVirtualGridCtrl::GetTextTopIndent()
{
	return m_nTextTopIndent;
}


//  ---
CGridHeaderSections* CVirtualGridCtrl::GetHeaderSections()
{
	return m_pHeader->GetSections();
}


//  ---
CGridColumn* CVirtualGridCtrl::GetColumn(int nIndex)
{
	if (!m_pColumns->GetSize())
		return 0;

	if (nIndex < 0 || nIndex > m_pColumns->GetUpperBound())
		return 0;

	return (*m_pColumns)[nIndex];
}



//  ---
COLORREF CVirtualGridCtrl::GetGridColor()
{
	return m_gridColor;
}


//  ---
BOOL CVirtualGridCtrl::GetCheckBoxes()
{
	return m_bCheckBoxes;
}


//  ---
int CVirtualGridCtrl::GetCheckHeight()
{
	return m_nCheckHeight;
}


//  ---
int CVirtualGridCtrl::GetCheckTopIndent()
{
	return m_nCheckTopIndent;
}


//  ---
BOOL CVirtualGridCtrl::GetGridLines()
{
	return m_bGridLines;
}



//  ---
UINT CVirtualGridCtrl::GetGridStyle()
{
	return m_nGridStyles;
}


//  ---
CImageList* CVirtualGridCtrl::GetImageList()
{
	return m_pImageList;
}



//  ---
int CVirtualGridCtrl::GetImageTopIndent()
{
	return m_nImageTopIndent;
}


//  ----
int CVirtualGridCtrl::GetGridLineWidth()
{
	return m_nGridLineWidth;
}




//  -----------------------------------------------------------------------------

//  ---------------  Some navigation methods  -----------------

CGridCell CVirtualGridCtrl::MoveLeft(const CGridCell& cell, int nOffset)
{
	CGridCell newCell;
	int i;

	i = max(cell.m_iColumn - nOffset, m_pFixed->GetCount());

	while (i >= m_pFixed->GetCount())  {
		newCell.m_iColumn = i;
		newCell.m_iRow = cell.m_iRow;

		//  Trying to set the cursor...
		if (IsCellAcceptCursor(newCell))
			return newCell;

		//  Previous column
		--i;
	}

	return cell;
}


//  ---
CGridCell CVirtualGridCtrl::MoveRight(const CGridCell& cell, int nOffset)
{
	CGridCell newCell;
	int i;

	//  New active column
	i = min(cell.m_iColumn + nOffset, m_pColumns->GetUpperBound());

	while (i <= m_pColumns->GetUpperBound())  {
		newCell.m_iColumn = i;
		newCell.m_iRow = cell.m_iRow;

		//  Trying to set the cursor...
		if (IsCellAcceptCursor(newCell))
			return newCell;

		//  Next column
		++i;
	}

	return cell;
}



//  ---
CGridCell CVirtualGridCtrl::MoveUp(const CGridCell& cell, int nOffset)
{
	CGridCell newCell;
	int i;

	//  New active row
	i = max(cell.m_iRow - nOffset, 0);

	while (i >= 0)  {
		newCell.m_iColumn = cell.m_iColumn;
		newCell.m_iRow = i;

		//  Trying to set the cursor...
		if (IsCellAcceptCursor(newCell))
			return newCell;

		//  Previous row
		i --;
	}

	return cell;
}


//  ---
CGridCell CVirtualGridCtrl::MoveDown(const CGridCell& cell, int nOffset)
{
	CGridCell newCell;
	int j;

	//  New active row
	j = min(cell.m_iRow + nOffset, m_pRows->GetCount() - 1);

	while (j <= m_pRows->GetCount() - 1)  {
		newCell.m_iColumn = cell.m_iColumn;
		newCell.m_iRow = j;

		//  Trying to set the cursor...
		if (IsCellAcceptCursor(newCell))
			return newCell;

		++j;
	}

	return cell;
}


//  ---
CGridCell CVirtualGridCtrl::MoveHome(CGridCell& cell)
{
	CGridCell newCell(cell);

	newCell.m_iColumn = m_pFixed->m_nCount;
	newCell = MoveRight(newCell, 0);

	return newCell;
}


//  ---
CGridCell CVirtualGridCtrl::MoveEnd(CGridCell& cell)
{
	CGridCell newCell(cell);

	newCell.m_iColumn = m_pColumns->GetUpperBound();
	newCell = MoveLeft(newCell, 0);

	return newCell;
}


//  ---
CGridCell CVirtualGridCtrl::MoveGridHome(const CGridCell& cell)
{
	int i, j;
	CGridCell newCell;

	//  New active column
	i = m_pFixed->GetCount();

	while (i <= cell.m_iColumn) {
		//  New active row...
		j = 0;

		// --
		while (j <= cell.m_iRow)  {
			newCell.m_iColumn = i;
			newCell.m_iRow = j;

			//  Trying to set the cursor...
			if (IsCellAcceptCursor(newCell))
				return newCell;

			//  Next row
			++j;
		}

		//  New column
		++i;
	}

	return cell;
}


//  ---
CGridCell CVirtualGridCtrl::MoveGridEnd(const CGridCell& cell)
{
	int i, j;
	CGridCell newCell;

	//  New active cell
	i = m_pColumns->GetUpperBound();

	while (i >= cell.m_iColumn) {
		j = m_pRows->GetCount() - 1;

		//  ---
		while (j >= cell.m_iRow) {
			newCell.m_iColumn = i;
			newCell.m_iRow = j;

			//  Trying to set the cursor
			if (IsCellAcceptCursor(newCell))
				return newCell;

			//  Previous row...
			j --;
		}

		//  Previous column
		i --;
	}

	return cell;
}



//  ---
CGridCell CVirtualGridCtrl::Select(const CGridCell& cell)
{
	CGridCell newCell;

	//  Is given cell accessible?
	if (IsCellAcceptCursor(cell))
		return cell;

	//  If selection is on the left then let's seek on the left
	if (cell.m_iColumn < m_cellFocused.m_iColumn) {
		int i = max(cell.m_iColumn, m_pFixed->GetCount());

		while (i <= m_cellFocused.m_iColumn) {
			newCell.m_iColumn = i;
			newCell.m_iRow = cell.m_iRow;

			//  Trying to set the cursor
			if (IsCellAcceptCursor(newCell))
				break;
			//  The next column...
			++i;
		}

		//  The cell is not found
		newCell = cell;

		if (IsCellAcceptCursor(newCell))
			return newCell;
	}

	//  If selection is on the right then let's seek on the right
	if (cell.m_iColumn > m_cellFocused.m_iColumn) {
		int i = min(cell.m_iColumn, m_pColumns->GetUpperBound());

		while (i >= m_cellFocused.m_iColumn)  {
			newCell.m_iColumn = i;
			newCell.m_iRow = cell.m_iRow;

			//  Try to set the cursor...
			if (IsCellAcceptCursor(newCell))
				break;
			//  Previous column...
			i --;
		}
		//  The cell is not found
		newCell = cell;

		if (IsCellAcceptCursor(newCell))
			return newCell;
	}

	//  If selection is above then let's seek on the top
	if (cell.m_iRow < m_cellFocused.m_iRow) {
		int j;
		CGridCell newCell;

		j = max(cell.m_iRow, 0);

		while (j <= m_cellFocused.m_iRow) {
			newCell.m_iColumn = cell.m_iColumn;
			newCell.m_iRow = j;

			//  Try to set the cursor
			if (IsCellAcceptCursor(newCell))
				break;
			//  Next row
			++j;
		}
		//  The cell is not found
		newCell = cell;

		if (IsCellAcceptCursor(newCell))
			return newCell;
	}

	//  If selection is below then let's seek at the bottom
	if (cell.m_iRow > m_cellFocused.m_iRow) {
		int j;
		CGridCell newCell;

		j = min(cell.m_iRow, m_pRows->GetCount() - 1);

		while (j >= m_cellFocused.m_iRow)  {
			newCell.m_iColumn = cell.m_iColumn;
			newCell.m_iRow = cell.m_iRow;

			//  Try to set the cursor
			if (IsCellAcceptCursor(newCell))
				break;
			//  Previous row
			j --;
		}
		//  The cell is not found
		newCell = cell;

		if (IsCellAcceptCursor(newCell))
			return newCell;
	}

	//  Nothing changes...
	return m_cellFocused;
}


//  ---
CGridCell CVirtualGridCtrl::First()
{
	CGridCell cell;
	int i, j;

	j = 0;

	while (j <= m_pRows->GetCount() - 1)  {
		i = m_pFixed->GetCount();

		while (i <= m_pColumns->GetUpperBound())  {
			cell.m_iColumn = i;
			cell.m_iRow = j;

			//  Trying to set the cursor...
			if (IsCellAcceptCursor(cell))
				return cell;

			//  Next column
			++i;
		}

		//  Next row
		++j;
	}

	//  Default result...
	return m_cellFocused;
}


//  ---
CGridCell CVirtualGridCtrl::Next(const CGridCell& cell)
{
	CGridCell newCell;
	int i, j;

	i = cell.m_iColumn + 1;
	j = cell.m_iRow;

	//  ---
	while (j <= m_pRows->GetCount() - 1) {
		while (i <= m_pColumns->GetUpperBound())  {
			newCell.m_iColumn = i;
			newCell.m_iRow = j;
			//  Trying to set the cursor taking into account line selection
			if (IsCellAcceptCursor(newCell) && (!m_bRowSelect || newCell.m_iRow != cell.m_iRow))
				return newCell;

			//  Next column...
			++i;
		}

		//  Next row from the first column...
		i = m_pFixed->GetCount();
		++j;
	}

	//  Default result
	return m_cellFocused;
}




//
CGridCell CVirtualGridCtrl::Prev(const CGridCell& cell)
{
	CGridCell newCell;
	int i, j;

	i = cell.m_iColumn - 1;
	j = cell.m_iRow;

	//  ---
	while (j >= 0)  {
		while (i >= m_pFixed->GetCount())  {
			newCell.m_iColumn = i;
			newCell.m_iRow = j;

			//  Trying to set the cursor taking into account line selection
			if (IsCellAcceptCursor(newCell) && (!m_bRowSelect || newCell.m_iRow != cell.m_iRow))
				return newCell;

			//  Previous column...
			--i;
		}

		//  Previous row of the last column
		i = m_pColumns->GetUpperBound();
		j --;
	}

	//  Default result
	return cell;
}


//  ---
CGridCell CVirtualGridCtrl::GetEditCell()
{
	return m_editCell;
}


//  ---
int CVirtualGridCtrl::GetTextRightIndent()
{
	return m_nTextRightIndent;
}



//  ----
BOOL CVirtualGridCtrl::GetReadOnly()
{
	return m_bReadOnly;
}


//  -----------------------------------------------------------------
void CVirtualGridCtrl::SetRowsHeight(int nHeight)
{
	m_pRows->SetHeight(nHeight);
}


//  ----------------------------------------------------------------
void CVirtualGridCtrl::SetRowCount(int nCount)
{
	m_pRows->SetCount(nCount);
}


//  -----
void CVirtualGridCtrl::ValidateEdit()
{
	CRuntimeClass* pEditClass;

	pEditClass = GetEditClass(m_cellFocused);

	//  Create or change an edit control...
	if (m_pEdit == 0 || pEditClass != m_pEdit->GetRuntimeClass())  {
		if (m_pEdit)
			delete m_pEdit;
		m_pEdit = CreateEdit(pEditClass);
		m_pEdit->m_pGrid = this;
	}
}


//  ---
void CVirtualGridCtrl::DoUpdateEdit()
{
	BOOL bSuccess;

	if (!(::IsWindow(m_pEdit->m_hWnd)))  {
		bSuccess = m_pEdit->Create(
			WS_CHILD | ES_AUTOHSCROLL | ES_MULTILINE | ES_NOHIDESEL,
			GetCellRect(m_cellFocused),
			this,
			0
			);
		ASSERT(bSuccess);
		m_pEdit->SetFont(&m_font);
	}

	m_editCell = m_cellFocused;
	m_pEdit->UpdateContents();
	m_pEdit->UpdateStyle();
}



//  ---------------------------------------------------------------------------------------
int CVirtualGridCtrl::AddColumn(
				LPCTSTR lpCaption,
				int nWidth,
				int nAlignment
				)
{
	CGridColumn* pColumn;

	pColumn = m_pColumns->Add();

	pColumn->SetCaption(lpCaption);
	pColumn->SetWidth(nWidth);
	pColumn->SetAlignment(nAlignment);

	return m_pColumns->GetUpperBound();
}


//  -----------------------------------------------------------------------------------
CGridCell CVirtualGridCtrl::GetCellFocused()
{
	return m_cellFocused;
}


//  --------------------
CGridColumns* CVirtualGridCtrl::GetGridColumns()
{
	return &(*m_pColumns);
}




//  ----
void CVirtualGridCtrl::ShiftGridPoints(int nPoints, LPPOINT lpPoints, int dx, int dy)
{
	int i;

	//  First we should offset vertical lines in X direction...
	i = 0;
	while (i < m_pFixed->GetCount() * (0 < (gsVertLine & m_nGridStyles)) * 2)  {
		lpPoints[i].x += dx;
		++i;
	}

	//  Second we should offset horizontal lines in Y direction...
	while (i < nPoints)  {
		lpPoints[i].y += dy;
		++i;
	}
}



//  ------------------------------
int CVirtualGridCtrl::GetFixedCount()
{
	return m_pFixed->GetCount();
}


//  ----
void CVirtualGridCtrl::SetFixedCount(int nCount)
{
	m_pFixed->SetCount(nCount);
}


//  ---
BOOL CVirtualGridCtrl::GetAllowSelect()
{
	return m_bAllowSelect;
}


//  -----
BOOL CVirtualGridCtrl::GetRowSelect()
{
	return m_bRowSelect;
}



//  -----  Used inside the PaintFixedGrid method  ----
void CVirtualGridCtrl::PaintVert3DLines(CDC* pDC, LPRECT lpRect, BOOL bDrawBottomLine)
{
	const UINT nFlags[] = {BF_TOPLEFT | BF_RIGHT, BF_RECT};
	int i, nWidth;
	CRect rect(*lpRect);

	rect.right = rect.left;

	//  Columns...
	for (i = 0; i < m_pFixed->GetCount(); ++i) {
		nWidth = m_pColumns->GetAt(i)->GetWidth();
		if (nWidth > 0)  {
			rect.left = rect.right;
			rect.right += nWidth;

			if (pDC->RectVisible(rect))
				Paint3DFrame(pDC, rect, nFlags[bDrawBottomLine]);
		}
	}
}



//  ------  Used inside the PaintFixedGrid method  ------
void CVirtualGridCtrl::PaintBottom3DMargin(CDC* pDC, LPRECT lpRect)
{
	if (pDC->RectVisible(lpRect))
		Paint3DFrame(pDC, lpRect, BF_LEFT | BF_TOP | BF_RIGHT);
}


//  ------  Used inside the PaintFixedGrid method  ------
void CVirtualGridCtrl::PaintHorz3DLines(CDC* pDC, LPRECT lpRect)
{
	CRect rect(*lpRect);

	rect.bottom = rect.top;
	do  {
		rect.top = rect.bottom;
		rect.bottom += m_pRows->m_nHeight;

		if (pDC->RectVisible(rect))
			Paint3DFrame(pDC, rect, BF_RECT);
	}  while (rect.bottom < lpRect->bottom);
}



//  ----
void CVirtualGridCtrl::SetRowHeight(int nHeight)
{
	m_pRows->SetHeight(nHeight);
}



//  ----
int CVirtualGridCtrl::GetRowCount()
{
	return m_pRows->GetCount();
}



//  ----
BOOL CVirtualGridCtrl::GetCancelOnExit()
{
	return m_bCancelOnExit;
}



//  ---
void CVirtualGridCtrl::SetCancelOnExit(BOOL bCancel)
{
	m_bCancelOnExit = bCancel;
}


//  ----
BOOL CVirtualGridCtrl::GetAllowEdit()
{
	return m_bAllowEdit;
}


//  ----
BOOL CVirtualGridCtrl::GetAlwaysEdit()
{
	return m_bAlwaysEdit;
}


//  ---
BOOL CVirtualGridCtrl::GetAlwaysSelected()
{
	return m_bAlwaysSelected;
}


//  ----
GridCheckStyle CVirtualGridCtrl::GetCheckStyle()
{
	return m_checkStyle;
}



//  ---
int CVirtualGridCtrl::GetFontHeight(CFont* pFont)
{
	int nHeight, nRetVal;

	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	CDC* pDC = pWnd->GetDC();
	nHeight = pDC->GetTextExtent(_T("^j")).cy;

	nRetVal = pWnd->ReleaseDC(pDC);
	ASSERT(nRetVal);
	
	return nHeight;
}



//  -----
BOOL CVirtualGridCtrl::GetDoubleBuffered()
{
	return m_bDoubleBuffered;
}


//  ----
void CVirtualGridCtrl::SetDoubleBuffered(BOOL bDoubleBuffered)
{
	if (m_bDoubleBuffered == bDoubleBuffered)
		return;		//  Nothing to change

	m_bDoubleBuffered = bDoubleBuffered;
	Invalidate();
}





void CVirtualGridCtrl::GridKeyDown(UINT &nChar, UINT nFlags)
{
	NMKEY dispInfo;
	CWnd* pWnd = GetParent();
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));

	dispInfo.hdr.code = NM_KEYDOWN;
	dispInfo.hdr.hwndFrom = m_hWnd;
	dispInfo.hdr.idFrom = GetDlgCtrlID();

	dispInfo.nVKey = nChar;
	dispInfo.uFlags = nFlags;

	if (0 == pWnd->SendMessage(WM_NOTIFY, dispInfo.hdr.idFrom, (LPARAM)&dispInfo))
		nChar = dispInfo.nVKey = nChar;
}





//
void CVirtualGridCtrl::KeyDown(UINT& nChar, UINT nFlags)
{
	CGridCell cell;
	GridCursorOffset homeOffsets[] = { goHome, goGridHome };
	GridCursorOffset endOffsets[] = { goEnd, goGridEnd };
	GridCursorOffset tabOffsets[] = { goNext, goPrev };


	//  --- First off, we have to notify parent. ---
	GridKeyDown(nChar, nFlags);

	//
	if (gkArrows & m_nCursorKeys) {
		switch (nChar)  {
		case VK_LEFT:
			SetCursor(GetCursorCell(m_cellFocused, goLeft), TRUE, TRUE);
			// Scroll grid to the left if "Row select" mode is on.
			if (m_bRowSelect)
				m_pHorzScrollBar->SetPosition(m_pHorzScrollBar->GetPosition() - m_pHorzScrollBar->GetLineStep());
			break;
		case VK_RIGHT:
			SetCursor(GetCursorCell(m_cellFocused, goRight), TRUE, TRUE);
			// Scroll grid to the right if "Row select" mode is on.
			if (m_bRowSelect)
				m_pHorzScrollBar->SetPosition(m_pHorzScrollBar->GetPosition() + m_pHorzScrollBar->GetLineStep());
			break;
		case VK_UP:
			if (!m_bAllowSelect)
				cell = m_visOrigin;
			else
				cell = m_cellFocused;
			//  Change selection
			SetCursor(GetCursorCell(cell, goUp), TRUE, TRUE);
			break;
		case VK_DOWN:
			//  If there's no focus then let's shift all the table...
			if (!m_bAllowSelect)  {
				cell.m_iColumn = m_visOrigin.m_iColumn;
				cell.m_iRow = m_visOrigin.m_iRow + m_visSize.m_iRow - 1;
				if (!IsCellVisible(cell, FALSE))
					--cell.m_iRow;
			}  else  {
				cell = m_cellFocused;
			}

			//  Change selection
			SetCursor(GetCursorCell(cell, goDown), TRUE, TRUE);
			break;
		case VK_PRIOR:
			SetCursor(GetCursorCell(m_cellFocused, goPageUp), TRUE, TRUE);
			break;
		case VK_NEXT:
			SetCursor(GetCursorCell(m_cellFocused, goPageDown), TRUE, TRUE);
			break;
		case VK_HOME:
			cell = GetCursorCell(m_cellFocused, homeOffsets[IS_CTRL_PRESSED() || m_bRowSelect]);
			SetCursor(cell, TRUE, TRUE);
			break;
		case VK_END:
			cell = GetCursorCell(m_cellFocused, endOffsets[IS_CTRL_PRESSED() || m_bRowSelect]);
			SetCursor(cell, TRUE, TRUE);
			break;
		}
	}

	if ((gkTabs & m_nCursorKeys) && (nChar == VK_TAB))
		SetCursor(GetCursorCell(m_cellFocused, tabOffsets[IS_SHIFT_PRESSED()]), TRUE, TRUE);
}




//
void CVirtualGridCtrl::KeyPress(UINT& nChar)
{
	//  The <Enter> key pressed?  OK, Let's show in-cell edit.
	if (nChar == VK_RETURN) {
		nChar = 0;

		//  Is editing take place?
		if (GetEditing()) {
			//  Insert the text, switch off in-cell edit.
			ApplyEdit();

			//  Set current selection to the next available cell.
			if (m_nCursorKeys & gkReturn)
				SetCursor(GetCursorCell(m_cellFocused, goNext), TRUE, TRUE);
		} else {
			//  No in-cell editing?  Create it.
			if (!GetAlwaysEdit()) {
				SetCursor(m_cellFocused, TRUE, TRUE);

				//  Show in-cell edit.
				SetEditing(TRUE);
			}
		}
	} else if (nChar == VK_ESCAPE) {
		//  Switch off in-cell edit.
		nChar = 0;

		//  Check editing.
		if (GetEditing()) {
			if (!GetAlwaysEdit())
				CancelEdit();
			else
				UndoEdit();
		}
	}
}






//  =======================================================================================
//  =======================================================================================
BEGIN_MESSAGE_MAP(CVirtualGridCtrl, CWnd)
	//{{AFX_MSG_MAP(CVirtualGridCtrl)
	ON_WM_PAINT()
	ON_WM_GETDLGCODE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SYSKEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CVirtualGridCtrl message handlers



//  --------------------------------------------------------------------------------------
void CVirtualGridCtrl::OnPaint()
{
	CPaintDC dc(this);	// device context for painting

	if (m_bDoubleBuffered)  {
		//  Use a memory DC to remove flicker.
		CMemDC1 memDC1(&dc);
		Paint(memDC1);
	}  else  {
		//  "Classical" painting
		Paint(&dc);
	}
}




//  ------------------------------------------------------------------------------
BOOL CVirtualGridCtrl::OnEraseBkgnd(CDC* pDC) 
{
	//  Don't!
	return TRUE;
}




//
UINT CVirtualGridCtrl::OnGetDlgCode() 
{
	return DLGC_WANTALLKEYS;
}




//
void CVirtualGridCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

	if (m_pRows->GetCount())  {
		InvalidateFocus();
		if (m_pEdit && pNewWnd != m_pEdit)
			HideCursor();
	}
}


//  --------------------------------------------------------------------
void CVirtualGridCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);

	if (m_pRows->m_nCount > 0)  {
		InvalidateFocus();
		if (m_pEdit == 0)	//  NYI
			ShowCursor();
	}
}


//  ----------------------------------------------------------------------
void CVirtualGridCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	BOOL bSuccess;
	CGridHeaderSection* pSection;
	CGridCell cell;

	m_titleTip.Hide();

	if (!AcquireFocus())  {
		bSuccess = ReleaseCapture();	//  ??????
		return;
	}

	//  If the header is visible and we hit it...
	if (m_bShowHeader && GetHeaderRect().PtInRect(point))  {
		//  Did we hit the resize area?
		pSection = GetResizeSectionAt(point);

		if (pSection)  {
			StartColResize(pSection, point.x, point.y);
		}  else  {
			pSection = GetSectionAt(point);

			if (pSection)  {
				//  Check 3D-effect
				if (m_pHeader->m_bFlat)
					HeaderClick(pSection);
				else
					StartHeaderClick(pSection, point);
			}
		}

		return;
	}

	//  Check newly selected cell
	if ((gkMouse & m_nCursorKeys) && GetGridRect().PtInRect(point))  {
		cell = GetCellAt(point.x, point.y);

		m_clickPos.m_iColumn = -1;
		m_clickPos.m_iRow = -1;

		if (cell.IsEmpty())  {
			SetEditing(FALSE);
			SetCursor(m_cellFocused, FALSE, FALSE);
		}  else  {
			SetCursor(cell, TRUE, TRUE);
			CellClick(cell, nFlags, point.x, point.y);
		}

		if (GetCheckRect(cell).PtInRect(point))  {
			CheckClick(cell);
			return;
		}

		m_clickPos = cell;
	}

	CWnd::OnLButtonDown(nFlags, point);
}



//  -----------------------------------------------------------------------
void CVirtualGridCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	//  If column resizing take place...
	if (m_bColResizing)  {
		StopColResize(FALSE);
		return;
	}

	//  If header clicking take place...
	if (m_bHeaderClicking)  {
		StopHeaderClick(FALSE);
		return;
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}



//  -----------------------------------------------------------------------
void CVirtualGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	BOOL bSuccess;
	CGridHeaderSection* pSection;

	//  Set the focus on itself
	if (!AcquireFocus())  {
		bSuccess = ReleaseCapture();
		return;
	}


	//  If the grid's header is visible and we hit it...
	if (m_bShowHeader && GetHeaderRect().PtInRect(point))  {
		//  If we hit header section border
		pSection = GetResizeSectionAt(point);
		if (pSection)  {
			int nIndex;
			int nWidth;

			//  Let's set the column width according to max text width
			nIndex = pSection->GetResizeColumnIndex();

			if (nIndex < m_pColumns->GetSize())  {
				nWidth = min((*m_pColumns)[nIndex]->GetMaxWidth(), GetColumnMaxWidth(nIndex));
				ColumnAutoSize(nIndex, nWidth);
				ColumnResize(nIndex, nWidth);
				m_bColResizing = TRUE;
				try {
					(*m_pColumns)[nIndex]->SetWidth(nWidth);
				}
				catch (...) {
					m_bColResizing = FALSE;
				}

				m_bColResizing = FALSE;
			}
		}

		//  End header click handling
		return;
	}

	//  Check newly selected cell...
	if ((gkMouse & m_nCursorKeys) && (GetGridRect().PtInRect(point)))  {
		CGridCell cell, prevCell;

		cell = GetCellAt(point.x, point.y);
		prevCell = m_clickPos;
		m_clickPos.m_iColumn = -1;
		m_clickPos.m_iRow = -1;

		if (cell.IsEmpty())  {
			SetEditing(FALSE);
			SetCursor(m_cellFocused, FALSE, FALSE);
		}  else  {
			//  We hit the cell
			SetCursor(cell, TRUE, TRUE);
			CellClick(cell, nFlags, point.x, point.y);
		}

		//  Check flag hitting...
		if (GetCheckRect(cell).PtInRect(point))  {
			CheckClick(cell);
			return;
		}

		//  Check beginning of editing
		if (cell == m_cellFocused && m_bAllowEdit)  {
			SetEditing();
			if (GetEditing())
				return;
		}

		//  Don't forget last click's position
		m_clickPos = cell;
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}





//
void CVirtualGridCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//  Show edit control if possible
//	if (m_bAllowEdit && isalnum(nChar)) {//Vad 22_05_06 cange on
	if (m_bAllowEdit && nChar >= 32 && nChar <= 255) {//this
		ShowEditChar(nChar);
		return;
	}

	KeyPress(nChar);
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}



//  ----------------------------------------------------------------------
void CVirtualGridCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	UpdateScrollBars();
	UpdateVisOriginSize();
	UpdateEdit(GetEditing());
	Invalidate();
}





// --------  WM_HSCROLL handler.  --------
void CVirtualGridCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == 0)
		m_pHorzScrollBar->ScrollMessage(nSBCode, nPos);
	else
		CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}



// ---------  WM_VSCROLL handler.  -------
void CVirtualGridCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == 0)
		m_pVertScrollBar->ScrollMessage(nSBCode, nPos);
	else
		CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}




//
LRESULT CVirtualGridCtrl::OnNcHitTest(CPoint point) 
{
	m_hitTest = point;
	ScreenToClient(&m_hitTest);

	return CWnd::OnNcHitTest(point);
}




//
BOOL CVirtualGridCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	HCURSOR hCursor;

	if (m_bColResizing)  {
		hCursor = ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		return TRUE;
	}

	//  --
	if (nHitTest == HTCLIENT && m_bShowHeader)  {
		CRect rect(GetHeaderRect());

		if (rect.PtInRect(m_hitTest) && GetResizeSectionAt(m_hitTest))  {
			hCursor = ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
			return TRUE;
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}



//  ----------------------------------------------------------------------------
void CVirtualGridCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	//  Is there columns resizing happening?
	if (m_bColResizing)  {
		//  Just go on
		StepColResize(point.x, point.y);
		CWnd::OnMouseMove(nFlags, point);

		//  ...And nothing more
		return;
	}

	//  Is someone click at the header?
	if (m_bHeaderClicking)  {
		//  Go on
		StepHeaderClick(point);
		CWnd::OnMouseMove(nFlags, point);

		//  ...That's enough
		return;
	}

	//  Title Tips handling
	if (m_bTitleTips)  {
		CRect cellRect;
		TCHAR szCellText[MAX_CELL_TEXT];
//		CGridHeaderSection* pSection;
		CGridCell cell;
		int nRetVal;

//		pSection = GetSectionAt(point);
		cell = GetCellAt(point.x, point.y);

		//  The mouse is above header?
/*
		if (pSection)  {
			strCellText = pSection->GetCaption();
			if (strCellText.GetLength())
				cellRect = pSection->GetBoundsRect();
		}  else 
*/
			
		if (!cell.IsEmpty())  {
			int nAlign;

			GetCellText(szCellText, cell, nAlign);
			if (lstrlen(szCellText))
				cellRect = GetCellRect(cell);
		}

		if (lstrlen(szCellText)) {
			LOGFONT lf;

			nRetVal = m_font.GetLogFont(&lf);
			ASSERT(nRetVal);

			CRect hoverRect(cellRect);
			hoverRect.right += 2;

			cellRect.right -= m_nTextLeftIndent;

//			if ((pSection && !pSection->m_bWordWrap) || !pSection)  {
				TRACE0("Showing title tip\n");
				m_titleTip.Show(cellRect, szCellText, 0, hoverRect, &lf);
//			}
		}

//		ASSERT(nRetVal);	???
	}

	CWnd::OnMouseMove(nFlags, point);
}





//
void CVirtualGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	KeyDown(nChar, nFlags);
}



//
void CVirtualGridCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_titleTip.Hide();
	CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}



//
void CVirtualGridCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_titleTip.Hide();
	CWnd::OnRButtonDown(nFlags, point);
}



//
void CVirtualGridCtrl::PaintSortImage(CDC* pDC, LPPOINT lpPoint, GridSortDirection gsd)
{
	BOOL bSuccess;
	int nOffset = 8;
	CPen* pOldPen;

	if (gsd == gsNone)
		return;		//  Nothing to paint.

	CPen penHilight(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
	CPen penShadow(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));

	pOldPen = pDC->SelectObject(&penHilight);

	switch (gsd)  {
		case gsAscending:
			//  Draw triangle pointing upwards (in clockwise direction).
			pDC->MoveTo(lpPoint->x + nOffset / 2, lpPoint->y);

			bSuccess = pDC->LineTo(lpPoint->x + nOffset, lpPoint->y + nOffset + 1);
			ASSERT(bSuccess);

			bSuccess = pDC->LineTo(lpPoint->x, lpPoint->y + nOffset + 1);
			ASSERT(bSuccess);

			pDC->SelectObject(&penShadow);
			bSuccess = pDC->LineTo(lpPoint->x + nOffset / 2, lpPoint->y);
			ASSERT(bSuccess);

			break;
		case gsDescending:
			//  Draw triangle pointing downwards (in clockwise direction).
			pDC->MoveTo(lpPoint->x + nOffset, lpPoint->y);

			bSuccess = pDC->LineTo(lpPoint->x + nOffset / 2, lpPoint->y + nOffset + 1);
			ASSERT(bSuccess);

			pDC->SelectObject(&penShadow);

			bSuccess = pDC->LineTo(lpPoint->x, lpPoint->y);
			ASSERT(bSuccess);

			bSuccess = pDC->LineTo(lpPoint->x + nOffset + 1, lpPoint->y);

			break;
	}

	pOldPen = pDC->SelectObject(pOldPen);
}




BOOL CVirtualGridCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	GridCursorOffset gco = (zDelta < 0) ? goDown : goUp;

	if (0 != (m_nCursorKeys & gkMouseWheel)) {
		SetCursor(GetCursorCell(GetCellFocused(), gco), true, true);
		InvalidateGrid();	//  !!!!!!
	}

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  CGridHeaderSection  ////////////////////////////////

//  =============================================================================
CGridHeaderSection::CGridHeaderSection(CGridHeaderSections* pParentSections) : 	m_pParentSections(pParentSections),
										m_nWidth(64),
										m_nAlignment(LVCFMT_LEFT),
										m_bWordWrap(FALSE)
{
	m_sections.m_pHeader = m_pParentSections->m_pHeader;
	m_sections.m_pOwnerSection = this;
}


//  ==============  CGridHeaderSection Copy Constructor  =================
CGridHeaderSection::CGridHeaderSection(const CGridHeaderSection &otherSection)
{
	m_boundsRect = otherSection.m_boundsRect;
	m_bWordWrap = otherSection.m_bWordWrap;
	m_nAlignment = otherSection.m_nAlignment;
	m_iColumn = otherSection.m_iColumn;
	m_nWidth = otherSection.m_nWidth;
	m_pParentSections = otherSection.m_pParentSections;
	m_sections = otherSection.m_sections;
	m_sections.m_pHeader = m_pParentSections->m_pHeader;
	m_strCaption = otherSection.m_strCaption;
}


//  ======================================================================
CGridHeaderSection::~CGridHeaderSection()
{
	for (INT_PTR i = 0; i < m_sections.m_sections.GetSize(); ++i)
		delete m_sections.m_sections[i];

	m_sections.m_sections.RemoveAll();
}



//  ---------------------------------------------------------------------
CGridHeaderSection& CGridHeaderSection::operator=(const CGridHeaderSection& otherSection)
{
	m_sections = otherSection.m_sections;
	m_strCaption = otherSection.m_strCaption;
	m_nWidth = otherSection.m_nWidth;
	m_nAlignment = otherSection.m_nAlignment;
	m_bWordWrap = otherSection.m_bWordWrap;

	return *this;
}


//  ----
BOOL CGridHeaderSection::IsSectionsStored()
{
	return m_sections.GetSize();
}


//  --
BOOL CGridHeaderSection::IsWidthStored()
{
	return (0 == m_sections.GetSize() && m_nWidth != 64);
}


//  --
BOOL CGridHeaderSection::GetAllowClick()
{
	int nIndex;

	//  Can we click at the column?
	if (GetHeader() && GetHeader()->GetGrid())  {
		nIndex = m_iColumn;

		if (nIndex < GetHeader()->GetGrid()->GetColumns()->GetSize())
			return GetHeader()->GetGrid()->GetColumns()->GetAt(nIndex)->m_bAllowClick;
	}

	return FALSE;
}


//  --
CRect CGridHeaderSection::GetBoundsRect()
{
	CRect rect(0, 0, 0, 0);

	//  No header - no dimensions...
	if (0 == GetHeader() || 0 == GetHeader()->GetGrid())
		return rect;

	//  Absolute borders
	rect = m_boundsRect;

	//  If the header is not FIXED then let's offset it...
	if (!GetFixedColumn())
		rect.OffsetRect(GetHeader()->GetGrid()->GetGridOrigin().x, 0);

	return rect;
}



//  --
int CGridHeaderSection::GetFirstColumnIndex()
{
	if (m_sections.GetSize())
		return m_sections.GetSection(0)->GetFirstColumnIndex();

	return m_iColumn;
}



//  --
BOOL CGridHeaderSection::GetFixedColumn()
{
	if (m_sections.GetSize())
		return m_sections.GetSection(0)->GetFixedColumn();

	if (0 == GetHeader() || 0 == GetHeader()->GetGrid())
		return FALSE;

	return m_iColumn < GetHeader()->GetGrid()->GetFixed()->GetCount();
}



//  --
CGridHeader* CGridHeaderSection::GetHeader()
{
	if (GetParentSections())
		return GetParentSections()->m_pHeader;

	return 0;
}



//  --
int CGridHeaderSection::GetLevel()
{
	if (GetParent())
		return GetParent()->GetLevel() + 1;

	return 0;
}


//  --
CGridHeaderSection* CGridHeaderSection::GetParent()
{
	if (GetParentSections())
		return GetParentSections()->m_pOwnerSection;

	return 0;
}



//  --
CGridHeaderSections* CGridHeaderSection::GetParentSections()
{
	return m_pParentSections;
}


//  --
int CGridHeaderSection::GetResizeColumnIndex()
{
	int i;

	//  If there's subheaders return the column of last of them
	for (i = m_sections.GetUpperBound(); i >= 0; i --)  {
		if (m_sections[i]->GetVisible())
			return m_sections[i]->GetResizeColumnIndex();

	}

	return m_iColumn;
}




//  --
CGridHeaderSections* CGridHeaderSection::GetSections()
{
	return &m_sections;
}



//  --
BOOL CGridHeaderSection::GetVisible()
{
	int i;

	//  If there's subheaders, let's see their visibility
	if (m_sections.GetSize())  {
		for (i = 0; i < m_sections.GetSize(); ++i) {
			if (m_sections[i]->GetVisible())
				return TRUE;
		}
	}

	//  Otherwise let's see column's visibility
	if (GetHeader() && GetHeader()->GetGrid())  {
		if (m_iColumn < GetHeader()->GetGrid()->GetColumns()->GetSize())
			return GetHeader()->GetGrid()->GetColumns()->GetColumn(m_iColumn)->m_bVisible;
	}

	//  No column - the section is visible
	return TRUE;
}



//  --
int CGridHeaderSection::GetWidth()
{
	int nWidth;
	int i;
	CVirtualGridCtrl* pGrid=0;

	//  If there's subheaders then the width equals to the sum of
	//  the subheaders widths
	if (m_sections.GetSize())  {
		nWidth = 0;

		for (i = 0; i < m_sections.GetSize(); ++i)
			nWidth += m_sections[i]->GetWidth();

		return nWidth;
	}

	//  Otherwise return the width of appropriate column
	if (GetHeader())
		pGrid = GetHeader()->GetGrid();

	if (pGrid)  {
		if (m_iColumn < pGrid->GetColumns()->GetSize())
			return pGrid->GetColumns()->GetColumn(m_iColumn)->GetWidth();
	}

	//  No column - its own width
	return m_nWidth;
}


//  ----
void CGridHeaderSection::SetAlignment(int nAlignment)
{
	if (m_nAlignment == nAlignment)
		return;		//  Nothing to change

	m_nAlignment = nAlignment;
	Changed(FALSE);
}


//  ---
void CGridHeaderSection::SetCaption(LPCTSTR lpCaption)
{
	if (!m_strCaption.Compare(lpCaption))
		return;		//  Nothing to change

	m_strCaption = lpCaption;
	Changed(FALSE);
}


//  ---
void CGridHeaderSection::SetSections(const CGridHeaderSections& sections)
{
	m_sections = sections;
}



//  ---
void CGridHeaderSection::SetWidth(int nWidth)
{
	CVirtualGridCtrl* pGrid = 0;

	if (nWidth < 0 || m_nWidth == nWidth)
		return;		//  Nothing to change

	if (GetHeader())
		pGrid = GetHeader()->GetGrid();

	if (pGrid)  {
		if (m_iColumn > pGrid->GetColumns()->GetUpperBound())  {
			if (m_sections.GetSize())  {
				CGridHeaderSection* pSection;

				pSection = m_sections[m_sections.GetUpperBound()];
				pSection->SetWidth(pSection->m_nWidth + (nWidth - m_nWidth));

				return;
			}
		}
	}

	m_nWidth = nWidth;
	Changed(FALSE);
}



//  -------------------------------------------------------------------------
void CGridHeaderSection::SetWordWrap(BOOL bWordWrap)
{
	if (m_bWordWrap == bWordWrap)
		return;		//  Nothing to change
	
	m_bWordWrap = bWordWrap;
	Changed(FALSE);
}



//  -------------------------------------------------------------------------
int CGridHeaderSection::GetAlignment()
{
	return m_nAlignment;
}


//  ------------------------------------------------------------------------
BOOL CGridHeaderSection::GetWordWrap()
{
	return m_bWordWrap;
}






/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////  CGridHeaderSections  ///////////////////////////////


//
CGridHeaderSections::CGridHeaderSections() : m_pHeader(0),
					m_pOwnerSection(0)
{
}



//
CGridHeaderSections::~CGridHeaderSections()
{
	//  Thank you, mr. bluescreen.
	INT_PTR i = m_sections.GetSize();
	for (; i; i--)
		delete m_sections[i - 1];
	m_sections.RemoveAll();
}


//  --
CGridHeaderSection* CGridHeaderSections::Add()
{
	INT_PTR nIndex = m_sections.Add(new CGridHeaderSection(this));

	if (m_pHeader && m_pHeader->GetGrid())
		Update(m_sections[nIndex]);

	return m_sections[nIndex];
}


//  --
int CGridHeaderSections::GetMaxColumn()
{
	if (m_sections.GetSize())
		return m_sections[m_sections.GetUpperBound()]->m_iColumn;

	return 0;
}



//  --
int CGridHeaderSections::GetMaxLevel()
{
	int nMaxLevel = 0;

	for (INT_PTR i = 0; i < m_sections.GetSize(); ++i) {
		CGridHeaderSection* pSection = m_sections.GetAt(i);
		if (nMaxLevel < pSection->GetLevel())
			nMaxLevel = pSection->GetLevel();

		nMaxLevel = max(nMaxLevel, pSection->m_sections.GetMaxLevel());
	}

	return nMaxLevel;
}


//  --
CGridHeaderSection* CGridHeaderSections::GetSection(int nIndex)
{
	return m_sections.GetAt(nIndex);
}



//  --
CGridHeaderSection* CGridHeaderSections::GetSection(int nColumnIndex, int nLevel)
{
	for (INT_PTR i = 0; i < m_sections.GetSize(); ++i) {
		CGridHeaderSection* pSection = m_sections[i];

		int l = pSection->GetLevel();
		
		//  Compare the column and the level
		if ((pSection->m_iColumn >= nColumnIndex)
			&& ((nLevel == -1) && (0 == pSection->m_sections.GetSize())
			|| (l == nLevel)))

			//  We have found it!!!
			return pSection;

		//  Recurse...
		pSection = pSection->m_sections.GetSection(nColumnIndex, nLevel);
		if (pSection)
			return pSection;
	}

	//  No such a section
	return 0;
}



//  ---
void CGridHeaderSections::SetSection(int nIndex, CGridHeaderSection* pSection)
{
	CVirtualGridCtrl* pGrid;

	pGrid = m_pHeader->GetGrid();

	
	delete m_sections[nIndex];
	m_sections.RemoveAt(nIndex);

	m_sections[nIndex] = pSection;
}


//  --
CGridHeader* CGridHeaderSections::GetOwner()
{
	return m_pHeader;
}


//  ---
void CGridHeaderSections::Update(CGridHeaderSection* pSection)
{
	if (m_pHeader)
		m_pHeader->Change();
}





//  ----
BOOL CGridHeaderSections::FindResizeSection(CPoint point, CGridHeaderSection*& pRetrievedSection)
{
	int dl, dr;
	CRect rect;
	CGridHeaderSection* pSection;
	CGridColumns* pColumns;

	pColumns = m_pHeader->GetGrid()->GetColumns();

	for (INT_PTR i = m_sections.GetUpperBound(); i >= 0; --i) {
		//  Get the cell and its column
		pSection = m_sections[i];

		//  We should search only for visible columns
		if (!pSection->GetVisible())
			continue;

		int iColumn = pSection->GetResizeColumnIndex();

		//  Get the rectangle of size changing area  ???
		rect = pSection->GetBoundsRect();

		//  Find out hit errors
		dl = 7;
		if (rect.Width() < 20)
			dl = 3;
		if (rect.Width() < 10)
			dl = 1;

		dr = 5;

		if (iColumn < m_pHeader->GetGrid()->GetColumns()->GetUpperBound())  {
			if (pColumns->GetColumn(iColumn + 1)->m_nWidth < 20)
				dr = 3;
			if (pColumns->GetColumn(iColumn + 1)->m_nWidth < 10)
				dr = 1;
		}

		//  Adjust target rect
		if (rect.Width() > 0)
			rect.left = rect.right - dl;
		rect.right += dr;

		//  We have got in the target?
		if (rect.PtInRect(point))  {
			//  Check the column on the fixed size
			if (iColumn < pColumns->GetSize() && (pColumns->GetColumn(iColumn)->m_bFixedSize
				|| !m_pHeader->GetGrid()->ColumnsSizing()))  {

				pRetrievedSection = 0;
				return FALSE;
			}  else  {
				pRetrievedSection = pSection;
				return TRUE;
			}
		}

		//  Search for the section in subheadings
		if (pSection->GetSections()->FindResizeSection(point, pRetrievedSection))
			return TRUE;
	}

	//  Nothing have found
	return FALSE;
}




//  
BOOL CGridHeaderSections::FindSection(CPoint point, CGridHeaderSection*& pRetrievedSection)
{
	int i;
	CGridHeaderSection* pSection;
	CRect rect;

	for (i = 0; i < m_sections.GetSize(); ++i) {
		pSection = m_sections[i];

		if (pSection->GetVisible())  {
			if (pSection->GetBoundsRect().PtInRect(point))  {
				pRetrievedSection = pSection;
				return TRUE;
			}
		}

		if (pSection->m_sections.FindSection(point, pRetrievedSection))
			return TRUE;
	}

	pRetrievedSection = 0;
	return FALSE;
}



//  ---
void CGridHeaderSections::UpdateColumnIndex(int& iColumn)
{
	for (INT_PTR i = 0; i < m_sections.GetSize(); ++i) {
		CGridHeaderSection* pSection = m_sections[i];

		//  Is there subheaders?
		if (0 == pSection->m_sections.GetSize())  {
			//  This is the lowermost section
			pSection->m_iColumn = iColumn;
			++iColumn;
		}  else  {
			//  Recurse...
			pSection->m_sections.UpdateColumnIndex(iColumn);
			//  The index is the last's index
			pSection->m_iColumn = pSection->m_sections.GetSection(pSection->m_sections.GetUpperBound())->m_iColumn;
		}
	}
}



//
void CGridHeaderSections::UpdateSectionsBounds(LPRECT lpRect)
{
	CGridHeaderSection* pSection;
	CRect rect(*lpRect);
	CRect sectionRect;

	rect.right = rect.left;

	//  Let's sort subheaders
	for (INT_PTR i = 0; i < m_sections.GetSize(); ++i) {
		pSection = m_sections[i];

		rect.left = rect.right;
		rect.right = rect.left + pSection->GetWidth();

		//  Rectangle
		sectionRect = rect;

		if (pSection->m_sections.GetSize())
			sectionRect.bottom = rect.top + m_pHeader->m_nSectionHeight;
		//  Store...
		pSection->m_boundsRect = sectionRect;

		//  Subheaders
		if (pSection->m_sections.GetSize())  {
			//  Substact upper-level line
			sectionRect.top = sectionRect.bottom;
			sectionRect.bottom = rect.bottom;
			
			//  Bottom-level subheaders
			pSection->m_sections.UpdateSectionsBounds(sectionRect);
		}
	}
}




/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  CGridHeader  //////////////////////////////////


//  =============================================================================
CGridHeader::CGridHeader(CVirtualGridCtrl* pGrid) : m_pGrid(pGrid),
						m_nSectionHeight(17),
						m_bSynchronized(TRUE),
						m_bAutoSynchronize(TRUE),
						m_bFullSynchronizing(FALSE),
						m_bGridFont(TRUE),
						m_bFlat(TRUE),
						m_bGridColor(FALSE),
						m_bAutoHeight(FALSE),
						m_pImageList(0)
{
	m_color = ::GetSysColor(COLOR_BTNFACE);
	m_sections.m_pHeader = this;
	m_sections.m_pOwnerSection = 0;
}



//
void CGridHeader::SynchronizeSections()
{
	int iColumn;

	//  We should refresh sections's inner parameters
	UpdateSections();

	//  Let's synchronize...
	if (m_pGrid == 0)
		return;		//  Nothing to synchronize


	if (0 == m_sections.GetSize())  {
		CRect rect;
		CGridHeaderSection* pSection;
		CGridColumn* pColumn;

		//  Absolute header borders...
		m_pGrid->GetClientRect(rect);
		rect.left = m_pGrid->GetColumnRect(0).left;
		rect.right = rect.left;
		rect.bottom = rect.top + GetHeight();


		//  Let's add
		iColumn = 0;
		while (iColumn < m_pGrid->GetColumns()->GetSize())  {
			pColumn = m_pGrid->GetColumn(iColumn);

			//  Section rectangle
			rect.left = rect.right;
			rect.right = rect.left + pColumn->GetWidth();

			//  Add the section
			pSection = m_sections.Add();
			
			pSection->m_iColumn = iColumn;
			pSection->m_boundsRect = rect;
			pSection->m_nWidth = pColumn->GetWidth();
			pSection->m_strCaption = pColumn->GetCaption();
			pSection->m_nAlignment = pColumn->GetAlignment();

			//  The next column
			++iColumn;
		}

		return;
	}

	iColumn = m_sections.GetSection(m_sections.GetUpperBound())->m_iColumn;

	if (iColumn < m_pGrid->GetColumns()->GetUpperBound())  {
		++iColumn;	//  !!!!
		CRect rect;
		CGridHeaderSection* pSection;
		CGridColumn* pColumn;

		//  Absolute header borders...
		m_pGrid->GetClientRect(rect);
		rect.left = m_pGrid->GetColumnRect(iColumn).left;
		rect.right = rect.left;
		rect.bottom = rect.top + GetHeight();

		//  Let's add
		while (iColumn < m_pGrid->GetColumns()->GetSize())  {
			pColumn = m_pGrid->GetColumns()->GetColumn(iColumn);

			//  Section rectangle
			rect.left = rect.right;
			rect.right = rect.left + pColumn->GetWidth();

			//  Add the section
			pSection = m_sections.Add();
			
			pSection->m_iColumn = iColumn;
			pSection->m_boundsRect = rect;
			pSection->m_nWidth = pColumn->GetWidth();

			//  The next column
			++iColumn;
		}

		--iColumn;
	} else if (iColumn > m_pGrid->GetColumns()->GetUpperBound()) {
		m_sections.DeleteSections();
	}

	//  Lowermost section must be synchronized by headline, justification and width
	m_sections.Synchronize(iColumn);
}




//  --
void CGridHeader::UpdateSections()
{
	CRect rect;
	int iColumn = 0;

	if (m_pGrid == 0)
		return;		//  Nothing to update

	//  Calculate column indices
	m_sections.UpdateColumnIndex(iColumn);

	//  Header's absolute borders
	m_pGrid->GetClientRect(rect);
	rect.right = rect.left + m_pGrid->GetColumnsWidth(0, m_pGrid->GetColumns()->GetUpperBound());
	rect.bottom = rect.top + GetHeight();

	//  Calculate section's borders
	m_sections.UpdateSectionsBounds(rect);
}




//  --
BOOL CGridHeader::IsColorStored()
{
	return !m_bGridColor;
}


//  --
BOOL CGridHeader::IsFontStored()
{
	return !m_bGridFont;
}


//  --
BOOL CGridHeader::IsSectionsStored()
{
	return !(0 == GetMaxLevel() && m_bFullSynchronizing && m_bSynchronized);
}



//  --
int CGridHeader::GetHeight()
{
	return (GetMaxLevel() + 1) * m_nSectionHeight;
}


//  --
int CGridHeader::GetMaxColumn()
{
	return m_sections.GetMaxColumn();
}


//  --
int CGridHeader::GetMaxLevel()
{
	return m_sections.GetMaxLevel();
}


//  --
int CGridHeader::GetWidth()
{
	int nWidth = 0;
	int i;

	for (i = 0; i < m_sections.GetSize(); ++i)
		nWidth += m_sections.GetSection(i)->GetWidth();

	return nWidth;
}


//  ---
void CGridHeader::SetAutoHeight(BOOL bAutoHeight)
{
	if (m_bAutoHeight == bAutoHeight)
		return;		//  Nothing to change

	m_bAutoHeight = bAutoHeight;
	if (bAutoHeight)
		SetSectionHeight(m_nSectionHeight);
}


//  ---
void CGridHeader::SetAutoSynchronize(BOOL bAutoSynchronize)
{
	if (m_bAutoSynchronize == bAutoSynchronize)
		return;		//  Nothing to change

	m_bAutoSynchronize = bAutoSynchronize;
	if (bAutoSynchronize)
		SetSynchronized();
}


//  ---
void CGridHeader::SetColor(COLORREF color)
{
	if (m_color == color)
		return;		//  Nothing to change

	m_color = color;
	m_bGridColor = FALSE;
	Change();
}


//  ---
void CGridHeader::SetFlat(BOOL bFlat)
{
	if (m_bFlat == bFlat)
		return;		//  Nothing to change

	m_bFlat = bFlat;
	if (bFlat && m_pGrid)
		m_pGrid->GetFixed()->SetFlat();

	SetSectionHeight(m_nSectionHeight);
	Change();
}


//  --
void CGridHeader::SetFont(CFont* pFont)
{
	m_pFont = pFont;
	Change();	//  ???
}


//  ---
void CGridHeader::SetFullSynchronizing(BOOL bFullSynchronizing)
{
	if (m_bFullSynchronizing == bFullSynchronizing)
		return;		//  Nothing to change

	m_bFullSynchronizing = bFullSynchronizing;
	if (bFullSynchronizing)
		SetSynchronized(FALSE);
}


//  ---
void CGridHeader::SetGridColor(BOOL bGridColor)
{
	if (m_bGridColor == bGridColor)
		return;		//  Nothing to change

	m_bGridColor = bGridColor;
	if (m_pGrid)
		GridColorChanged(m_pGrid->GetGridColor());
	SetSectionHeight(m_nSectionHeight);
	Change();
}


//  ---
void CGridHeader::SetGridFont(BOOL bGridFont)
{
	if (m_bGridColor == bGridFont)
		return;		//  Nothing to change
	
	m_bGridFont = bGridFont;
	if (m_pGrid)
		GridFontChanged(m_pGrid->GetFont());
	Change();
}


//  ---
void CGridHeader::SetImageList(CImageList* pImageList)
{
	if (m_pImageList == pImageList)
		return;		//  Nothing to change

	m_pImageList = pImageList;

	//  Adjust height and (probably) other changes
	SetSectionHeight(m_nSectionHeight);
	Change();
}



//
void CGridHeader::SetSections(const CGridHeaderSections& sections)
{
	m_sections = sections;
	SetSynchronized(FALSE);
}




//
void CGridHeader::SetSectionHeight(int nHeight)
{
	int nTextHeight, nImageHeight;

	if (m_bAutoHeight)  {
		nTextHeight = m_pGrid->GetFontHeight(m_pFont) + 4;	//  NYI
		nImageHeight = 0;

		if (m_pImageList)  {
			IMAGEINFO ii;
			BOOL bSuccess;

			bSuccess = m_pImageList->GetImageInfo(0, &ii);
			ASSERT(bSuccess);

			nImageHeight = ii.rcImage.bottom - ii.rcImage.top + 2;
			if (!m_bGridColor)
				++nImageHeight;
			if (!m_bFlat)
				++nImageHeight;
		}

		nHeight = max(nTextHeight, nImageHeight);
	}

	if (nHeight < 0)
		nHeight = 0;
	
	if (m_nSectionHeight != nHeight)  {
		m_nSectionHeight = nHeight;
		Change();
	}
}



//  -------------------------------------------------------------
void CGridHeader::SetSynchronized(BOOL bSynchronized)
{
	if (m_bSynchronized == bSynchronized)
		return;		//  Nothing to change

	m_bSynchronized = bSynchronized;
	if ((bSynchronized || m_bAutoSynchronize) && (m_pGrid != 0))  {
		m_bSynchronized = TRUE;
		SynchronizeSections();
	}
}


//  ---
void CGridHeader::Change()
{
	UpdateSections();
	if (m_pGrid)
		m_pGrid->HeaderChange();
}


//  ---
void CGridHeader::GridColorChanged(COLORREF newColor)
{

}


//  ---
void CGridHeader::GridFontChanged(CFont* pNewFont)
{

}



///////////////////////////////////////////////////////////////////////////////
////////////////////////////  CGridColumn  ////////////////////////////////////
IMPLEMENT_DYNCREATE(CGridColumn, CObject);


//  =====================  Default Constructor  =====================
CGridColumn::CGridColumn(CGridColumns* pOwnerColumns) : m_pColumns(pOwnerColumns),
							m_nWidth(64),
							m_nMinWidth(0),
							m_nMaxWidth(10000),
							m_nAlignment(LVCFMT_LEFT),
							m_bTabStop(TRUE),
							m_bVisible(TRUE),
							m_bAllowClick(TRUE),
							m_bFixedSize(FALSE),
							m_bWantReturns(FALSE),
							m_bWordWrap(FALSE),
							m_bReadOnly(FALSE),
							m_nMaxLength(0),
							m_editStyle(geSimple),
							m_checkKind(gcNone),
							m_nTag(0),
							m_bAlignEdit(FALSE)
{
}



//  ================================  Copy Constructor  =======================
CGridColumn::CGridColumn(const CGridColumn &otherColumn)
{
	m_bAlignEdit = otherColumn.m_bAlignEdit;
	m_bAllowClick = otherColumn.m_bAllowClick;
	m_bFixedSize = otherColumn.m_bFixedSize;
	m_bReadOnly = otherColumn.m_bReadOnly;
	m_bTabStop = otherColumn.m_bTabStop;
	m_bVisible = otherColumn.m_bVisible;
	m_bWantReturns = otherColumn.m_bWantReturns;
	m_bWordWrap = otherColumn.m_bWordWrap;
	m_checkKind = otherColumn.m_checkKind;
	m_editStyle = otherColumn.m_editStyle;
	m_nAlignment = otherColumn.m_nAlignment;
	m_nMaxWidth = otherColumn.m_nMaxWidth;
	m_nMinWidth = otherColumn.m_nMinWidth;
	m_nTag = otherColumn.m_nTag;
	m_nWidth = otherColumn.m_nWidth;
	m_pColumns = otherColumn.m_pColumns;

	m_pickList.Append(otherColumn.m_pickList);

	m_strCaption = otherColumn.m_strCaption;
	m_strEditMask = otherColumn.m_strEditMask;
}



//  ---
CGridColumn& CGridColumn::operator=(const CGridColumn& sourceColumn)
{
	if (this != &sourceColumn)  {
		INT_PTR nIndex;

		m_strCaption = sourceColumn.m_strCaption;
		m_nWidth = sourceColumn.m_nWidth;
		m_nMinWidth = sourceColumn.m_nMinWidth;
		m_nMaxWidth = sourceColumn.m_nMaxWidth;
		m_bFixedSize = sourceColumn.m_bFixedSize;
		m_nMaxLength = sourceColumn.m_nMaxLength;
		m_nAlignment = sourceColumn.m_nAlignment;
		m_bReadOnly = sourceColumn.m_bReadOnly;
		m_editStyle = sourceColumn.m_editStyle;
		m_strEditMask = sourceColumn.m_strEditMask;
		m_checkKind = sourceColumn.m_checkKind;
		m_bWantReturns = sourceColumn.m_bWantReturns;
		m_bWordWrap = sourceColumn.m_bWordWrap;
		m_bTabStop = sourceColumn.m_bTabStop;
		m_bVisible = sourceColumn.m_bVisible;

		m_pickList.RemoveAll();
		nIndex = m_pickList.Append(sourceColumn.m_pickList);

		m_nTag = sourceColumn.m_nTag;
		m_bAllowClick = sourceColumn.m_bAllowClick;
	}

	return *this;
}



//  --
int CGridColumn::GetEditAlignment()
{
	if (m_bAlignEdit)
		return m_nAlignment;

	return LVCFMT_LEFT;
}


//  --
CVirtualGridCtrl* CGridColumn::GetGrid()
{
	if (m_pColumns)
		return m_pColumns->m_pGrid;
	
	return 0;
}


//  --
void CGridColumn::GetPickList(CStringArray& pickList)
{
	pickList.RemoveAll();
	INT_PTR nIndex = pickList.Append(m_pickList);
}


//  --
int CGridColumn::GetPickListCount()
{
	return (int)m_pickList.GetSize();
}


//  --
CGridHeaderSection* CGridColumn::GetTitle()
{
	if (0 == GetGrid())
		return 0;

	return GetGrid()->GetHeaderSection(GetIndex(), -1);
}


//  --
int CGridColumn::GetWidth()
{
	if (!m_bVisible)
		return 0;

	return m_nWidth;
}


//  --
BOOL CGridColumn::IsPickListStored()
{
	return (BOOL)m_pickList.GetSize();
}


//  --
void CGridColumn::SetAlignEdit(BOOL bAlignEdit)
{
	if (m_bAlignEdit == bAlignEdit)
		return;		//  Nothing to change

	m_bAlignEdit = bAlignEdit;
	Changed(FALSE);
}


//  --
void CGridColumn::SetCheckKind(GridCheckKind checkKind)
{
	if (m_checkKind == checkKind)
		return;		//  Nothing to change

	m_checkKind = checkKind;
	Changed(FALSE);
}


//  --
void CGridColumn::SetMaxWidth(int nMaxWidth)
{
	if (nMaxWidth < m_nMinWidth)
		nMaxWidth = m_nMinWidth;
	if (nMaxWidth > 10000)
		nMaxWidth = 10000;

	m_nMaxWidth = nMaxWidth;
	SetWidth(m_nWidth);
}


//  ---
void CGridColumn::SetMinWidth(int nMinWidth)
{
	if (nMinWidth < 0)
		nMinWidth = 0;
	if (nMinWidth > m_nMaxWidth)
		nMinWidth = m_nMaxWidth;
	m_nMinWidth = nMinWidth;
	SetWidth(m_nWidth);
}


//  --
void CGridColumn::SetPickList(const CStringArray& pickList)
{
	m_pickList.RemoveAll();
	INT_PTR i = m_pickList.Append(pickList);
}



//  --
void CGridColumn::SetTabStop(BOOL bTabStop)
{
	if (m_bTabStop == bTabStop)
		return;		//  Nothing to change
	m_bTabStop = bTabStop;
	Changed(FALSE);
}


//  --
void CGridColumn::SetWantReturns(BOOL bWantReturns)
{
	if (m_bWantReturns == bWantReturns)
		return;		//  Nothing to change
	m_bWantReturns = bWantReturns;
	Changed(FALSE);
}


//  --
void CGridColumn::SetWordWrap(BOOL bWordWrap)
{
	if (m_bWordWrap == bWordWrap)
		return;		//  Nothing to change
	m_bWordWrap = bWordWrap;
	Changed(FALSE);
}


//  --
void CGridColumn::SetAlignment(int nAlignment)
{
	if (m_nAlignment == nAlignment)
		return;		//  Nothing to change
	m_nAlignment = nAlignment;
	Changed(FALSE);
}


//  ---
void CGridColumn::SetCaption(LPCTSTR lpCaption)
{
	if (!m_strCaption.Compare(lpCaption))
		return;		//  The caption remains the same

	m_strCaption = lpCaption;
	Changed(FALSE);
}


//  --
void CGridColumn::SetEditMask(LPCTSTR lpEditMask)
{
	if (!m_strEditMask.Compare(lpEditMask))
		return;		//  Nothing to change
	m_strEditMask = lpEditMask;
	Changed(FALSE);
}


//  --
void CGridColumn::SetEditStyle(GridEditStyle style)
{
	if (m_editStyle == style)
		return;		//  Nothing to change
	m_editStyle = style;
	Changed(FALSE);
}


//  --
void CGridColumn::SetMaxLength(int nMaxLength)
{
	if (m_nMaxLength == nMaxLength)
		return;		//  Nothing to change
	m_nMaxLength = nMaxLength;
	Changed(FALSE);
}


//  --
void CGridColumn::SetReadOnly(BOOL bReadOnly)
{
	if (m_bReadOnly == bReadOnly)
		return;		//  Nothing to change
	m_bReadOnly = bReadOnly;
	Changed(FALSE);
}


//  --
void CGridColumn::SetVisible(BOOL bVisible)
{
	if (m_bVisible == bVisible)
		return;		//  Nothing to change
	m_bVisible = bVisible;
	Changed();
}



//  --
void CGridColumn::SetWidth(int nWidth)
{
	if (m_nWidth == nWidth)
		return;		//  Nothing to change

	if (nWidth < m_nMinWidth)
		nWidth = m_nMinWidth;
	if (nWidth > m_nMaxWidth)
		nWidth = m_nMaxWidth;

	m_nWidth = nWidth;
	Changed();
}





//  --------------------------------------------------------
BOOL CGridColumn::GetAllowClick()
{
	return m_bAllowClick;
}


//  ------------------------------------------------------
void CGridColumn::SetAllowClick(BOOL bAllowClick)
{
	m_bAllowClick = bAllowClick;
}


//  -------------------------------------------------------
int CGridColumn::GetMaxWidth()
{
	return m_nMaxWidth;
}


//  ------------------------------------------------------
int CGridColumn::GetMinWidth()
{
	return m_nMinWidth;
}


//  -----------------------------------------------------
GridEditStyle CGridColumn::GetEditStyle()
{
	return m_editStyle;
}



/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  CGridColumns  /////////////////////////////////////////


//  ====================================================================================
CGridColumns::CGridColumns(CVirtualGridCtrl* pGrid) : m_pGrid(pGrid)
{
}



//  ======================  Copy constructor  ============================
CGridColumns::CGridColumns(const CGridColumns& otherColumns)
{
	INT_PTR i;

	for (i = m_columns.GetUpperBound(); i >= 0; --i)
		delete m_columns[i];

	m_columns.RemoveAll();

	for (i = 0; i < otherColumns.m_columns.GetSize(); ++i)
		m_columns.Add(new CGridColumn(*otherColumns.m_columns[i]));
}


//  ===========================================================================
CGridColumns::~CGridColumns()
{
	for (INT_PTR i = m_columns.GetUpperBound(); i >= 0; --i)
		delete m_columns[i];
}


//  ---
CGridColumns& CGridColumns::operator =(const CGridColumns& otherColumns)
{
	int i;
	INT_PTR nRetVal;
	CGridColumn* pColumn;

	for (i = 0; i < m_columns.GetSize(); ++i)
		delete m_columns[i];

	m_columns.RemoveAll();

//	if (m_pGrid)
//		m_pGrid->ColumnsChange(this);

	for (i = 0; i < otherColumns.m_columns.GetSize(); ++i) {
		pColumn = new CGridColumn(*otherColumns.m_columns[i]);
		pColumn->m_pColumns = this;
		nRetVal = m_columns.Add(pColumn);
	}

	if (m_pGrid)
		m_pGrid->ColumnsChange();

	return *this;
}


//  --
CGridColumn* CGridColumns::GetAt(int nIndex)
{
	return m_columns[nIndex];
}


//  --
int CGridColumns::GetSize()
{
	return (int)m_columns.GetSize();
}


//  --
int CGridColumns::GetUpperBound()
{
	return (int)m_columns.GetUpperBound();
}

//  --
CGridColumn* CGridColumns::Add()
{
	INT_PTR iColumn;

	if (m_pGrid == 0)  {
		iColumn = m_columns.Add(new CGridColumn(this));
		return m_columns[iColumn];
	}

	iColumn = m_columns.Add(new CGridColumn(this));
	m_pGrid->ColumnsChange();
	return m_columns[iColumn];
}




//  ---
void CGridColumns::RemoveAt(int nIndex)
{
	if (nIndex < 0 || nIndex > m_columns.GetUpperBound())
		return;		//  Nothing to remove

	delete m_columns[nIndex];
	m_columns.RemoveAt(nIndex);

	if (m_pGrid)
		m_pGrid->ColumnsChange();
}



//
CGridColumn* CGridColumns::GetColumn(int nIndex)
{
	return m_columns[nIndex];
}



//  --
void CGridColumns::SetColumn(int nIndex, CGridColumn* pColumn)
{
	if (nIndex < 0 || nIndex > m_columns.GetUpperBound())
		return;

	delete m_columns.GetAt(nIndex);
	m_columns.SetAt(nIndex, pColumn);
}



//  ---
void CGridColumns::Update(CGridColumn* pColumn)
{
	if (m_pGrid)
		m_pGrid->ColumnsChange();
}



////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  CGridRows  //////////////////////////////////////


//  ============================================================================
CGridRows::CGridRows(CVirtualGridCtrl* pGrid) : m_nCount(0),
						m_pGrid(pGrid),
						m_nHeight(17),
						m_bAutoHeight(FALSE)
{
	CDC ScreenDC;
	VERIFY(ScreenDC.CreateIC(_T("DISPLAY"), NULL, NULL, NULL));
	const int nLogDPIX = ScreenDC.GetDeviceCaps(LOGPIXELSX),
		nLogDPIY = ScreenDC.GetDeviceCaps(LOGPIXELSY);

	if (nLogDPIX == 96 && nLogDPIY == 96)
	{
		;// 96 DPI, Small Fonts
	}
	else
	{
		if (nLogDPIX == 120 && nLogDPIY == 120)
		{
			m_nHeight = 25;
		}
		else
		{
			m_nHeight = 25;
		}
	}
}


//
int CGridRows::GetMaxCount()
{
	if (m_nHeight > 0)
		return (INT_MAX - 2) / m_nHeight - 2;

	return INT_MAX - 2;
}


//
void CGridRows::SetAutoHeight(BOOL bAutoHeight)
{
	if (m_bAutoHeight == bAutoHeight)
		return;		//  Nothing to change


	m_bAutoHeight = bAutoHeight;
	if (bAutoHeight)
		SetHeight(m_nHeight);
}


//
void CGridRows::SetHeight(int nHeight)
{
	int nTextHeight, nFixedTextHeight;
	int nCheckHeight, nImageHeight;
	int nHeightWithGridLines;
	CGridFixed* pFixed;
	CImageList* pImageList;

	pFixed = m_pGrid->GetFixed();
	pImageList = m_pGrid->GetImageList();

	if (m_bAutoHeight && m_pGrid)  {
		//  Text height
		nTextHeight = m_pGrid->GetFontHeight(m_pGrid->GetFont()) + m_pGrid->GetTextTopIndent();
		nFixedTextHeight = m_pGrid->GetFontHeight(pFixed->GetFont());

		//  Flags height
		nCheckHeight = 0;
		if (m_pGrid->GetCheckBoxes())  {
			nCheckHeight = m_pGrid->GetCheckHeight() + m_pGrid->GetCheckTopIndent();
			if (pFixed->GetCount())  {
				if (!pFixed->GetFlat())
					nCheckHeight += 3;
				else if ((!pFixed->m_bGridColor) && (m_pGrid->GetGridLines())
					&& (gsHorzLine && m_pGrid->GetGridStyle()))

					++nCheckHeight;
			}  else  {
				if ((m_pGrid->GetGridLines()) && (gsHorzLine & m_pGrid->GetGridStyle()))
					++nCheckHeight;
			}
		}

		//  Image height
		nImageHeight = 0;

		if (pImageList)  {
			IMAGEINFO ii;

			pImageList->GetImageInfo(0, &ii);
			nImageHeight = ii.rcImage.right - ii.rcImage.left + m_pGrid->GetImageTopIndent();

			if (pFixed->GetCount())  {
				if (!pFixed->GetFlat())
					nImageHeight += 3;
				else if ((!pFixed->m_bGridColor) && m_pGrid->GetGridLines()
					&& (gsHorzLine && m_pGrid->GetGridStyle()))

					++nImageHeight;
			}  else  {
				if (m_pGrid->GetGridLines() && (gsHorzLine & m_pGrid->GetGridStyle()))
					++nImageHeight;
			}
		}

		//  Taking into account the grid lines
		nHeightWithGridLines = m_pGrid->GetGridLineWidth() * 2 * (m_pGrid->GetGridLines() && (gsHorzLine & m_pGrid->GetGridStyle()));

		//  At last... row height:
		nHeight = 0;
		nHeight = max(nHeight, nTextHeight);
		nHeight = max(nHeight, nFixedTextHeight);
		nHeight = max(nHeight, nCheckHeight);
		nHeight = max(nHeight, nImageHeight);
		nHeight = max(nHeight, nHeightWithGridLines);
	}

	//  ---
	if (nHeight < 0)
		nHeight = 0;

	//  Set...
	if (m_nHeight == nHeight)
		return;		//  Nothing to change

	m_nHeight = nHeight;

	if (m_nCount > GetMaxCount())
		SetCount(m_nCount);
	else
		Change();
}


//  --------------------------  User's event  -----------------------------
void CGridRows::Change()
{
	if (m_pGrid)
		m_pGrid->RowsChange();
}


//  -----------------------------------------------------------------------
void CGridRows::SetCount(int nCount)
{
	if (nCount < 0)
		nCount = 0;

	if (nCount > GetMaxCount())
		nCount = GetMaxCount();

	if (nCount == m_nCount)
		return;		//  Nothing to change

	m_nCount = nCount;
	Change();
}



///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  CGridFixed  ////////////////////////////////////////


//  =================================================================================
CGridFixed::CGridFixed(CVirtualGridCtrl* pGrid) : m_nCount(0),
						m_pGrid(pGrid),
						m_color(::GetSysColor(COLOR_BTNFACE)),
						m_bGridFont(TRUE),
						m_bFlat(TRUE),
						m_bGridColor(FALSE)
{
}



//
BOOL CGridFixed::IsColorStored()
{
	return !m_bGridColor;
}


//  --
BOOL CGridFixed::IsFontStored()
{
	return !m_bGridFont;
}


//  ---
void CGridFixed::FontChange()
{
	m_bGridFont = FALSE;
	Change();
}


//  ----------------------------------------------------------------
void CGridFixed::SetColor(COLORREF color)
{
	if (m_color == color)
		return;		//  Nothing to change

	m_color = color;
	m_bGridColor = FALSE;
	Change();
}


//  -----------------------------------------------------------------
void CGridFixed::SetFlat(BOOL bFlat)
{
	if (m_bFlat == bFlat)
		return;		//  Nothing to change

	m_bFlat = bFlat;

	//  Adjust 3D-effect of grid's header
	if (!bFlat && m_pGrid)
		m_pGrid->GetHeader()->SetFlat(FALSE);

	Change();
}


//  ----------------------------------------------------------------
void CGridFixed::SetFont(CFont* pFont)
{
	//  NYI
}


//  ----------------------------------------------------------------
void CGridFixed::SetGridColor(BOOL bGridColor)
{
	if (m_bGridColor == bGridColor)
		return;		//  Nothing to change

	m_bGridColor = bGridColor;
	if (m_pGrid)
		GridColorChanged(m_pGrid->GetGridColor());

	Change();
}


//  ---------------------------------------------------------------
void CGridFixed::SetGridFont(BOOL bGridFont)
{
	if (m_bGridFont == bGridFont)
		return;		//  Nothing to change

	m_bGridFont = bGridFont;
	if (m_pGrid)
		GridFontChanged(m_pGrid->GetFont());
	Change();
}


//  ----------------------------------------------------------------
void CGridFixed::Change()
{
	if (m_pGrid == 0)
		return;		//  Nothing to change

	m_pGrid->FixedChange();
}


//  ----------------------------------------------------------------
void CGridFixed::GridColorChanged(COLORREF newColor)
{
	if (m_bGridColor)  {
		SetColor(newColor);
		m_bGridColor = TRUE;
	}
}


//  ---------------------------------------------------------------
void CGridFixed::GridFontChanged(CFont* pFont)
{
	if (m_bGridFont)  {
		SetFont(pFont);
		m_bGridFont = TRUE;
	}
}



//  --------------------------------------------------------------
void CGridFixed::SetCount(int nCount)
{
	//  Check the value
	if (m_pGrid && nCount > m_pGrid->GetColumns()->GetUpperBound())
		nCount = m_pGrid->GetColumns()->GetUpperBound();

	if (nCount < 0)
		nCount = 0;

	//  Set...
	if (m_nCount == nCount)
		return;		//  Nothing to set

	m_nCount = nCount;
	Change();
}


//  ------
CFont* CGridFixed::GetFont()
{
	return m_pFont;
}


//  ---------------------------------------------------------
BOOL CGridFixed::GetFlat()
{
	return m_bFlat;
}


//  ---
int CGridRows::GetCount()
{
	return m_nCount;
}


//  ---
int CGridFixed::GetCount()
{
	return m_nCount;
}





//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/////////////////////////  CGridScrollBar  ///////////////////////////////////


void CGridScrollBar::SetVisible(BOOL bVisible)
{
	if (m_bVisible != bVisible) {
		m_bVisible = bVisible;
		Update();
	}
}


void CGridScrollBar::Change()
{
	switch (m_nBar) {
	case SB_HORZ:
		m_pGrid->HorzScrollChange();
		break;
	case SB_VERT:
		m_pGrid->VertScrollChange();
		break;
	}
}



//
void CGridScrollBar::Scroll(int nScrollCode, int& nScrollPos)
{
/* NYI
  User notification.
*/
}



//
void CGridScrollBar::ScrollMessage(int nScrollCode, short nPos)
{
	LockUpdate();

	switch (nScrollCode) {
	case SB_LINELEFT:
		SetPositionEx(m_nPosition - m_nLineStep, nScrollCode);
		break;
	case SB_LINERIGHT:
		SetPositionEx(m_nPosition + m_nLineStep, nScrollCode);
		break;
	case SB_PAGELEFT:
		SetPositionEx(m_nPosition - m_nPageStep, nScrollCode);
		break;
	case SB_PAGERIGHT:
		SetPositionEx(m_nPosition + m_nPageStep, nScrollCode);
		break;
	case SB_THUMBPOSITION:
		SetPositionEx(WinPosToScrollPos(nPos, m_nMin, m_nMax), nScrollCode);
		break;
	case SB_THUMBTRACK:
		if (m_bTracking)
			SetPositionEx(WinPosToScrollPos(nPos, m_nMin, m_nMax), nScrollCode);
		break;
	case SB_ENDSCROLL:
		SetPositionEx(m_nPosition, nScrollCode);
		break;
	}

	UnLockUpdate();
}



//
void CGridScrollBar::SetParams(int nMin, int nMax, int nPageStep, int nLineStep)
{
	// Check new values.
	if (nPageStep < 0)
		nPageStep = 0;
	if (nLineStep < 0)
		nLineStep = 0;
	if (nMax < nMin)
		nMax = nMin;

	// Is there changed something?
	if ((m_nMin != nMin) || (m_nMax != nMax) || (m_nPageStep != nPageStep) || (m_nLineStep != nLineStep)) {
		// Set new parameters.
		m_nMin = nMin;
		m_nMax = nMax;
		m_nPageStep = nPageStep;
		m_nLineStep = nLineStep;

		// Adjust position.
		if (m_nPosition > GetRange() - m_nPageStep)
			m_nPosition = GetRange() - m_nPageStep;
		if (m_nPosition < 0)
			m_nPosition = 0;

		// Update scroll bar.
		Update();
	}
}


//
void CGridScrollBar::SetPositionEx(int nValue, int nScrollCode)
{
	CRect r;

	// Check the position.
	UpdatePosition(nValue);

	// Is position changed?
	if (nValue != m_nPosition) {
		Scroll(nScrollCode, nValue);

		// Check again.
		UpdatePosition(nValue);
	}

	// Does the position changed after user's reaction?
	if (nValue != m_nPosition) {
		// --- Offset the grid. ---

		// Swith off the focus.
		m_pGrid->HideFocus();

		//  Offset.
		if (m_nBar == SB_HORZ) {
			CRect rcFixed;

			m_pGrid->GetClientRect(&r);
			rcFixed = m_pGrid->GetFixedRect();
			r.left = rcFixed.right;
			m_pGrid->ScrollWindowEx(
				(m_nPosition - nValue) * m_nLineSize,
				0,
				&r,
				&r,
				0,
				0,
				SW_INVALIDATE
				);
		} else {
			r = m_pGrid->GetGridRect();
			m_pGrid->ScrollWindowEx(
				0,
				(m_nPosition - nValue) * m_nLineSize,
				&r,
				&r,
				0,
				0,
				SW_INVALIDATE
				);
		}

		// Set new position.
		m_nPosition = nValue;

		m_pGrid->ShowFocus();
	}

	// Set the scroll bar.
	Update();

	// Reflect updates to grid.
	Change();
}




//
void CGridScrollBar::Update()
{
	if (m_pGrid->GetSafeHwnd() && !m_nUpdateLock) {
		SCROLLINFO si;

		// Set scroll bar parameters.
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = nMaxWinPos * ((m_bVisible && (GetRange() > GetPageStep())) ? 1 : 0);
		si.nPage = ScrollPosToWinPos(GetMin() + GetPageStep(), GetMin(), GetMax());
		si.nPos = ScrollPosToWinPos(GetPosition(), GetMin(), GetMax());

		m_pGrid->SetScrollInfo(m_nBar, &si);
	}
}







//  ------------------------------------------------------
int CGridHeaderSections::GetSize()
{
	return (int)m_sections.GetSize();
}


//  -------------------------------------------------------
CVirtualGridCtrl* CGridHeader::GetGrid()
{
	return m_pGrid;
}


//  ------------------------------------------------------
CGridColumns* CVirtualGridCtrl::GetColumns()
{
	return &(*m_pColumns);
}



//  --
int CGridHeaderSections::GetUpperBound()
{
	return (int)m_sections.GetUpperBound();
}


//  --
BOOL CVirtualGridCtrl::ColumnsSizing()
{
	return m_bColumnsSizing;
}


//  --
int CGridColumn::GetIndex()
{
	int i;

	if (!m_pColumns->GetSize())
		return -1;

	for (i = 0; i < m_pColumns->GetSize(); ++i) {
		if (this == m_pColumns->GetColumn(i))
			return i;
	}

	return -1;
}



//  ---
CString CGridColumn::GetCaption()
{
	return m_strCaption;
}

int CGridColumn::GetAlignment()
{
	return m_nAlignment;
}


//  --
void CGridColumn::SetFixedSize(BOOL bFixedWidth)
{
	if (m_bFixedSize == bFixedWidth)
		return;		//  Nothing to change

	m_bFixedSize = bFixedWidth;
}

int CGridColumn::GetFixedSize()
{
	return m_bFixedSize;
}


//  ---
BOOL CGridColumn::GetReadOnly()
{
	return m_bReadOnly;
}


//  ---
BOOL CGridColumn::GetVisible()
{
	return m_bVisible;
}


//  --
void CGridColumn::Changed(BOOL bAllItems)
{
	if (m_pColumns)
		m_pColumns->Update(this);
}



//  --
void CGridHeaderSections::DeleteSections()
{
	INT_PTR i;
	CGridHeaderSection* pSection;

	for (i = m_sections.GetUpperBound(); i >= 0; i --)  {
		pSection = m_sections[i];

		pSection->m_sections.DeleteSections();

		if (0 == pSection->m_sections.GetSize()
			&& pSection->m_iColumn > m_pHeader->GetGrid()->GetColumns()->GetUpperBound())  {

			delete pSection;
			m_sections.RemoveAt(i);
		}
	}
}


//  ----------------------------------------------------------------
void CGridHeaderSections::Synchronize(int& iColumn)
{
	CGridColumn* pColumn;

	for (INT_PTR i = m_sections.GetUpperBound(); i >= 0; --i) {
		CGridHeaderSection* pSection = m_sections[i];

		if (0 == pSection->m_sections.GetSize())  {
			iColumn = pSection->m_iColumn;

			pColumn = m_pHeader->GetGrid()->GetColumns()->GetColumn(iColumn);
			pSection->m_nWidth = pColumn->GetWidth();

			if (m_pHeader->m_bFullSynchronizing)  {
				pSection->m_strCaption = pColumn->GetCaption();
				pSection->m_nAlignment = pColumn->GetAlignment();
			}
		}  else  {
			pSection->m_sections.Synchronize(iColumn);
		}
	}
}


//  ---
CGridHeaderSections* CGridHeader::GetSections()
{
	return &m_sections;
}



//  ----
CGridHeaderSections& CGridHeaderSections::operator=(const CGridHeaderSections& otherSections)
{
	int i;

	for (i = 0; i < m_sections.GetSize(); ++i)
		delete m_sections[i];

	m_sections.RemoveAll();


	for (i = 0; i < otherSections.m_sections.GetSize(); ++i)
		m_sections.Add(otherSections.m_sections[i]);

	return *this;
}



//  ---
CString CGridHeaderSection::GetCaption()
{
	return m_strCaption;
}


//  ---
void CGridHeaderSection::Changed(BOOL bAllItems)
{

}




//  ---
CGridHeaderSection* CGridHeaderSection::Add()
{
	CGridHeaderSection* pSection;
	CGridHeader* pHeader;

	pSection = m_sections.Add();
	pHeader = GetHeader();
	if (pHeader)
		pHeader->GetGrid()->HeaderChange();

	return pSection;
}



//  ---
int CGridColumn::GetMaxLength()
{
	return m_nMaxLength;
}



//  --
BOOL CGridColumn::GetWantReturns()
{
	return m_bWantReturns;
}


//  ---
BOOL CGridColumn::GetWordWrap()
{
	return m_bWordWrap;
}



BOOL CGridColumn::GetAlignEdit()
{
	return m_bAlignEdit;
}


//  --
BOOL CGridColumn::GetTabStop()
{
	return m_bTabStop;
}


//  -----
GridCheckKind CGridColumn::GetCheckKind()
{
	return m_checkKind;
}


//  ----
BOOL CGridRows::GetAutoHeight()
{
	return m_bAutoHeight;
}


//  ----
int CGridRows::GetHeight()
{
	return m_nHeight;
}


//  ---
BOOL CGridHeader::GetAutoHeight()
{
	return m_bAutoHeight;
}


//  ---
BOOL CGridHeader::GetAutoSynchronize()
{
	return m_bAutoSynchronize;
}


//  ---
BOOL CGridHeader::GetFlat()
{
	return m_bFlat;
}

//  ---
BOOL CGridHeader::GetFullSynchronizing()
{
	return m_bFullSynchronizing;
}

//  ---
BOOL CGridHeader::GetGridColor()
{
	return m_bGridColor;
}

//  --
int CGridHeader::GetSectionHeight()
{
	return m_nSectionHeight;
}


//  --
BOOL CGridFixed::GetGridColor()
{
	return m_bGridColor;
}







/////////////////////////////////////////////////////////////////////////////
// CGridListBox

IMPLEMENT_DYNCREATE(CGridListBox, CListBox)


//  =========================================================================
CGridListBox::CGridListBox() : m_pGrid(0)
{
}


//
BEGIN_MESSAGE_MAP(CGridListBox, CListBox)
	//{{AFX_MSG_MAP(CGridListBox)
	ON_WM_CHAR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridListBox message handlers


//  ------------------------------------------------------------------------
void CGridListBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListBox::OnChar(nChar, nRepCnt, nFlags);
}


//  ------------------------------------------------------------------------
void CGridListBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CGridEdit* pEdit;

	CListBox::OnLButtonUp(nFlags, point);

	pEdit = m_pGrid->GetEdit();

	if (m_pGrid == 0 || pEdit == 0)
		return;

	CRect rcClient;
	GetClientRect(rcClient);

	if (::IsWindow(pEdit->GetSafeHwnd()))  {
		pEdit->CloseUp((point.x >= 0) && (point.y >= 0) && (point.x < rcClient.Width()) && (point.y < rcClient.Height()));
	}
}



//
void CGridListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	BOOL bOutside;
	UINT uItem = ItemFromPoint(point, bOutside);
	if (uItem == (UINT)GetCurSel())
		return;		//  Current item not changed.

	SetCurSel(uItem);

	CListBox::OnMouseMove(nFlags, point);
}



/////////////////////////////////////////////////////////////////////////////
// CGridEdit

IMPLEMENT_DYNCREATE(CGridEdit, CEdit);


//  ===============================================================================
CGridEdit::CGridEdit() : m_editStyle(geSimple),
			m_nDropDownCount(8),
			m_bButtonPressed(FALSE),
			m_bDropListVisible(FALSE),
			m_bButtonTracking(FALSE),
			m_bDefocusing(FALSE),
			m_bWantReturns(FALSE),
			m_pPickList(0),
			m_pGrid(0)
{
	m_nButtonWidth = ::GetSystemMetrics(SM_CXVSCROLL);
}


//  ==============================================================================
CGridEdit::~CGridEdit()
{
	if (m_pPickList != 0)
		delete m_pPickList;	//  Bug fix: thanks to mr. QuiOui
}


//  -----
CRect CGridEdit::GetButtonRect()
{
	CRect rect;

	GetClientRect(rect);
	rect.left = rect.Width() - m_nButtonWidth;
	rect.top = 0;		// ??
	return rect;
}




//
void CGridEdit::SetButtonWidth(int nWidth)
{
	if (m_nButtonWidth == nWidth)
		return;		//  Nothing to change

	m_nButtonWidth = nWidth;
	Invalidate();
}



//
CGridListBox* CGridEdit::GetDropList()
{
	BOOL bSuccess;

	if (m_pPickList)
		return m_pPickList;

	m_pPickList = new CGridListBox;

	m_pPickList->m_pGrid = m_pGrid;

	CRect rect(0, 0, 0, 0);

	bSuccess = m_pPickList->Create(
		WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_WANTKEYBOARDINPUT,
		rect,
		m_pGrid,
		0
		);
	ASSERT(bSuccess);

	bSuccess = m_pPickList->ModifyStyleEx(0, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
	ASSERT(bSuccess);

	m_pPickList->SetParent(0);
	m_pPickList->SetFocus();

	return m_pPickList;
}



//
void CGridEdit::SetDropListVisible(BOOL bVisible)
{
	if (bVisible)
		DropDown();
	else
		CloseUp(FALSE);
}


//  ---
void CGridEdit::SetEditStyle(GridEditStyle style)
{
	if (m_editStyle == style)
		return;		//  Nothing to change
	m_editStyle = style;
	Invalidate();
}




//
void CGridEdit::UpdateBounds(BOOL bScrollCaret)
{
	CRect rect;
	CRect workingRect;
	BOOL bSuccess;
	int nLeft, nTop, nHeight, nWidth;
	CPoint textOffset;

	if (m_pGrid == 0)
		return;		//  Nothing to update

	rect = m_pGrid->GetEditRect(m_pGrid->GetEditCell());
	CRect initialRect(rect);	//  Memorize
	workingRect = m_pGrid->GetFixedRect();

	//  Adjust the line according to fixed part...
	if (rect.left < workingRect.right)
		rect.left = workingRect.right;
	if (rect.right < workingRect.right)
		rect.right = workingRect.right;

	//  Adjust the line according to the header...
	workingRect = m_pGrid->GetHeaderRect();
	if (rect.top < workingRect.bottom)
		rect.top = workingRect.bottom;
	if (rect.bottom < workingRect.bottom)
		rect.bottom = workingRect.bottom;

	//  Set position...
	bSuccess = SetWindowPos(
		&wndTop,
		rect.left,
		rect.top,
		rect.Width(),
		rect.Height(),
		SWP_SHOWWINDOW | SWP_NOREDRAW
		);
	ASSERT(bSuccess);

	//  Calculate new text borders...
	nLeft = initialRect.left - rect.left;
	nTop = initialRect.top - rect.top;
	nWidth = initialRect.Width();
	nHeight = initialRect.Height();		//  Bug fix: thanks to mr. QuiOui

	textOffset = m_pGrid->GetCellTextIndent(m_pGrid->GetEditCell());

	//  Taking the button into account...
	if (m_editStyle != geSimple)
		nWidth -= (m_nButtonWidth + 1);
	else
		nWidth -= m_pGrid->GetTextRightIndent();

	//  Set the text borders...
	rect.left = nLeft + textOffset.x;
	rect.top = nTop + textOffset.y;
	rect.right = nWidth - textOffset.x + (m_nAlignment == LVCFMT_RIGHT);
	rect.bottom = nHeight;

	SendMessage(EM_SETRECTNP, 0, (LPARAM)(LPRECT)rect);

	//  Move cursor at the line's end
	if (bScrollCaret)
		SendMessage(EM_SCROLLCARET, 0, 0);
}



//
void CGridEdit::UpdateColors()
{
	//  NYI
}




//
void CGridEdit::UpdateContents()
{
	CGridColumn* pColumn;

	if (m_pGrid == 0 || !m_pGrid->IsCellValid(m_pGrid->GetEditCell()))
		return;		//  Nothing to update

	pColumn = m_pGrid->GetColumn(m_pGrid->GetEditCell().m_iColumn);

	m_nMaxLength = pColumn->GetMaxLength();
	m_bReadOnly = pColumn->GetReadOnly();
	m_bWantReturns = pColumn->GetWantReturns();
	m_bWordWrap = pColumn->GetWordWrap();
	m_nAlignment = pColumn->GetAlignment();
	SetWindowText(m_pGrid->GetEditText(m_pGrid->GetEditCell()));
	SetSel(0, -1);
}



//
void CGridEdit::UpdateList()
{
	if (m_pPickList) {
		m_pPickList->ShowWindow(SW_HIDE);
		m_pPickList->SetFont(GetFont());
	}
}



//
void CGridEdit::UpdateListValue(BOOL bAccept)
{
	if (m_pPickList) {
		int nIndex = m_pPickList->GetCurSel();

		if (m_pGrid)
			m_pGrid->EditCloseUp(m_pGrid->GetEditCell(), nIndex, bAccept);

		if (bAccept && nIndex >= 0)  {
			CString strText;

			m_pPickList->GetText(nIndex, strText);
			SetWindowText(strText);
			SetSel(0, -1);
		}
	}
}



//  ---
void CGridEdit::UpdateStyle()
{
	GridEditStyle style;

	style = geSimple;

	if (m_pGrid && !m_pGrid->GetReadOnly())
		style = m_pGrid->GetEditStyle(m_pGrid->GetEditCell());

	SetEditStyle(style);
}


//  ---
void CGridEdit::CloseUp(BOOL bAccept)
{
	const UINT nFlags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW;
	BOOL bSuccess;

	if (!m_bDropListVisible)
		return;		//  Nothing to close

	CWnd* pWnd = GetCapture();
	if (pWnd)
		pWnd->SendMessage(WM_CANCELMODE, 0, 0);

	//  Hide the listbox
	bSuccess = m_pPickList->SetWindowPos(0, 0, 0, 0, 0, nFlags);
	ASSERT(bSuccess);

	m_bDropListVisible = FALSE;
	
	Invalidate();

	//  Set selected value
	UpdateListValue(bAccept);
}



//
void CGridEdit::Deselect()
{
	SetSel(0, -1);
}



//
void CGridEdit::DropDown()
{
	const UINT nFlags = SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW;
	BOOL bSuccess;

	if (m_bDropListVisible && m_pGrid == 0)
		return;

	if (!(m_editStyle & gePickList) && !(m_editStyle & geDataList))
		return;

	m_pPickList = GetDropList();
	if (m_pPickList == 0)
		return;		//  Nothing to dropdown

	//  Update the list box and set its size
	UpdateList();
	UpdateListItems();
	UpdateListBounds();

	RECT rc;
	m_pPickList->GetWindowRect(&rc);
	m_pGrid->ClientToScreen(&rc);

	//  Show the list box in the correct place
	bSuccess = m_pPickList->SetWindowPos(
		&wndTop,
		rc.left,
		rc.top,
		0,
		0,
		nFlags
		);
	ASSERT(bSuccess);

	m_bDropListVisible = TRUE;
	Invalidate();
	SetFocus();
}



//
void CGridEdit::Hide()
{
	const UINT nFlags = SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOREDRAW;
	if (IsWindowVisible()) {
		m_pGrid->m_bEditing = FALSE;

		//
		Invalidate();
		BOOL bSuccess = SetWindowPos(0, 0, 0, 0, 0, nFlags);
		ASSERT(bSuccess);

		if (this == GetFocus()) {
			m_bDefocusing = TRUE;
			CWnd* pWndOld = m_pGrid->SetFocus();
			m_bDefocusing = FALSE;
		}
	}
}


//
void CGridEdit::Press()
{
	if (m_pGrid)
		m_pGrid->EditButtonPress(m_pGrid->GetEditCell());
}


//
void CGridEdit::SelectNext()
{
}


//  ----
void CGridEdit::Show()
{
	BOOL bScrollCaret;

	if (m_pGrid == 0)
		return;

	bScrollCaret = !m_pGrid->GetEditing();
	m_pGrid->m_bEditing = TRUE;
	m_pGrid->m_bCellSelected = TRUE;

	UpdateColors();

	UpdateBounds(bScrollCaret);
	if (m_pGrid == GetFocus())
		SetFocus();

	CDC* pDC = GetDC();

	pDC->ExcludeClipRect(GetButtonRect());

	ReleaseDC(pDC);
}



//  ---
void CGridEdit::UpdateListItems()
{
	int nIndex;

	if (m_pGrid == 0 || m_pPickList == 0)
		return;		//  Nothing to update


	//  Clean up old list, populate new one...
	m_pPickList->ResetContent();
	
	CStringArray items;
	m_pGrid->GetEditList(m_pGrid->GetEditCell(), items);

	for (int i = 0; i < items.GetSize(); ++i)
		nIndex = m_pPickList->AddString(items[i]);

	//  Set selected position.
	CString strEditText;
	GetWindowText(strEditText);
	nIndex = m_pPickList->SelectString(-1, strEditText);
}




//
void CGridEdit::UpdateListBounds()
{
	if (m_pGrid == 0 || m_pPickList == 0)
		return;		//  No bounds to update.

	int nWidth = 0;
	int i;

	if (m_pPickList->GetCount()) {
		CString strText;

		CDC* pDC = m_pPickList->GetDC();

		for (i = 0; i < m_pPickList->GetCount(); ++i) {
			m_pPickList->GetText(i, strText);
			int x = pDC->GetTextExtent(strText).cx;

			if (nWidth < x)
				nWidth = x;
		}

		i = m_pPickList->ReleaseDC(pDC);
		nWidth += 6;
	}  else  {
		nWidth = 100;
	}

	//  Let's calculate the height...
	int nHeight = 0;

	if (m_nDropDownCount < 1 || !m_pPickList->GetCount())
		nHeight = m_pPickList->GetItemHeight(0) * 2;
	else if (m_pPickList->GetCount() < m_nDropDownCount)
		nHeight = m_pPickList->GetCount() * m_pPickList->GetItemHeight(0) + 6;
	else
		nHeight = (m_nDropDownCount + 1) * m_pPickList->GetItemHeight(0);

	//  Adjust list size and position according to column size and position
	CRect rect;

	//  Adjust according to column width...
	rect = m_pGrid->GetCellRect(m_pGrid->GetEditCell());
	nWidth = max(nWidth, rect.Width());

	//  Position...
	int nLeft = rect.left + rect.Width() - nWidth - 1;
	int nTop = rect.top + rect.Height() - 1;

	BOOL bSuccess = m_pPickList->SetWindowPos(0, nLeft, nTop, nWidth, nHeight, SWP_NOZORDER);
	ASSERT(bSuccess);
}




//  Paint button with ellipsis or "combo box button".
void CGridEdit::PaintButton(HDC hDC)
{
	if (m_editStyle == geSimple)
		return;		//  Nothing to paint.

	//  Get button rectangle.
	CRect rcButton = GetButtonRect();

	//  Paint the button.
	switch (m_editStyle) {
	case geEllipsis:
		//  Button with ellipsis.
		PaintBtnEllipsis(hDC, rcButton, m_bButtonPressed);
		break;
	case gePickList:
	case geDataList:
		PaintBtnComboBox(hDC, rcButton, m_bButtonPressed);
		break;
	}
}




//
void CGridEdit::StartButtonTracking(POINT ptHit)
{
	CWnd* pWndOld = SetCapture();
	m_bButtonTracking = TRUE;
	StepButtonTracking(ptHit);
}



//
void CGridEdit::StepButtonTracking(POINT ptHit)
{
	CRect rcButton = GetButtonRect();
	BOOL bPress = rcButton.PtInRect(ptHit);

	if (m_bButtonPressed != bPress) {
		m_bButtonPressed = bPress;
		InvalidateRect(rcButton);
	}
}


//
void CGridEdit::StopButtonTracking()
{
	if (!m_bButtonTracking)
		return;		//  Nothing to stop.
	POINT pt = {-1, -1};

	StepButtonTracking(pt);
	m_bButtonTracking = FALSE;
	BOOL bSuccess = ::ReleaseCapture();
	ASSERT(bSuccess);
}



//
void CGridEdit::MouseDown(UINT nFlags, CPoint point)
{
	if (m_editStyle != geSimple && GetButtonRect().PtInRect(point)) {
		if (m_bDropListVisible) {
			CloseUp(FALSE);
		} else {
			StartButtonTracking(point);
			if (m_editStyle != geEllipsis)
				DropDown();
		}
	}
}



/*
  *  Delete the requested message from the queue, but throw back
  *  any WM_QUIT msgs that PeekMessage may also return.
*/
void KillMessage(HWND hWnd, UINT uMsg)
{
	MSG msg;

	msg.message = 0;

	if (::PeekMessage(&msg, hWnd, uMsg, uMsg, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			PostQuitMessage((int)msg.wParam);
	}
}



//
void CGridEdit::DoDropDownKeys(TCHAR& nChar, LPARAM nFlags)
{
	switch (nChar) {
	case VK_UP:
	case VK_DOWN:
	{
		//  Opening or closing...
		if (nFlags & 0x20000000) {
			if (m_bDropListVisible)
				CloseUp(TRUE);
			else
				DropDown();

			nChar = 0;
		}

		break;
	}
	case VK_RETURN:
	case VK_ESCAPE:
	{
		if ((!(nFlags & 0x20000000)) && m_bDropListVisible) {
			KillMessage(m_hWnd, WM_CHAR);
			CloseUp(nChar == VK_RETURN);
			nChar = 0;
		}
		break;
	}
	}
}



//
void CGridEdit::DoButtonKeys(TCHAR &nChar, LPARAM nFlags)
{
	if (nChar == VK_RETURN && IS_CTRL_PRESSED()) {
		::KillMessage(m_hWnd, WM_CHAR);
		nChar = 0;

		//  Button pressing emulation.
		switch (m_editStyle) {
		case geEllipsis:
			Press();
			break;
		case gePickList:
		case geDataList:
			break;	// NYI
		}
	}
}




//
LRESULT CGridEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_CHAR:
	{
		//  Open up the list.
		if (m_editStyle == gePickList || m_editStyle == geDataList) {
			DoDropDownKeys((TCHAR&)wParam, lParam);

			if (wParam && m_bDropListVisible)
				return m_pPickList->SendMessage(message, wParam, lParam);
		}

		//  Button pressing emulation.
		if (!m_bWantReturns) {
			DoButtonKeys((TCHAR&)wParam, lParam);
			if (wParam == 0)
				return 0;	//  ???
		}

		break;
	}

	case WM_SETFOCUS:
		break;
	case WM_LBUTTONDOWN:
		break;
	}

	return CEdit::WindowProc(message, wParam, lParam);
}



//
void CGridEdit::SendToParent(UINT& nChar, UINT nFlags)
{
	if (m_pGrid) {
		m_pGrid->KeyDown(nChar, nFlags);
		nChar = 0;
	}
}




//
void CGridEdit::ParentEvent(UINT& nChar, UINT nFlags)
{
	if (m_pGrid)
		m_pGrid->GridKeyDown(nChar, nFlags);
}




//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGridEdit, CEdit)
	//{{AFX_MSG_MAP(CGridEdit)
	ON_WM_GETDLGCODE()
	ON_WM_KILLFOCUS()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYUP()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CHAR()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_WM_CANCELMODE()
	ON_WM_KEYDOWN()
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridEdit message handlers


void CGridEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	//  Paint the button if required.
	PaintButton(dc.m_hDC);

	//  Remove button rectangle from painting area.
	if (m_editStyle != geSimple) {
		CRect rcButton = GetButtonRect();
		dc.ExcludeClipRect(rcButton);
	}

	//  Default paint.
	CEdit::DefWindowProc(WM_PAINT, (WPARAM)dc.m_hDC, 0);
}




//
UINT CGridEdit::OnGetDlgCode() 
{
	UINT nCode = CEdit::OnGetDlgCode();

	nCode |= DLGC_WANTARROWS | DLGC_WANTCHARS;
	if (gkTabs & m_pGrid->m_nCursorKeys)
		nCode |= DLGC_WANTTAB;

	return nCode;
}



//
void CGridEdit::OnCancelMode()
{
	CEdit::OnCancelMode();
	StopButtonTracking();
}



//
void CGridEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	CloseUp(FALSE);
}



//
void CGridEdit::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CEdit::OnWindowPosChanged(lpwndpos);
	Invalidate();
}


//
void CGridEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_editStyle != geSimple && GetButtonRect().PtInRect(point)) {
		SetCapture();
		MouseDown(nFlags, point);
	} else {
		CloseUp(FALSE);
		CEdit::OnLButtonDown(nFlags, point);
	}
}


//
void CGridEdit::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_editStyle != geSimple && GetButtonRect().PtInRect(point))
		return;
	
	CEdit::OnLButtonDblClk(nFlags, point);
}


//
BOOL CGridEdit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	POINT ptCursor;

	BOOL bSuccess = ::GetCursorPos(&ptCursor);
	ASSERT(bSuccess);

	ScreenToClient(&ptCursor);

	if (m_editStyle != geSimple && GetButtonRect().PtInRect(ptCursor)) {
		::SetCursor(::LoadCursor(0, IDC_ARROW));
		return FALSE;
	}
	
	return CEdit::OnSetCursor(pWnd, nHitTest, message);
}



//
void CGridEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	//  NYI
}



//
void CGridEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);

	switch (nChar) {
	case VK_UP:
	case VK_DOWN:
		if ((IS_CTRL_PRESSED() || !((1 << 13) & nFlags)) && (!(m_bWantReturns && m_bWordWrap)))
			SendToParent(nChar, nFlags);
		break;
	case VK_PRIOR:
	case VK_NEXT:
		if (IS_CTRL_PRESSED())
			SendToParent(nChar, nFlags);
		break;
	case VK_ESCAPE:
		SendToParent(nChar, nFlags);
		break;
	case VK_DELETE:
		if (!EditCanModify())
			SendToParent(nChar, nFlags);
		break;
	case VK_INSERT:
		if (!EditCanModify() || nFlags == 0)
			SendToParent(nChar, nFlags);
		break;
	case VK_TAB:
		SendToParent(nChar, nFlags);
		break;
	}

	//  Char not handled: OK let's send it to grid.
	if (nChar)
		ParentEvent(nChar, nFlags);
}



//
void CGridEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_pGrid)
		m_pGrid->OnKeyUp(nChar, nRepCnt, nFlags);
}


//
void CGridEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	BOOL bPressed = m_bButtonPressed;

	StopButtonTracking();

	if (m_editStyle == geEllipsis && bPressed)
		Press();
	
	CEdit::OnLButtonUp(nFlags, point);
}




//
void CGridEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bButtonTracking) {
		StepButtonTracking(point);

		//  If list box is dropped...
		if (m_bDropListVisible) {
			ClientToScreen(&point);
			m_pPickList->ScreenToClient(&point);
			CRect rect;
			m_pPickList->GetClientRect(&rect);
			if (rect.PtInRect(point)) {
				StopButtonTracking();
				m_pPickList->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(point.x, point.y));
				return;
			}
		}
	}

	CEdit::OnMouseMove(nFlags, point);
}





//
BOOL CGridEdit::EditCanModify()
{
	if (m_pGrid == 0)
		return FALSE;

	return m_pGrid->EditCanModify(m_pGrid->GetEditCell());
}



//
void CGridEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//  Send the character to the grid.
	m_pGrid->KeyPress(nChar);

	if (/*/isalnum(nChar) && */ !m_pGrid->EditCanAcceptKey(m_pGrid->m_editCell, nChar)) {
		nChar = 0;
		MessageBeep(0);
	}

	if (!EditCanModify())
		nChar = 0;

	if (nChar)
		CEdit::OnChar(nChar, nRepCnt, nFlags);
}




void CGridEdit::OnEnChange()
{
	m_pGrid->EditChange(m_pGrid->GetEditCell());
}



//
void PaintBtnEllipsis(HDC hDC, LPRECT lpRect, BOOL bPressed)
{
	UINT nFlags = 0;
	BOOL bSuccess;
	CDC* pDC = CDC::FromHandle(hDC);

	if (bPressed)
		nFlags = BF_FLAT;

	pDC->DrawEdge(lpRect, EDGE_RAISED, BF_RECT | BF_MIDDLE | nFlags);

	nFlags = (lpRect->right - lpRect->left) / 2 - 1 + (bPressed ? 1 : 0);

	bSuccess = pDC->PatBlt(lpRect->left + nFlags, lpRect->top + nFlags, 2, 2, BLACKNESS);
	ASSERT(bSuccess);

	bSuccess = pDC->PatBlt(lpRect->left + nFlags - 3, lpRect->top + nFlags, 2, 2, BLACKNESS);
	ASSERT(bSuccess);

	bSuccess = pDC->PatBlt(lpRect->left + nFlags + 3, lpRect->top + nFlags, 2, 2, BLACKNESS);
	ASSERT(bSuccess);
}



//
void PaintBtnComboBox(HDC hDC, LPRECT lpRect, BOOL bPressed)
{
	UINT nFlags = 0;
	BOOL bSuccess;
	CDC* pDC = CDC::FromHandle(hDC);

	if (bPressed)
		nFlags = DFCS_FLAT;

	bSuccess = pDC->DrawEdge(lpRect, EDGE_RAISED, BF_RECT | BF_MIDDLE | nFlags);
	nFlags = (lpRect->right - lpRect->left) / 2 - 1 + (bPressed ? 1 : 0);
//	int dx = div(lpRect->right - lpRect->left, 2).rem - 1;
	int dx = div((long)lpRect->right - lpRect->left,(long)2).rem - 1;//new2005

	bSuccess = pDC->PatBlt(lpRect->left + nFlags - 2 + dx, lpRect->top + nFlags - 1, 7, 1, BLACKNESS);
	ASSERT(bSuccess);

	bSuccess = pDC->PatBlt(lpRect->left + nFlags - 1 + dx, lpRect->top + nFlags + 0, 5, 1, BLACKNESS);
	ASSERT(bSuccess);

	bSuccess = pDC->PatBlt(lpRect->left + nFlags - 0 + dx, lpRect->top + nFlags + 1, 3, 1, BLACKNESS);
	ASSERT(bSuccess);

	bSuccess = pDC->PatBlt(lpRect->left + nFlags + 1 + dx, lpRect->top + nFlags + 2, 1, 1, BLACKNESS);
	ASSERT(bSuccess);
}

void CVirtualGridCtrl::PressReturn()
{
	//  Is editing take place?
	if (GetEditing()) 
	{
		//  Insert the text, switch off in-cell edit.
		ApplyEdit();

		//  Set current selection to the next available cell.
//		if (m_nCursorKeys & gkReturn)
//			SetCursor(GetCursorCell(m_cellFocused, goNext), TRUE, TRUE);
	}
	else 
	{
		//  No in-cell editing?  Create it.
		if (!GetAlwaysEdit())
		{
			SetCursor(m_cellFocused, TRUE, TRUE);

			//  Show in-cell edit.
//			SetEditing(TRUE);
		}
	}
}

