/****************************************************************************************
* Copyright (C) 2018-2019, Jovibor: https://github.com/jovibor/						    *
* This software is available under the "MIT License modified with The Commons Clause".  *
* https://github.com/jovibor/Pepper/blob/master/LICENSE                                 *
* This is a Hex control for MFC apps, implemented as CWnd derived class.				*
* The usage is quite simple:														    *
* 1. Construct CHexCtrl object — HEXCTRL::CHexCtrl myHex;							    *
* 2. Call myHex.Create member function to create an instance.   					    *
* 3. Call myHex.SetData method to set the data and its size to display as hex.	        *
****************************************************************************************/
#pragma once
#pragma comment(lib, "Dwmapi.lib")
#include <afxcontrolbars.h>
#include <vector>
#include <unordered_map>
#include "HexCtrlRes.h"
#include "ScrollEx.h"

namespace HEXCTRL {
	/********************************************************************************************
	* HEXCOLORSTRUCT - All HexCtrl colors.														*
	********************************************************************************************/
	struct HEXCOLORSTRUCT {
		COLORREF clrTextHex { GetSysColor(COLOR_WINDOWTEXT) };		//Hex chunks color.
		COLORREF clrTextAscii { GetSysColor(COLOR_WINDOWTEXT) };	//Ascii text color.
		COLORREF clrTextSelected { GetSysColor(COLOR_WINDOWTEXT) }; //Selected text color.
		COLORREF clrTextCaption { RGB(0, 0, 180) };					//Caption color
		COLORREF clrTextInfoRect { GetSysColor(COLOR_WINDOWTEXT) };	//Text color of the bottom "Info" rect.
		COLORREF clrTextCursor { RGB(255, 255, 255) };				//Cursor text color.
		COLORREF clrBk { GetSysColor(COLOR_WINDOW) };				//Background color.
		COLORREF clrBkSelected { RGB(200, 200, 255) };				//Background color of the selected Hex/Ascii.
		COLORREF clrBkInfoRect { RGB(250, 250, 250) };				//Background color of the bottom "Info" rect.
		COLORREF clrBkCursor { RGB(0, 0, 250) };					//Cursor background color.
	};
	using PHEXCOLORSTRUCT = HEXCOLORSTRUCT * ;
	/********************************************************************************************
	* HEXCREATESTRUCT - for CHexCtrl::Create method.												*
	********************************************************************************************/
	struct HEXCREATESTRUCT
	{
		CWnd*		    pwndParent { };			//Parent window's pointer.
		UINT		    uId { };				//Hex control Id.
		DWORD			dwExStyles { };			//Extended window styles.
		CRect			rect { };				//Initial rect. If null, the window is screen centered.
		bool			fFloat { false };		//Is float or child (incorporated into another window)?.
		const			LOGFONTW* pLogFont { };	//Font to be used, nullptr for default.
		CWnd*			pwndMsg { };			//Window ptr that is to recieve command messages, if nullptr parent window is used.
		PHEXCOLORSTRUCT pstColor { };			//Pointer to HEXCOLORSTRUCT, if nullptr default colors are used.
	};

	/********************************************************************************************
	* HEXDATASTRUCT - for CHexCtrl::SetData method.												*
	********************************************************************************************/
	struct HEXDATASTRUCT {
		ULONGLONG ullDataSize { };			//Size of the data to display, in bytes.
		ULONGLONG ullSelectionStart { };	//Set selection at this position. Works only if ullSelectionSize > 0.
		ULONGLONG ullSelectionSize { };		//How many bytes to set as selected.
		CWnd* pwndMsg { };					//Window to send the control messages to. If nullptr then the parent window is used.
		unsigned char* pData { };			//Pointer to the data. Not used if it's virtual control.
		bool fMutable { false };			//Will data be mutable (editable) or just read mode.
		bool fVirtual { false };			//Is Virtual data mode?.
	};

