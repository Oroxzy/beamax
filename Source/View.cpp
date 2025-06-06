﻿
#include <math.h>
#include "stdafx.h"
#include "Dialog.h"
#include "Object.h"
#include "Document.h"
#include "View.h"
#include <gdiplus.h>
#include <vector>
#include <string>
#include <set>
#include <cmath>
#include <algorithm>

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

IMPLEMENT_DYNCREATE(View, CScrollView)

BEGIN_MESSAGE_MAP(View, CScrollView)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_ERASEBKGND()
    ON_COMMAND(IDM_DELETE, OnDelete)
    ON_UPDATE_COMMAND_UI(IDM_DELETE, OnUpdateDelete)
    ON_COMMAND(IDM_FIXED_SUPPORT, OnCreateFixedSupport)
    ON_COMMAND(IDM_HINGED_SUPPORT, OnCreateHingedSupport)
    ON_COMMAND(IDM_ROLLER_SUPPORT, OnCreateRollerSupport)
    ON_COMMAND(IDM_POINT_LOAD, OnCreatePointLoad)
    ON_COMMAND(IDM_LINEAR_DISTRIBUTED_LOAD, OnCreateLinearDistributedLoad)
    ON_COMMAND(IDM_VIEW_CONTINUOUS_BEAM, OnViewContinuousBeam)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_CONTINUOUS_BEAM, OnUpdateViewContinuousBeam)
    ON_COMMAND(IDM_VIEW_SHEAR_FORCE, OnViewShearForce)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_SHEAR_FORCE, OnUpdateViewShearForce)
    ON_COMMAND(IDM_VIEW_BENDING_MOMENT, OnViewBendingMoment)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_BENDING_MOMENT, OnUpdateViewBendingMoment)
    ON_COMMAND(IDM_VIEW_DISPLACEMENT, OnViewDisplacement)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_DISPLACEMENT, OnUpdateViewDisplacement)
    ON_COMMAND(IDM_VIEW_NUMERICAL_VALUES, OnViewNumericalValues)
    ON_UPDATE_COMMAND_UI(IDM_VIEW_NUMERICAL_VALUES, OnUpdateViewNumericalValues)
    ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
END_MESSAGE_MAP()

View::View() : _document(nullptr)  // <– so wird korrekt initialisiert
{
    // create popup menu
    _popupMenu = new CMenu;
    _popupMenu->CreatePopupMenu();
    _popupMenu->AppendMenu(MF_STRING, IDM_FIXED_SUPPORT, "&Fixed Support\tAlt+X");
    _popupMenu->AppendMenu(MF_STRING, IDM_HINGED_SUPPORT, "&Hinged Support\tAlt+N");
    _popupMenu->AppendMenu(MF_STRING, IDM_ROLLER_SUPPORT, "&Roller Support\tAlt+R");
    _popupMenu->AppendMenu(MF_SEPARATOR);
    _popupMenu->AppendMenu(MF_STRING, IDM_POINT_LOAD, "&Point Load\tAlt+P");
    _popupMenu->AppendMenu(MF_STRING, IDM_LINEAR_DISTRIBUTED_LOAD, "&Distributed Load\tAlt+L");
    _popupMenu->AppendMenu(MF_SEPARATOR);
    _popupMenu->AppendMenu(MF_STRING, IDM_DELETE, "&Delete\tDel");

    // default view settings
    _viewContinuousBeam = TRUE;
    _viewShearForce = FALSE;
    _viewBendingMoment = TRUE;
    _viewDisplacement = FALSE;
    _viewNumericalValues = FALSE;
}

View::~View()
{
    // delete popup menu
    delete _popupMenu;
}

void View::OnInitialUpdate()
{
    srand((unsigned)time(NULL));  // <<< NEU: Zufallszahlengenerator initialisieren

    CScrollView::OnInitialUpdate();
    SetScrollSizes(MM_TEXT, CSize(100, 100));

    _document = (Document*)m_pDocument;

    _viewShearForce = TRUE;
    _viewBendingMoment = TRUE;
    _viewNumericalValues = TRUE;
}

void View::LPtoDP(CRect& rect)
{
    CClientDC dc(this);
    OnPrepareDC(&dc, NULL);
    dc.LPtoDP(&rect);
    rect.NormalizeRect();
}

