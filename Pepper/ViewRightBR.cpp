#include "stdafx.h"
#include "ViewRightBR.h"

IMPLEMENT_DYNCREATE(CViewRightBR, CScrollView)

BEGIN_MESSAGE_MAP(CViewRightBR, CScrollView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CViewRightBR::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	m_pChildFrame = (CChildFrame*)GetParentFrame();

	m_pMainDoc = (CPepperDoc*)GetDocument();
	m_pLibpe = m_pMainDoc->m_pLibpe;
	if (!m_pLibpe)
		return;

	m_stListInfo.clrListTextTooltip = RGB(255, 255, 255);
	m_stListInfo.clrListBkTooltip = RGB(0, 132, 132);
	m_stListInfo.clrHeaderText = RGB(255, 255, 255);
	m_stListInfo.clrHeaderBk = RGB(0, 132, 132);
	m_stListInfo.dwHeaderHeight = 35;

	m_lf.lfHeight = 16;
	StringCchCopyW(m_lf.lfFaceName, 9, L"Consolas");
	m_stListInfo.pListLogFont = &m_lf;
	m_hdrlf.lfHeight = 17;
	m_hdrlf.lfWeight = FW_BOLD;
	StringCchCopyW(m_hdrlf.lfFaceName, 16, L"Times New Roman");
	m_stListInfo.pHeaderLogFont = &m_hdrlf;

	CreateListTLSCallbacks();
}

void CViewRightBR::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_pChildFrame)
		return;
	if (LOWORD(lHint) == IDC_HEX_RIGHT_TOP_RIGHT)
		return;

	m_fDrawRes = false;

	if (m_pActiveList)
		m_pActiveList->ShowWindow(SW_HIDE);

	CRect rect;
	GetClientRect(&rect);
	m_pChildFrame->m_stSplitterRight.GetPane(1, 0)->GetClientRect(&rect);

	switch (LOWORD(lHint))
	{
	case IDC_LIST_TLS:
		m_stListTLSCallbacks.SetWindowPos(this, 0, 0, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
		m_pActiveList = &m_stListTLSCallbacks;
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width() / 2, 0);
		break;
	case IDC_TREE_RESOURCE:
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width() / 2, 0);
		break;
	case IDC_SHOW_RESOURCE:
		ShowResource((std::vector<std::byte>*)pHint, HIWORD(lHint));
		m_pChildFrame->m_stSplitterRightBottom.ShowCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width() / 2, 0);
		break;
	default:
		m_pChildFrame->m_stSplitterRightBottom.HideCol(1);
		m_pChildFrame->m_stSplitterRightBottom.SetColumnInfo(0, rect.Width(), 0);
	}

	m_pChildFrame->m_stSplitterRightBottom.RecalcLayout();
}