	/********************************************************************************************
	* HEXNOTIFYSTRUCT - used in notifications routines.											*
	********************************************************************************************/
	struct HEXNOTIFYSTRUCT
	{
		NMHDR			hdr;			//Standart Windows header.
		ULONGLONG		ullByteIndex;	//Index of the byte to draw next.
		unsigned char	chByte;			//Value of that byte to send back.
	};
	using PHEXNOTIFYSTRUCT = HEXNOTIFYSTRUCT * ;

	/********************************************
	* CHexDlgAbout class definition.			*
	********************************************/
	class CHexDlgAbout : public CDialogEx
	{
	public:
		explicit CHexDlgAbout(CWnd* m_pParent = nullptr) : CDialogEx(IDD_HEXCTRL_ABOUT) {}
		virtual ~CHexDlgAbout() {}
	protected:
		virtual BOOL OnInitDialog() override;
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		DECLARE_MESSAGE_MAP()
	private:
		bool m_fGithubLink { true };
		HCURSOR m_curHand { };
		HCURSOR m_curArrow { };
		CFont m_fontDefault;
		CFont m_fontUnderline;
		CBrush m_stBrushDefault;
		COLORREF m_clrMenu { GetSysColor(COLOR_MENU) };
	};

	/********************************************
	* CHexDlgSearch class definition.			*
	********************************************/
	class CHexCtrl;
	class CHexDlgSearch : public CDialogEx
	{
	private:
		struct HEXSEARCH
		{
			std::wstring	wstrSearch { };			//String search for.
			DWORD			dwSearchType { };		//Hex, Ascii, Unicode, etc...
			ULONGLONG		ullStartAt { };			//An offset, search should start at.
			int				iDirection { };
			bool			fWrap { false };		//Was search wrapped?
			int				iWrap { };				//Wrap direction.
			bool			fSecondMatch { false }; //First or subsequent match. 
			bool			fFound { false };
			bool			fCount { true };		//Do we count matches or just print "Found".
		};
	public:
		friend class CHexCtrl;
		explicit CHexDlgSearch(CWnd* m_pParent = nullptr) {}
		virtual ~CHexDlgSearch() {}
		BOOL Create(UINT nIDTemplate, CHexCtrl* pParentWnd);
		CHexCtrl* GetParent() const;
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		virtual BOOL OnInitDialog();
		afx_msg void OnButtonSearchF();
		afx_msg void OnButtonSearchB();
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg void OnClose();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnRadioBnRange(UINT nID);
		void SearchCallback();
		void ClearAll();
		DECLARE_MESSAGE_MAP()
	private:
		CHexCtrl* m_pParent { };
		HEXSEARCH m_stSearch { };
		DWORD m_dwOccurrences { };
		int m_iRadioCurrent { };
		COLORREF m_clrSearchFailed { RGB(200, 0, 0) };
		COLORREF m_clrSearchFound { RGB(0, 200, 0) };
		CBrush m_stBrushDefault;
		COLORREF m_clrMenu { GetSysColor(COLOR_MENU) };
	};