void View::OnDraw(CDC* pDrawDC)
{
    CDC* pDC = pDrawDC;

    // clipbox in logical units
    CRect rectLogical;
    pDC->GetClipBox(rectLogical);

    // clipbox in device units
    CRect rectDevice = rectLogical;
    LPtoDP(rectDevice);

    CDC dc;
    CBitmap bmp;
    CBitmap* pbmpOld = NULL;

    // use swap buffer
    if (/*(!pDC->IsPrinting()) &&*/ (dc.CreateCompatibleDC(pDC)) &&
        (bmp.CreateCompatibleBitmap(pDC, rectDevice.Width(), rectDevice.Height())))
    {
        OnPrepareDC(&dc, NULL);
        pDC = &dc;
        dc.OffsetViewportOrg(-rectDevice.left, -rectDevice.top);
        pbmpOld = dc.SelectObject(&bmp);
        dc.SetBrushOrg(rectDevice.left % 8, rectDevice.top % 8);
        dc.IntersectClipRect(rectLogical);
    }

    // draw all the things
    CBrush brush(::GetSysColor(COLOR_WINDOW));
    pDC->FillRect(rectLogical, &brush);
    Draw(pDC, pDrawDC);

    // BitBlt Bitmap into View
    if (pDrawDC != pDC)
    {
        pDrawDC->SetViewportOrg(0, 0);
        pDrawDC->SetWindowOrg(0, 0);
        pDrawDC->SetMapMode(MM_TEXT);
        dc.SetViewportOrg(0, 0);
        dc.SetWindowOrg(0, 0);
        dc.SetMapMode(MM_TEXT);
        pDrawDC->BitBlt(rectDevice.left, rectDevice.top, rectDevice.Width(), rectDevice.Height(), &dc, 0, 0, SRCCOPY);
        dc.SelectObject(pbmpOld);
    }
}

BOOL View::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void View::Draw(CDC* pDC, CDC* pDrawDC)
{
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // let the view background be transparent
    pDC->SetBkMode(TRANSPARENT);

    // cound number of views
    int views = 0;
    if (_viewShearForce)
    {
        views++;
    }
    if (_viewBendingMoment)
    {
        views++;
    }
    if (_viewDisplacement) 
    {
        views++;
    }

    // if something to draw
    if ((views != 0) || (_viewContinuousBeam))
    {
        // calculate virtual window width
        CRect rect;
        GetClientRect(&rect);
        int width = rect.Width();
        if (width < 400)
        {
            width = 400;
        }

        // calculate horizontal scale
        double borderX = _document->_beamLength / 6;
        double scaleX = width / (_document->_beamLength + 2 * borderX);
        int beamX = (int)(borderX * scaleX);

        // calculate height of the beam
        int beamHeight;
        if (_viewContinuousBeam)
        {
            beamHeight = 40 + (_document->SortLoadLevels(pDC, scaleX) * 30) + 32;
        }
        else
        {
            beamHeight = 0;
        }

        // calculate virtual window height
        int height = rect.Height() - 24;
        if (height < (beamHeight + (views * 80)))
        {
            height = beamHeight + (views * 80);
        }

        // height of one view
        int viewHeight = 0;
        if (views != 0)
        {
            viewHeight = (height - beamHeight) / views;
        }

        // select new black drawing pencil and font
        CPen newPen(PS_SOLID, 1, RGB(0, 0, 0));
        CPen* oldPen = pDC->SelectObject(&newPen);
        CFont newFont;
        newFont.CreateFont(
            -16, 0, 0, 0,              // Höhe: -20 entspricht ca. 14pt
            FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

        CFont* oldFont = (CFont *)pDC->SelectObject(&newFont);
        pDC->SetTextAlign(TA_LEFT | TA_TOP);

        if (pDrawDC->IsPrinting())
        {
            pDC->TextOut(0, 0, "Beamax");
        }

        // draw the continuous beam
        if (_viewContinuousBeam == TRUE)
        {
            int beamY = beamHeight - 32;

            // draw the beam object/line
            // GDI+ Initialisierung (lokal)
            Graphics graphics(pDC->GetSafeHdc());
            graphics.SetSmoothingMode(SmoothingModeAntiAlias);
            graphics.SetCompositingQuality(CompositingQualityHighQuality);

            // schwarzer Pen, 1 Pixel
            Pen pen(Color(255, 0, 0, 0), 1.5f);  // RGBA (voll Deckkraft, schwarz)

            // Linie zeichnen
            PointF start((REAL)beamX, (REAL)beamY);
            PointF end((REAL)(beamX + (REAL)(_document->_beamLength * scaleX)), (REAL)beamY);
            graphics.DrawLine(&pen, start, end);

            // draw the support and load objects
            ObjectCast* item;
            POSITION position = _document->_objectList.GetHeadPosition();
            while (position != NULL)
            {
                item = (ObjectCast*)_document->_objectList.GetNext(position);
                if (item != NULL)
                {
                    item->Draw(pDC, beamX, beamY, scaleX);
                    if (CLASSOF(item, "FixedSupport") || CLASSOF(item, "HingedSupport") || CLASSOF(item, "RollerSupport"))
                    {
                        SupportCast* support = (SupportCast*)item;
                        double pos = support->GetPosition();

                        double reaction = _document->GetSupportReactionAt(pos);
                        double maxReaction = _document->GetMaxReaction();

                        double absReaction = fabs(reaction);

                        bool isMax = (fabs(maxReaction) > EPSILON) && (fabs(reaction) >= fabs(maxReaction) - 1e-6);
                        COLORREF color = isMax ? RGB(220, 0, 0) : RGB(0, 0, 0);

                        DrawReactionValue(pDC, beamX + (int)(pos * scaleX), beamY + 26, reaction, color);
                        pDC->SetTextColor(RGB(0, 0, 0)); // Sicherheit
                    }
                }
            }

            // enable bound box
            item = (ObjectCast*)_document->_selected;
            if (item != NULL)
            {
                item->GetBoundRect().Draw(pDC);
            }

            // Einheit + Maxwert der Auflagerreaktionen anzeigen
            char rmaxLabel[64];
            sprintf_s(rmaxLabel, "[kN]  Rmax = %.2f", _document->GetMaxReaction());

            CSize textSize = pDC->GetTextExtent(rmaxLabel, (int)strlen(rmaxLabel));

            // ⬅️ Ausrichtung ändern: Text soll rechts außen stehen
            pDC->SetTextAlign(TA_LEFT | TA_TOP);

            // ⬅️ Position nach rechts vom Balken verschieben
            int x = beamX + (int)(_document->_beamLength * scaleX) + textSize.cx / 4;
            int y = beamY + 4;  // direkt unterhalb vom Strich (Balkenlinie)

            pDC->SetTextColor(RGB(0, 0, 0));  // Farbe zurücksetzen auf schwarz
            pDC->TextOut(x, y, rmaxLabel);
        }

        // get fixed degrees of freedom
        int fixedDegreesOfFreedom = 0;
        SupportCast * object;
        POSITION position = _document->_objectList.GetHeadPosition();
        while (position != NULL)
        {
            object = (SupportCast *)_document->_objectList.GetNext(position);
            if (object != NULL)
            {
                if (CLASSOF(object, "FixedSupport"))
                {
                    fixedDegreesOfFreedom += 3;
                }
                if (CLASSOF(object, "HingedSupport"))
                {
                    fixedDegreesOfFreedom += 2;
                }
                if (CLASSOF(object, "RollerSupport")) 
                {
                    fixedDegreesOfFreedom += 1;
                }
            }
        }

        // start analysis and draw result
        if ((views > 0) && (fixedDegreesOfFreedom >= 3))
        {
            if (!_document->Analyse())
            {
                AfxMessageBox("Could not run analysis.");
            }
            else
            {
                DrawResults(pDC, beamX, beamHeight + (viewHeight / 2), scaleX, viewHeight, views);
            }
        }

        // restore old drawing pen and font
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldFont);
    }
}