int CViewRightBR::ShowResource(std::vector<std::byte>* pData, UINT uResType)
{
	HICON hIcon;
	ICONINFO iconInfo;
	if (m_hwndRes)
		::DestroyWindow(m_hwndRes);
	m_imgRes.DeleteImageList();
	m_iResTypeToDraw = -1;

	switch (uResType)
	{
	case 1: //RT_CURSOR
	{
		hIcon = CreateIconFromResourceEx((PBYTE)pData->data(), pData->size(), FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		if (!hIcon)
			return -1;
		if (!GetIconInfo(hIcon, &iconInfo))
			return -1;
		if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
			return -1;

		m_imgRes.Create(m_stBmp.bmWidth, m_stBmp.bmWidth, ILC_COLORDDB, 0, 1);
		m_imgRes.SetBkColor(m_clrBkImgList);
		if (m_imgRes.Add(hIcon) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmWidth));

		m_iResTypeToDraw = 1;
		m_fDrawRes = true;
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		break;
	}
	case 2: //RT_BITMAP
	{
		BITMAPINFO* pDIBInfo = (BITMAPINFO*)pData->data();
		int iColors = pDIBInfo->bmiHeader.biClrUsed ? pDIBInfo->bmiHeader.biClrUsed : 1 << pDIBInfo->bmiHeader.biBitCount;
		LPVOID  pDIBBits;

		if (pDIBInfo->bmiHeader.biBitCount > 8)
			pDIBBits = (LPVOID)((PDWORD)(pDIBInfo->bmiColors + pDIBInfo->bmiHeader.biClrUsed) +
			((pDIBInfo->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
		else
			pDIBBits = (LPVOID)(pDIBInfo->bmiColors + iColors);

		HDC hDC = ::GetDC(m_hWnd);
		HBITMAP hBitmap = CreateDIBitmap(hDC, &pDIBInfo->bmiHeader, CBM_INIT, pDIBBits, pDIBInfo, DIB_RGB_COLORS);
		if (!hBitmap)
			return -1;
		if (!GetObjectW(hBitmap, sizeof(BITMAP), &m_stBmp))
			return -1;

		CBitmap bmp;
		if (!bmp.Attach(hBitmap))
			return -1;
		m_imgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
		if (m_imgRes.Add(&bmp, nullptr) == -1)
			return -1;

		m_iResTypeToDraw = 2;
		m_fDrawRes = true;
		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));
		bmp.DeleteObject();
		::ReleaseDC(m_hWnd, hDC);
		break;
	}
	case 3: //RT_ICON
	{
		hIcon = CreateIconFromResourceEx((PBYTE)pData->data(), pData->size(), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		if (!hIcon)
			return -1;
		if (!GetIconInfo(hIcon, &iconInfo))
			return -1;
		if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
			return -1;
		m_imgRes.Create(m_stBmp.bmWidth, m_stBmp.bmHeight, ILC_COLORDDB, 0, 1);
		m_imgRes.SetBkColor(m_clrBkImgList);
		if (m_imgRes.Add(hIcon) == -1)
			return -1;

		SetScrollSizes(MM_TEXT, CSize(m_stBmp.bmWidth, m_stBmp.bmHeight));

		m_iResTypeToDraw = 3;
		m_fDrawRes = true;
		DeleteObject(iconInfo.hbmColor);
		DeleteObject(iconInfo.hbmMask);
		DestroyIcon(hIcon);
		break;
	}
	case 4: //RT_MENU
		break;
	case 5: //RT_DIALOG
	{
		m_hwndRes = CreateDialogIndirectParamW(NULL, (LPCDLGTEMPLATEW)pData->data(), this->m_hWnd, NULL, NULL);
		if (m_hwndRes)
		{
			CRect rcClient;
			::GetWindowRect(m_hWnd, &rcClient);
			::SetWindowPos(m_hwndRes, m_hWnd, rcClient.left, rcClient.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		m_iResTypeToDraw = 5;
		m_fDrawRes = true;
		break;
	}
	case 6: //RT_STRING
		break;
	case 12: //RT_GROUP_CURSOR
		break;
	case 14: //RT_GROUP_ICON
	{/*
		PCLIBPE_RESOURCE_ROOT_TUP pTupResRoot { };
		if (m_pLibpe->GetResourceTable(pTupResRoot) != S_OK)
			return -1;

	#pragma pack( push )
	#pragma pack( 2 )
		typedef struct
		{
			BYTE   bWidth;               // Width, in pixels, of the image
			BYTE   bHeight;              // Height, in pixels, of the image
			BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
			BYTE   bReserved;            // Reserved
			WORD   wPlanes;              // Color Planes
			WORD   wBitCount;            // Bits per pixel
			DWORD   dwBytesInRes;         // how many bytes in this resource?
			WORD   nID;                  // the ID
		} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

		typedef struct
		{
			WORD            idReserved;   // Reserved (must be 0)
			WORD            idType;       // Resource type (1 for icons)
			WORD            idCount;      // How many images?
			GRPICONDIRENTRY   idEntries[1]; // The entries for each image
		} GRPICONDIR, *LPGRPICONDIR;
	#pragma pack( pop )

		LPGRPICONDIR pGRPIDir = (LPGRPICONDIR)pData->data();
		m_iImgResWidth = 0;
		m_iImgResHeight = 0;
		for (int i = 0; i < pGRPIDir->idCount; i++)
		{
			m_iImgResWidth = max(pGRPIDir->idEntries[i].bWidth, m_iImgResWidth);
			m_iImgResHeight = max(pGRPIDir->idEntries[i].bHeight, m_iImgResHeight);
		}

		for (int i = 0; i < pGRPIDir->idCount; i++)
		{
			auto& rootvec = std::get<1>(*pTupResRoot);
			for (auto& iterRoot : rootvec)
			{
				if (std::get<0>(iterRoot).Id == 3) //RT_ICON
				{
					auto& lvl2tup = std::get<4>(iterRoot);
					auto& lvl2vec = std::get<1>(lvl2tup);

					for (auto& iterlvl2 : lvl2vec)
					{
						if (std::get<0>(iterlvl2).Id == pGRPIDir->idEntries[i].nID)
						{
							auto& lvl3tup = std::get<4>(iterlvl2);
							auto& lvl3vec = std::get<1>(lvl3tup);

							if (!lvl3vec.empty())
							{
								auto& data = std::get<3>(lvl3vec.at(0));
								if (!data.empty())
								{
									hIcon = CreateIconFromResourceEx((PBYTE)data.data(), data.size(), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
									if (!hIcon)
										return -1;
									if (!GetIconInfo(hIcon, &iconInfo))
										return -1;
									if (!GetObjectW(iconInfo.hbmMask, sizeof(BITMAP), &m_stBmp))
										return -1;

									m_iImgResWidth += m_stBmp.bmWidth;
									m_imgRes.Create(m_iImgResWidth, m_iImgResHeight, ILC_COLORDDB, pGRPIDir->idCount, pGRPIDir->idCount);
									m_imgRes.SetBkColor(RGB(255, 255, 255));

									if (m_imgRes.Add(hIcon) == -1)
										return -1;

									m_fDrawRes = true;
									DeleteObject(iconInfo.hbmColor);
									DeleteObject(iconInfo.hbmMask);
									DestroyIcon(hIcon);
									break;
								}
							}
						}
					}
				}
			}
		}
		SetScrollSizes(MM_TEXT, CSize(m_iImgResWidth, m_iImgResHeight));
*/	}
	case 16: //RT_VERSION
		break;
	}
	Invalidate();
	UpdateWindow();

	return 1;
}

void CViewRightBR::OnDraw(CDC* pDC)
{
	if (!m_fDrawRes)
		return;

	CRect rect;
	GetClientRect(&rect);
	CSize size = GetTotalSize();
	CPoint ptDrawAt;
	int x, y;

	pDC->GetClipBox(&rect);
	pDC->FillSolidRect(rect, RGB(230, 230, 230));

	switch (m_iResTypeToDraw)
	{
	case 1: //RT_CURSOR
	case 2: //RT_BITMAP
	case 3: //RT_ICON
		//Draw at center independing of scrolls.
		if (size.cx > rect.Width())
			x = size.cx / 2 - (m_stBmp.bmWidth / 2);
		else
			x = rect.Width() / 2 - (m_stBmp.bmWidth / 2);
		if (size.cy > rect.Height())
			y = size.cy / 2 - (m_stBmp.bmHeight / 2);
		else
			y = rect.Height() / 2 - (m_stBmp.bmHeight / 2);

		ptDrawAt.SetPoint(x, y);
		m_imgRes.Draw(pDC, 0, ptDrawAt, ILD_NORMAL);

		break;
	case 5: //RT_DIALOG
		break;
	}
}

void CViewRightBR::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_pActiveList)
		m_pActiveList->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CViewRightBR::CreateListTLSCallbacks()
{
	PCLIBPE_TLS_TUP pTLS { };
	if (m_pLibpe->GetTLSTable(pTLS) != S_OK)
		return -1;

	m_stListTLSCallbacks.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_LIST_TLS_CALLBACKS, &m_stListInfo);
	m_stListTLSCallbacks.SendMessageW(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_stListTLSCallbacks.InsertColumn(0, L"TLS Callbacks", LVCFMT_CENTER | LVCFMT_FIXED_WIDTH, 300);

	int listindex { };
	WCHAR str[9] { };

	for (auto& iterCallbacks : std::get<2>(*pTLS))
	{
		swprintf_s(str, 9, L"%08X", iterCallbacks);
		m_stListTLSCallbacks.InsertItem(listindex, str);
		listindex++;
	}

	return 0;
}