	/********************************************
	* CHexCtrl class definition.				*
	********************************************/
	class CHexCtrl : public CWnd
	{
	public:
		friend class CHexDlgSearch;
		CHexCtrl();
		virtual ~CHexCtrl() {}
		bool Create(const HEXCREATESTRUCT& hcs); //Main initialization method, CHexCtrl::Create.
		bool IsCreated();						 //Shows whether control created or not.
		void SetData(const HEXDATASTRUCT& hds);  //Main method for setting data to display (and edit).																
		void ClearData();						 //Clears all data from HexCtrl's view (not touching data itself).
		void ShowOffset(ULONGLONG ullOffset, ULONGLONG ullSize = 1); //Shows (selects) given offset.
		void SetFont(const LOGFONT* pLogFontNew);//Sets the control's font.
		void SetFontSize(UINT uiSize);			 //Sets the control's font size.
		UINT GetFontSize();						 //Gets the control's font size.
		void SetColor(const HEXCOLORSTRUCT& clr);//Sets all the colors for the control.
		void SetCapacity(DWORD dwCapacity);		 //Sets the control's current capacity.
		int GetDlgCtrlID() const;
		CWnd* GetParent() const;
	protected:
		enum HEXCTRL_SHOWAS { ASBYTE = 1, ASWORD = 2, ASDWORD = 4, ASQWORD = 8 };
		DECLARE_MESSAGE_MAP()
		void OnDraw(CDC* pDC) {} //All drawing is in OnPaint.
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnPaint();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg BOOL OnNcActivate(BOOL bActive);
		afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
		afx_msg void OnNcPaint();
		afx_msg void OnDestroy();
		void RecalcAll();
		void RecalcWorkAreaHeight(int iClientHeight);
		void RecalcScrollSizes(int iClientHeight = 0, int iClientWidth = 0);
		void RecalcScrollPageSize();
		ULONGLONG GetCurrentLineV();
		ULONGLONG HitTest(LPPOINT); //Is any hex chunk withing given point?
		void HexPoint(ULONGLONG ullChunk, ULONGLONG& ullCx, ULONGLONG& ullCy);
		void CopyToClipboard(UINT nType);
		void Search(CHexDlgSearch::HEXSEARCH& rSearch);
		void SetSelection(ULONGLONG ullClick, ULONGLONG ullStart, ULONGLONG ullSize, bool fHighlight = false);
		void SelectAll();
		void UpdateInfoText();
		void ToWchars(ULONGLONG ull, wchar_t* pwsz, DWORD dwBytes = 4);
		void SetShowAs(HEXCTRL_SHOWAS enShowAs);
		void SetSingleByteData(ULONGLONG ullByte, BYTE chData, bool fWhole = true, bool fHighPart = true, bool fMoveNext = true);
		void SetCursorPos(ULONGLONG ullPos, bool fHighPart); //Sets the cursor position when in Edit mode.
		void CursorMoveRight();
		void CursorMoveLeft();
		void CursorMoveUp();
		void CursorMoveDown();
		void CursorScroll();
	private:
		bool m_fCreated { false };			//Is control created or not yet.
		bool m_fFloat { false };			//Is control window float or not.
		bool m_fVirtual { false };			//Is control works in "Virtual" mode.
		bool m_fMutable { false };			//Is control works in Edit mode.
		unsigned char* m_pData { };			//Modifiable in "Edit" mode.
		ULONGLONG m_ullDataSize { };		//Size of the displayed data in bytes.
		DWORD m_dwCapacity { 16 };			//How many bytes displayed in one row
		const DWORD m_dwCapacityMax { 64 }; //Maximum capacity.
		DWORD m_dwCapacityBlockSize { m_dwCapacity / 2 }; //Size of block before space delimiter.
		HEXCTRL_SHOWAS m_enShowAs { HEXCTRL_SHOWAS::ASBYTE }; //Show data mode.
		CWnd* m_pwndParentOwner { };		//Parent or owner window pointer.
		CWnd* m_pwndMsg { };				//Window the control messages will be sent to.
		SIZE m_sizeLetter { 1, 1 };			//Current font's letter size (width, height).
		CFont m_fontHexView;				//Main Hex chunks font.
		CFont m_fontBottomRect;				//Font for bottom Info rect.
		CHexDlgSearch m_dlgSearch;			//Search dialog.
		CHexDlgAbout m_dlgAbout;			//About dialog.
		CScrollEx m_stScrollV;				//Vertical scroll object.
		CScrollEx m_stScrollH;				//Horizontal scroll object.
		CMenu m_menuMain;					//Main popup menu.
		CMenu m_menuSubShowAs;				//Submenu "Show as..."
		HEXCOLORSTRUCT m_stColor;			//All control related colors.
		CBrush m_stBrushBkSelected;			//Brush for "selected" background.
		CPen m_penLines { PS_SOLID, 1, RGB(200, 200, 200) };
		int m_iSizeFirstHalf { };		    //Size of first half of capacity.
		int m_iSizeHexByte { };			    //Size of two hex letters representing one byte.
		int m_iIndentAscii { };			    //Indent of Ascii text begining.
		int m_iIndentFirstHexChunk { };	    //First hex chunk indent.
		int m_iIndentTextCapacityY { };	    //Caption text (0 1 2... D E F...) vertical offset.
		int m_iIndentBottomLine { 1 };	    //Bottom line indent from window's bottom.
		int m_iDistanceBetweenHexChunks { };//Distance between begining of two hex chunks.
		int m_iSpaceBetweenHexChunks { };   //Space between Hex chunks.
		int m_iSpaceBetweenAscii { };	    //Space between two Ascii chars.
		int m_iSpaceBetweenBlocks { };	    //Additional space between hex chunks after half of capacity.
		int m_iHeightTopRect { };		    //Height of the header where offsets (0 1 2... D E F...) reside.
		int m_iHeightBottomRect { 22 };	    //Height of bottom Info rect.
		int m_iHeightBottomOffArea { m_iHeightBottomRect + m_iIndentBottomLine }; //Height of not visible rect from window's bottom to m_iThirdHorizLine.
		int m_iHeightWorkArea { };		    //Needed for mouse selection point.y calculation.
		int m_iFirstVertLine { }, m_iSecondVertLine { }, m_iThirdVertLine { }, m_iFourthVertLine { }; //Vertical lines indent.
		ULONGLONG m_ullSelectionStart { }, m_ullSelectionEnd { }, m_ullSelectionClick { }, m_ullBytesSelected { };
		const wchar_t* const m_pwszHexMap { L"0123456789ABCDEF" };
		const char* const m_pszHexMap { "0123456789ABCDEF" };
		std::unordered_map<unsigned, std::wstring> m_umapCapacity;
		std::wstring m_wstrBottomText { };  //Info text (bottom rect).
		const std::wstring m_wstrErrVirtual { L"This function isn't supported in Virtual mode!" };
		bool m_fLMousePressed { false };
		UINT m_dwCtrlId { };				//Id of the control.
		DWORD m_dwOffsetDigits { 8 };		//Amount of digits in "Offset", depends on data size set in SetData.
		ULONGLONG m_ullCursorPos { };		//Current cursor position.
		bool m_fCursorHigh { true };		//Cursor's High or Low bits position (first or last digit in hex chunk).
		bool m_fCursorAscii { false };		//Whether cursor at Ascii or Hex chunks area.