void View::OnLButtonDown(UINT flags, CPoint point)
{
    POSITION position;
    ObjectCast* object;

    _document->_selected = NULL;

    // scan object list
    position = _document->_objectList.GetHeadPosition();
    while ((position != NULL) && (_document->_selected == NULL))
    {
        object = (ObjectCast*)_document->_objectList.GetNext(position);
        if (object->GetBoundRect().m_rect.PtInRect(point))
        {
            _document->_selected = object;
        }
    }

    RedrawWindow();
    CView::OnLButtonDown(flags, point);
}

void View::OnRButtonDown(UINT flags, CPoint point)
{
    // toggle delete options
    _popupMenu->EnableMenuItem(IDM_DELETE, MF_BYCOMMAND | (_document->_selected != NULL ? MF_ENABLED : MF_GRAYED));

    // track popup menu
    CRect rect;
    GetWindowRect(&rect);
    CPoint position = rect.TopLeft() + CSize(point);
    _popupMenu->TrackPopupMenu(TPM_LEFTALIGN, position.x, position.y, this, rect);

    CView::OnRButtonDown(flags, point);
}


void View::OnLButtonDblClk(UINT flags, CPoint point)
{
    if (_document->_selected == NULL)
    {
        return;
    }
    if (CLASSOF(_document->_selected, "FixedSupport"))
    {
        OnEditFixedSupport();
    }
    if (CLASSOF(_document->_selected, "HingedSupport"))
    {
        OnEditHingedSupport();
    }
    if (CLASSOF(_document->_selected, "RollerSupport"))
    {
        OnEditRollerSupport();
    }
    if (CLASSOF(_document->_selected, "PointLoad"))
    {
        OnEditPointLoad();
    }
    if (CLASSOF(_document->_selected, "LinearDistributedLoad"))
    {
        OnEditLinearDistributedLoad();
    }
    CView::OnLButtonDblClk(flags, point);
}

void View::OnCreateFixedSupport()
{
    FixedSupportDialog dialog;

    dialog.SetPosition(TRUE);
    if (dialog.DoModal() == IDOK)
    {
        double position = 0;
        if (!dialog.GetPosition())
        {
            position = _document->_beamLength;
        }
        if (!_document->CreateFixedSupport(position))
        {
            MessageBox("Could not create fixed support.", "Error", MB_OK | MB_ICONSTOP);
        }
    }

    _document->UpdateAllViews(NULL);
}

void View::OnEditFixedSupport()
{
    FixedSupportDialog dialog;

    FixedSupport* item = (FixedSupport*)_document->_selected;
    dialog.SetPosition(TRUE);
    if (item->GetPosition() != 0)
    {
        dialog.SetPosition(FALSE);
    }
    if (dialog.DoModal() == IDOK)
    {
        item->SetPosition(0);
        if (dialog.GetPosition() != TRUE)
        {
            item->SetPosition(_document->_beamLength);
        }
    }

    _document->UpdateAllViews(NULL);
}

void View::OnCreateHingedSupport()
{
    HingedSupportDialog dialog;

    dialog.SetPosition(0);
    if (dialog.DoModal() == IDOK)
    {
        if (!_document->CreateHingedSupport(dialog.GetPosition()))
        {
            MessageBox("Could not create hinged support.", "Error", MB_OK | MB_ICONSTOP);
        }
    }

    _document->UpdateAllViews(NULL);
}

void View::OnEditHingedSupport()
{
    HingedSupportDialog dialog;

    HingedSupport* item = (HingedSupport*)_document->_selected;
    dialog.SetPosition(item->GetPosition());
    if (dialog.DoModal() == IDOK)
    {
        double position = dialog.GetPosition();
        if ((position < 0) || (position > _document->_beamLength))
        {
            MessageBox("Support position not in beam range.", "Error", MB_OK | MB_ICONSTOP);
        }
        else
        {
            item->SetPosition(position);
            _document->UpdateAllViews(NULL);
        }
    }
}

void View::OnCreateRollerSupport()
{
    RollerSupportDialog dialog;

    dialog.SetPosition(0);
    if (dialog.DoModal() == IDOK)
    {
        if (!_document->CreateRollerSupport(dialog.GetPosition()))
        {
            MessageBox("Could not create roller support.", "Error", MB_OK | MB_ICONSTOP);
        }
    }

    _document->UpdateAllViews(NULL);
}

void View::OnEditRollerSupport()
{
    RollerSupportDialog dialog;

    RollerSupport* item = (RollerSupport*)_document->_selected;
    dialog.SetPosition(item->GetPosition());
    if (dialog.DoModal() == IDOK)
    {
        double position = dialog.GetPosition();
        if ((position < 0) || (position > _document->_beamLength))
        {
            MessageBox("Support position not in beam range.", "Error", MB_OK | MB_ICONSTOP);
        }
        else
        {
            item->SetPosition(position);
            _document->UpdateAllViews(NULL);
        }
    }
}

void View::OnCreatePointLoad()
{
    PointLoadDialog dialog;

    dialog.SetPosition(0);
    dialog.SetValue(1);
    if (dialog.DoModal() == IDOK)
    {
        if (!_document->CreatePointLoad(dialog.GetPosition(), dialog.GetValue()))
        {
            MessageBox("Could not create point load.", "Error", MB_OK | MB_ICONSTOP);
        }
    }

    _document->UpdateAllViews(NULL);
}

void View::OnEditPointLoad()
{
    PointLoadDialog dialog;

    PointLoad* item = (PointLoad*)_document->_selected;
    dialog.SetPosition(item->GetPosition());
    dialog.SetValue(item->GetValue());
    if (dialog.DoModal() == IDOK)
    {
        double position = dialog.GetPosition();
        if ((position < 0) || (position > _document->_beamLength))
        {
            MessageBox("Point load not in beam range.", "Error", MB_OK | MB_ICONSTOP);
        }
        else
        {
            item->SetPosition(position);
            item->SetValue(dialog.GetValue());
            _document->UpdateAllViews(NULL);
        }
    }
}

void View::OnCreateLinearDistributedLoad()
{
    LinearDistributedLoadDialog dialog;

    dialog.SetPosition(1.0);
    dialog.SetValueStart(1.0);
    dialog.SetValueEnd(1.0);
    dialog.SetLength(1.0);
    if (dialog.DoModal() == IDOK)
    {
        if (!_document->CreateLinearDistributedLoad(dialog.GetPosition(), dialog.GetValueStart(), dialog.GetValueEnd(), dialog.GetLength()))
        {
            MessageBox("Could not create linear distributed load.", "Error", MB_OK | MB_ICONSTOP);
        }
    }

    _document->UpdateAllViews(NULL);
}

void View::OnEditLinearDistributedLoad()
{
    LinearDistributedLoadDialog dialog;

    LinearDistributedLoad* item = (LinearDistributedLoad*)_document->_selected;
    dialog.SetPosition(item->GetPosition());
    dialog.SetValueStart(item->GetValueStart());
    dialog.SetValueEnd(item->GetValueEnd());
    dialog.SetLength(item->GetLength());
    if (dialog.DoModal() == IDOK)
    {
        double position = dialog.GetPosition();
        double length = dialog.GetLength();
        if ((length < 0) || (position < 0) || ((position + length) > _document->_beamLength))
        {
            MessageBox("Linear distributed load not in beam range.", "Error", MB_OK | MB_ICONSTOP);
        }
        else
        {
            item->SetPosition(position);
            item->SetValueStart(dialog.GetValueStart());
            item->SetValueEnd(dialog.GetValueEnd());
            item->SetLength(length);
            _document->UpdateAllViews(NULL);
        }
    }
}