		/////////////////////////Enums///////////////////////////////////////////////
		enum HEXCTRL_CLIPBOARD { COPY_HEX, COPY_HEXFORMATTED, COPY_ASCII };
		enum HEXCTRL_MENU {
			IDM_MAIN_SEARCH, IDM_MAIN_COPYASHEX, IDM_MAIN_COPYASHEXFORMATTED, IDM_MAIN_COPYASASCII, IDM_MAIN_ABOUT,
			IDM_SUB_SHOWASBYTE, IDM_SUB_SHOWASWORD, IDM_SUB_SHOWASDWORD, IDM_SUB_SHOWASQWORD
		};
		enum HEXCTRL_SEARCH {
			SEARCH_HEX, SEARCH_ASCII, SEARCH_UNICODE,
			SEARCH_FORWARD, SEARCH_BACKWARD,
			SEARCH_NOTFOUND, SEARCH_FOUND,
			SEARCH_BEGINNING, SEARCH_END,
		};
	};

	/************************************************
	* WM_NOTIFY message codes (NMHDR.code values)	*
	************************************************/

	constexpr auto HEXCTRL_MSG_DESTROY = 0x00FF;
	constexpr auto HEXCTRL_MSG_GETDATA = 0x0100;
	constexpr auto HEXCTRL_MSG_SETDATA = 0x0101;

	//Version string.
	constexpr auto HEXCTRL_VERSION_WSTR = L"Hex Control for MFC, v2.0.1";
};