void View::OnDelete(void)
{
    if (_document->_selected != NULL)
    {
        _document->DeleteObject((ObjectCast*)_document->_selected);
        _document->_selected = NULL;
        _document->Analyse();  // <--- Das fehlte vermutlich
    }

    _document->UpdateAllViews(NULL);
}

void View::OnUpdateDelete(CCmdUI* pCmdUI)
{
    if (_document->_selected != NULL)
    {
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void View::OnViewContinuousBeam()
{
    if (_viewContinuousBeam)
    {
        _viewContinuousBeam = FALSE;
    }
    else
    {
        _viewContinuousBeam = TRUE;
    }

    _document->UpdateAllViews(NULL);
}

void View::OnUpdateViewContinuousBeam(CCmdUI* pCmdUI)
{
    if (_viewContinuousBeam)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void View::OnViewShearForce()
{
    if (_viewShearForce)
    {
        _viewShearForce = FALSE;
    }
    else
    {
        _viewShearForce = TRUE;
    }

    _document->UpdateAllViews(NULL);
}

void View::OnUpdateViewShearForce(CCmdUI* pCmdUI)
{
    if (_viewShearForce)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void View::OnViewBendingMoment()
{
    if (_viewBendingMoment)
    {
        _viewBendingMoment = FALSE;
    }
    else
    {
        _viewBendingMoment = TRUE;
    }

    _document->UpdateAllViews(NULL);
}

void View::OnUpdateViewBendingMoment(CCmdUI* pCmdUI)
{
    if (_viewBendingMoment)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void View::OnViewDisplacement()
{
    if (_viewDisplacement)
    {
        _viewDisplacement = FALSE;
    }
    else
    {
        _viewDisplacement = TRUE;
    }

    _document->UpdateAllViews(NULL);
}

void View::OnUpdateViewDisplacement(CCmdUI* pCmdUI)
{
    if (_viewDisplacement)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void View::OnViewNumericalValues()
{
    if (_viewNumericalValues)
    {
        _viewNumericalValues = FALSE;
    }
    else
    {
        _viewNumericalValues = TRUE;
    }
    _document->UpdateAllViews(NULL);
}

void View::OnUpdateViewNumericalValues(CCmdUI* pCmdUI)
{
    if (_viewNumericalValues)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

BOOL View::OnPreparePrinting(CPrintInfo* pPrintInfo)
{
    return DoPreparePrinting(pPrintInfo);
}

void View::OnBeginPrinting(CDC* pDC, CPrintInfo* pPrintInfo)
{
}

void View::OnEndPrinting(CDC* pDC, CPrintInfo* pPrintInfo)
{
}

void View::OnPrepareDC(CDC* pDC, CPrintInfo* pPrintInfo)
{
    CScrollView::OnPrepareDC(pDC, pPrintInfo);

    if (pDC->IsPrinting())
    {
        pDC->SetMapMode(MM_ANISOTROPIC);
        CRect r;
        GetClientRect(&r);
        CSize w = r.Size();
        CSize v(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
        v.cy = (int)((v.cx * w.cy) / w.cx);
        pDC->SetWindowExt(w);
        pDC->SetViewportExt(v);
    }
}

double View::GetMaximum(Section* object)
{
    if ((object->A4 != 0) || (object->A3 != 0) || (object->A2 != 0))
    {
        // start at section / 2
        double x = object->Length / 2;

        // newton iteration
        for (int i = 0; i < 32; i++)
        {
            x = x - ((4 * object->A4 * x * x * x + 3 * object->A3 * x * x +
                2 * object->A2 * x + object->A1) /
                (12 * object->A4 * x * x + 6 * object->A3 * x +
                    2 * object->A2));
        }

        if ((x > 0) && (x < object->Length))
        {
            return x;
        }
    }
    return 0;
}

double View::SolvePolynom(double x, Section* object)
{
    if (object->A4 == 0)
    {
        if (object->A3 == 0)
        {
            if (object->A2 == 0)
            {
                if (object->A1 == 0)
                {
                    return (object->A0);
                }
                return (object->A1 * x + object->A0);
            }
            return (object->A2 * x * x + object->A1 * x + object->A0);
        }
        return (object->A3 * x * x * x + object->A2 * x * x + object->A1 * x + object->A0);
    }
    return (object->A4 * x * x * x * x + object->A3 * x * x * x + object->A2 * x * x + object->A1 * x + object->A0);
}

BOOL View::IsRectEmpty(CDC* pDC, int x1, int y1, int x2, int y2)
{
    COLORREF Background = pDC->GetBkColor();
    for (int x = (x1 - 1); x < (x2 + 1); x++)
    {
        for (int y = (y1 - 1); y < (y2 + 1); y++)
        {
            if (pDC->GetPixel(x, y) != Background)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void View::DrawValue(CDC* pDC, int x, int y, BOOL mirror, double value, COLORREF color)
{
    if (mirror) value *= -1;

    char buffer[32];
    sprintf_s(buffer, "%.2f", value);

    CSize textSize = pDC->GetTextExtent(buffer, (int)strlen(buffer));
    int x1 = x - textSize.cx / 2;
    int x2 = x + textSize.cx / 2;

    pDC->SetTextColor(color);

    const int maxTries = 6;
    const int spacing = 14;

    // Richtungspriorität: positiver Wert => unten (TOP), negativer => oben (BOTTOM)
    bool preferBottom = value >= 0;

    for (int i = 0; i < maxTries; ++i)
    {
        bool tryBottom = (i % 2 == 0) ? preferBottom : !preferBottom;
        int offset = ((i + 1) / 2) * spacing;

        int yTry = tryBottom ? y + offset : y - offset;
        int y1 = tryBottom ? yTry : yTry - textSize.cy;
        int y2 = tryBottom ? yTry + textSize.cy : yTry;

        pDC->SetTextAlign(TA_CENTER | (tryBottom ? TA_TOP : TA_BOTTOM));

        if (IsRectEmpty(pDC, x1, y1, x2, y2))
        {
            pDC->TextOut(x, yTry, buffer);
            return;
        }
    }

    // Fallback: trotzdem anzeigen, falls keine freie Stelle
    pDC->SetTextAlign(TA_CENTER | TA_TOP);
    pDC->TextOut(x, y, buffer);
}

void View::DrawReactionValue(CDC* pDC, int x, int y, double value, COLORREF color)
{
    char buffer[32];
    sprintf_s(buffer, "%.2f", value);

    pDC->SetTextColor(color);
    pDC->SetTextAlign(TA_CENTER | TA_BASELINE);
    pDC->TextOut(x, y, buffer);
}

// Hauptfunktion zum Zeichnen einer Ansicht mit Beschriftung, Farbflächen und Werten
void View::DrawView(CDC* pDC, int beamX, int beamY, double scaleX, int viewHeight, BOOL mirror, BOOL values, double unitScale, char* unitName, char* viewName, CObList* sectionList)
{
    // Zeichnet die horizontale Linie des Trägers
    DrawBeam(pDC, beamX, beamY, scaleX);

    // Zeichnet den Namen der Ansicht mit Rahmen
    DrawViewName(pDC, beamX, beamY, viewName);

    // Ermittelt Einheitstext und Extremwerte für spätere Verwendung
    double maxForColor = 0.0;
    double minForColor = 0.0;
    CString unitLabel = GetUnitLabel(viewName, unitName, sectionList, maxForColor, minForColor);

    // Zeichnet den Einheiten-Text am rechten Rand
    DrawUnitLabel(pDC, beamX, beamY, scaleX, unitLabel);

    // Berechnet den größten darzustellenden Betrag zur Skalierung in Y-Richtung
    double maximum = CalculateMaxValue(sectionList);
    double scaleY = (viewHeight / maximum) * 0.36 * (mirror ? -1 : 1);

    // Initialisiert GDI+ für hochwertige Grafikdarstellung
    Graphics graphics(pDC->GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetCompositingQuality(CompositingQualityHighQuality);

    // Zeichnet die Kurven (inkl. Fläche, Start/Endlinien)
    DrawCurves(graphics, sectionList, beamX, beamY, scaleX, scaleY, viewName);

    // Optional: Zeichnet numerische Werte direkt an die Kurve
    if (values)
    {
        DrawNumericalValues(pDC, sectionList, beamX, beamY, scaleX, scaleY, mirror, unitScale, maxForColor, viewName);
    }
}

// Zeichnet die horizontale Linie für den Träger
void View::DrawBeam(CDC* pDC, int beamX, int beamY, double scaleX)
{
    int beamLengthPx = (int)(_document->_beamLength * scaleX);
    pDC->MoveTo(beamX, beamY);
    pDC->LineTo(beamX + beamLengthPx, beamY);
}

// Zeichnet den Ansichtsnamen links neben dem Träger mit Rahmen
void View::DrawViewName(CDC* pDC, int beamX, int beamY, const char* viewName)
{
    CSize textSize = pDC->GetTextExtent(viewName, (int)strlen(viewName));
    int left = beamX - textSize.cx - 20;
    int right = beamX - 12;
    int top = beamY - (textSize.cy / 2) - 3;
    int bottom = beamY + (textSize.cy / 2) + 3;

    pDC->MoveTo(left, top);
    pDC->LineTo(right, top);
    pDC->LineTo(right, bottom);
    pDC->LineTo(left, bottom);
    pDC->LineTo(left, top);
    pDC->TextOut(left + 4, beamY - (textSize.cy / 2), viewName);
}

// Liefert das Beschriftungslabel und bestimmt Extremwerte je nach Ansichtstyp
CString View::GetUnitLabel(const char* viewName, const char* unitName, CObList* sectionList, double& maxVal, double& minVal)
{
    CString label;

    // === Querkraftanzeige: FZ ===
    if (strcmp(viewName, "FZ") == 0) {
        maxVal = -1e20;
        minVal = +1e20;

        POSITION pos = sectionList->GetHeadPosition();
        while (pos) {
            Section* s = (Section*)sectionList->GetNext(pos);
            for (double x = 0.0; x <= s->Length; x += 0.01) {
                double y = SolvePolynom(x, s);
                maxVal = (maxVal > y) ? maxVal : y;
                minVal = (minVal < y) ? minVal : y;
            }
        }

        double absMax = (fabs(maxVal) > fabs(minVal)) ? fabs(maxVal) : fabs(minVal);
        label.Format("[kN]  Qmax = %.2f", absMax);
    }

    // === Biegemomentanzeige: MY ===
    else if (strcmp(viewName, "MY") == 0) {
        maxVal = -1e20;
        minVal = +1e20;

        POSITION pos = sectionList->GetHeadPosition();
        while (pos) {
            Section* s = (Section*)sectionList->GetNext(pos);
            for (double x = 0.0; x <= s->Length; x += 0.01) {
                double y = SolvePolynom(x, s);
                maxVal = (maxVal > y) ? maxVal : y;
                minVal = (minVal < y) ? minVal : y;
            }
        }

        double absMax = (fabs(maxVal) > fabs(minVal)) ? fabs(maxVal) : fabs(minVal);
        label.Format("[kNm]  Mmax = %.2f", absMax);
    }

    // === Durchbiegung: UZ ===
    else if (strcmp(viewName, "UZ") == 0) {
        maxVal = -1e20;
        minVal = +1e20;

        POSITION pos = sectionList->GetHeadPosition();
        while (pos) {
            Section* s = (Section*)sectionList->GetNext(pos);
            for (double x = 0.0; x <= s->Length; x += 0.01) {
                double y = s->A4 * pow(x, 4) + s->A3 * pow(x, 3) + s->A2 * x * x + s->A1 * x + s->A0;
                maxVal = (maxVal > y) ? maxVal : y;
                minVal = (minVal < y) ? minVal : y;
            }
        }

        double absMax = (fabs(maxVal) > fabs(minVal)) ? fabs(maxVal) : fabs(minVal);
        label.Format("[mm]  wmax = %.2f", absMax * 1000.0);
    }

    // === Fallback für andere Ansichten ===
    else {
        maxVal = 0.0;
        minVal = 0.0;
        label = unitName;
    }

    return label;
}

// Zeichnet die Einheitenbeschriftung am rechten Rand der Ansicht
void View::DrawUnitLabel(CDC* pDC, int beamX, int beamY, double scaleX, CString label)
{
    CSize textSize = pDC->GetTextExtent(label);
    pDC->TextOut(beamX + (int)(_document->_beamLength * scaleX) + 12, beamY - (textSize.cy / 2), label);
}

// Berechnet das größte Ergebnis der Polynome zur Skalierung
double View::CalculateMaxValue(CObList* sectionList)
{
    // Startwert mit kleinstmöglichem positiven Betrag (0 erlaubt, da fabs(y) ≥ 0)
    double maxVal = 0.0;

    // Alle Abschnitte durchgehen
    POSITION pos = sectionList->GetHeadPosition();
    while (pos) {
        Section* s = (Section*)sectionList->GetNext(pos);

        // Im Abstand von 0.01 entlang des Abschnitts probieren
        for (double x = 0.0; x <= s->Length; x += 0.01) {
            double y = fabs(SolvePolynom(x, s));  // Betrag nehmen (für Darstellung)
            maxVal = (maxVal > y) ? maxVal : y;   // maxVal aktualisieren (ohne std::max)
        }
    }

    return maxVal;
}

// Zeichnet die Verformungskurven sowie farbige Flächen für positive und negative Bereiche
void View::DrawCurves(Graphics& graphics, CObList* sectionList, int beamX, int beamY, double scaleX, double scaleY, const char* viewName)
{
    SolidBrush brushRed(Color(80, 0, 0, 255));     // Transparente Farbe für positive Fläche
    SolidBrush brushBlue(Color(80, 255, 0, 0));    // Transparente Farbe für negative Fläche
    Pen curvePen(Color(255, 0, 0, 0), 1.5f);       // Linie für Kurve

    POSITION pos = sectionList->GetHeadPosition();
    while (pos)
    {
        Section* obj = (Section*)sectionList->GetNext(pos);
        double dx = 1.0 / scaleX;
        std::vector<PointF> posPts, negPts;
        PointF lastPt; bool first = true;
        double xAbsStart = obj->Start;

        for (double x = 0; x <= obj->Length; x += dx)
        {
            double y = SolvePolynom(x, obj) * scaleY;

            PointF pt((REAL)(beamX + (xAbsStart + x) * scaleX), (REAL)(beamY + y));
            if (!first) graphics.DrawLine(&curvePen, lastPt, pt);
            lastPt = pt; first = false;
            (y >= 0 ? posPts : negPts).push_back(pt);
        }

        if (posPts.size() > 1)
        {
            posPts.insert(posPts.begin(), PointF(posPts.front().X, (REAL)beamY));
            posPts.push_back(PointF(posPts.back().X, (REAL)beamY));
            graphics.FillPolygon(&brushRed, posPts.data(), (INT)posPts.size());
        }

        if (negPts.size() > 1)
        {
            negPts.insert(negPts.begin(), PointF(negPts.front().X, (REAL)beamY));
            negPts.push_back(PointF(negPts.back().X, (REAL)beamY));
            graphics.FillPolygon(&brushBlue, negPts.data(), (INT)negPts.size());
        }

        double yStart = SolvePolynom(0, obj) * scaleY;
        double yEnd = SolvePolynom(obj->Length, obj) * scaleY;

        graphics.DrawLine(&curvePen,
            PointF((REAL)(beamX + xAbsStart * scaleX), (REAL)beamY),
            PointF((REAL)(beamX + xAbsStart * scaleX), (REAL)(beamY + yStart)));

        graphics.DrawLine(&curvePen,
            PointF((REAL)(beamX + (xAbsStart + obj->Length) * scaleX), (REAL)(beamY + yEnd)),
            PointF((REAL)(beamX + (xAbsStart + obj->Length) * scaleX), (REAL)beamY));
    }
}

// Zeichnet Zahlenwerte (z. B. w, M, Q) in die Grafik, farblich hervorgehoben bei Extremwerten
void View::DrawNumericalValues(CDC* pDC, CObList* sectionList, int beamX, int beamY,
    double scaleX, double scaleY, BOOL mirror,
    double unitScale, double maxForColor, const char* viewName)
{
    UINT oldAlign = pDC->SetTextAlign(TA_CENTER | TA_TOP);
    std::set<std::pair<int, std::string>> drawn;

    // Bestimme kleinstes nicht-null Ergebnis zur Min-Wert-Erkennung
    double minShownAbs = 0.0;
    bool hasMin = false;

    POSITION pos = sectionList->GetHeadPosition();
    while (pos)
    {
        Section* obj = (Section*)sectionList->GetNext(pos);
        double yVals[] = {
            SolvePolynom(0, obj),
            SolvePolynom(GetMaximum(obj), obj),
            SolvePolynom(obj->Length, obj)
        };
        for (double y : yVals)
        {
            if (fabs(y) > EPSILON && (!hasMin || fabs(y) < fabs(minShownAbs))) {
                minShownAbs = y;
                hasMin = true;
            }
        }
    }

    // Zeichnet alle relevanten Werte: Start, Extremum, Ende
    pos = sectionList->GetHeadPosition();
    while (pos)
    {
        Section* obj = (Section*)sectionList->GetNext(pos);
        double xs[] = { 0, GetMaximum(obj), obj->Length };

        for (double x : xs)
        {
            double y = SolvePolynom(x, obj);
            if (fabs(y) <= EPSILON) continue;

            // Nur Extremum bei MY/UZ bei Mirror unterdrücken, nicht bei FZ
            bool isExtremum = (x == GetMaximum(obj));
            bool isFZ = (viewName && strcmp(viewName, "FZ") == 0);
            if (isExtremum && mirror && !isFZ) continue;

            int xPos = beamX + (int)((obj->Start + x) * scaleX);
            int yPos = beamY + (int)(y * scaleY);

            char buf[32];
            sprintf_s(buf, "%.2f", y * unitScale);
            std::pair<int, std::string> key(xPos, buf);

            if (drawn.insert(key).second)
            {
                bool isMax = fabs(fabs(y) - fabs(maxForColor)) < 1e-6;
                bool isMin = fabs(fabs(y) - fabs(minShownAbs)) < 1e-6;
                COLORREF color = isMax ? RGB(220, 0, 0) :
                    (isMin ? RGB(0, 200, 0) : RGB(0, 0, 0));
                DrawValue(pDC, xPos, yPos, mirror, y * unitScale, color);
            }
        }
    }

    pDC->SetTextAlign(oldAlign);
}

void View::DrawResults(CDC* pDC, int beamX, int beamY, double scaleX, int viewHeight, int views)
{
    // draw shear force graph
    if (_viewShearForce)
    {
        DrawView(pDC, beamX, beamY, scaleX, viewHeight, TRUE, _viewNumericalValues, 1, "[kN]", "FZ", &_document->_shearForceList);
        beamY += viewHeight;
    }

    // draw bending moment graph
    if (_viewBendingMoment)
    {
        DrawView(pDC, beamX, beamY, scaleX, viewHeight, FALSE, _viewNumericalValues, 1, "[kNm]", "MY", &_document->_bendingMomentList);
        beamY += viewHeight;
    }

    // draw displacement graph
    if (_viewDisplacement)
    {
        DrawView(pDC, beamX, beamY, scaleX, viewHeight, FALSE, _viewNumericalValues, 1000, "[mm]", "UZ", &_document->_displacementList);
        beamY += viewHeight;
    }
}
