#include "stdafx.h"
#include "Object.h"
#include <math.h>
#include <gdiplus.h>
#include <algorithm>

using namespace Gdiplus;
#define PASTELL_MIN 160
#define PASTELL_SPAN 80
#define FILL_ALPHA 100

IMPLEMENT_SERIAL(ObjectCast, CObject, 1);

void ObjectCast::Serialize(CArchive& archive)
{
    CObject::Serialize(archive);
    if (archive.IsStoring())
        archive << _type;
    else
        archive >> _type;
}

void ObjectCast::Draw(CDC* pDC, int x, int y, double Scale)
{
}

CRectTracker & ObjectCast::GetBoundRect()
{
    return _boundRect;
}

void ObjectCast::SetType(int type)
{
    _type = type;
}

int ObjectCast::GetType()
{
    return _type;
}

IMPLEMENT_SERIAL(SupportCast, ObjectCast, 2);

void SupportCast::Serialize(CArchive& archive)
{
    ObjectCast::Serialize(archive);
    if (archive.IsStoring())
        archive << _position;
    else
        archive >> _position;
}

void SupportCast::SetPosition(double value)
{
    _position = value;
}

double SupportCast::GetPosition()
{
    return _position;
}

IMPLEMENT_SERIAL(FixedSupport, SupportCast, 3);

void FixedSupport::Serialize(CArchive& archive)
{
    if (archive.IsLoading())
        archive >> _position;
    else
        archive << _position;
}

void FixedSupport::Draw(CDC* pDC, int x, int y, double Scale)
{
    x = x + ((int)(_position * Scale));

    pDC->MoveTo(x, y - 8);
    pDC->LineTo(x, y + 8);

    if (fabs(_position) < EPSILON)
    {
        pDC->MoveTo(x - 4, y - 4);
        pDC->LineTo(x, y - 8);
        pDC->MoveTo(x - 4, y);
        pDC->LineTo(x, y - 4);
        pDC->MoveTo(x - 4, y + 4);
        pDC->LineTo(x, y);
        pDC->MoveTo(x - 4, y + 8);
        pDC->LineTo(x, y + 4);

        _boundRect = CRectTracker(CRect(x - 8, y - 13, x + 4, y + 13), CRectTracker::dottedLine);
    }
    else
    {
        pDC->MoveTo(x + 4, y - 4);
        pDC->LineTo(x, y - 8);
        pDC->MoveTo(x + 4, y);
        pDC->LineTo(x, y - 4);
        pDC->MoveTo(x + 4, y + 4);
        pDC->LineTo(x, y);
        pDC->MoveTo(x + 4, y + 8);
        pDC->LineTo(x, y + 4);

        _boundRect = CRectTracker(CRect(x - 4, y - 13, x + 8, y + 13), CRectTracker::dottedLine);
    }
}

IMPLEMENT_SERIAL(HingedSupport, SupportCast, 3);

void HingedSupport::Serialize(CArchive& archive)
{
    if (archive.IsLoading())
        archive >> _position;
    else
        archive << _position;
}

void HingedSupport::Draw(CDC* pDC, int x, int y, double Scale)
{
    x = x + ((int)(_position * Scale));

    pDC->MoveTo(x, y);
    pDC->LineTo(x + 8, y + 8);
    pDC->LineTo(x - 8, y + 8);
    pDC->LineTo(x, y);

    _boundRect = CRectTracker(CRect(x - 12, y - 4, x + 12, y + 12), CRectTracker::dottedLine);
}

IMPLEMENT_SERIAL(RollerSupport, SupportCast, 3);

void RollerSupport::Serialize(CArchive& archive)
{
    if (archive.IsLoading())
        archive >> _position;
    else
        archive << _position;
}

void RollerSupport::Draw(CDC* pDC, int x, int y, double Scale)
{
    x = x + ((int)(_position * Scale));

    pDC->MoveTo(x, y);
    pDC->LineTo(x + 8, y + 8);
    pDC->LineTo(x - 8, y + 8);
    pDC->LineTo(x, y);
    pDC->MoveTo(x + 8, y + 10);
    pDC->LineTo(x - 8, y + 10);

    _boundRect = CRectTracker(CRect(x - 12, y - 4, x + 12, y + 14),
        CRectTracker::dottedLine);
}

IMPLEMENT_SERIAL(LoadCast, ObjectCast, 2);

void LoadCast::Serialize(CArchive& archive)
{
    ObjectCast::Serialize(archive);
    if (archive.IsStoring()) {
        archive << _position << _value << _length << _level;
    }
    else {
        archive >> _position >> _value >> _length >> _level;
    }
}

void LoadCast::GetExtent(CDC* pDC, double & Start,
    double & Length, double Scale)
{
}

void LoadCast::SetPosition(double position)
{
    _position = position;
}

double LoadCast::GetPosition()
{
    return _position;
}

void LoadCast::SetValue(double value)
{
    _value = value;
}

double LoadCast::GetValue()
{
    return _value;
}

void LoadCast::SetLength(double length)
{
    _length = length;
}

double LoadCast::GetLength()
{
    return _length;
}

void LoadCast::SetLevel(int level)
{
    _level = level;
}

int LoadCast::GetLevel()
{
    return _level;
}

IMPLEMENT_SERIAL(PointLoad, LoadCast, 3);

PointLoad::PointLoad()
{
    _level = 0;
}

void PointLoad::Serialize(CArchive& archive)
{
    if (archive.IsLoading())
        archive >> _position >> _value;
    else
        archive << _position << _value;
}

void PointLoad::Draw(CDC* pDC, int x, int y, double Scale)
{
    char buffer[36];

    x = x + ((int)(_position * Scale));
    y = y - ((_level - 1) * 30);

    pDC->MoveTo(x, y - 30);
    pDC->LineTo(x, y - 1);
    pDC->MoveTo(x - 4, y - 9);
    pDC->LineTo(x, y - 3);
    pDC->MoveTo(x + 4, y - 9);
    pDC->LineTo(x, y - 3);

    sprintf_s(buffer, "%.2f", _value);
    pDC->SetTextAlign(TA_LEFT | TA_TOP);
    pDC->TextOut(x + 3, y - 30, buffer);

    CSize textSize(pDC->GetTextExtent(buffer, (int)strlen(buffer)));

    _boundRect = CRectTracker(CRect(x - 9, y - 34, x + textSize.cx + 7, y + 4), CRectTracker::dottedLine);
}

void PointLoad::GetExtent(CDC* pDC, double& Start, double& Length, double Scale)
{
    Start = (int)(_position * Scale) - 9;
    char buffer[36];
    sprintf_s(buffer, "%.2f", _value);
    CSize textSize(pDC->GetTextExtent(buffer, (int)strlen(buffer)));
    Length = textSize.cx + 7;
}

IMPLEMENT_SERIAL(LinearDistributedLoad, LoadCast, 3);

LinearDistributedLoad::LinearDistributedLoad()
{
    _position = 0.0;
    _valueStart = _valueEnd = 0.0;
    _length = 0.0;
    _level = 0;

    _fillR = 160 + rand() % 80;
    _fillG = 160 + rand() % 80;
    _fillB = 160 + rand() % 80;
}

LinearDistributedLoad::LinearDistributedLoad(double position, double valueStart, double valueEnd, double length)
{
    _position = position;
    _valueStart = valueStart;
    _valueEnd = valueEnd;
    _length = length;
    _level = 0;

    _fillR = 160 + rand() % 80;
    _fillG = 160 + rand() % 80;
    _fillB = 160 + rand() % 80;
}

Color LinearDistributedLoad::GetFillColor() const
{
    return Color(100, _fillR, _fillG, _fillB);
}

void LinearDistributedLoad::Serialize(CArchive& ar)
{
    ObjectCast::Serialize(ar);

    if (ar.IsStoring())
    {
        ar << _position << _valueStart << _valueEnd << _length;
        ar << _fillR << _fillG << _fillB;
    }
    else
    {
        ar >> _position >> _valueStart >> _valueEnd >> _length;
        ar >> _fillR >> _fillG >> _fillB;
    }
}

void LinearDistributedLoad::Draw(CDC* pDC, int x, int y, double Scale)
{
    Graphics graphics(pDC->GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetCompositingQuality(CompositingQualityHighQuality);

    x = x + (int)(_position * Scale);
    y = y - ((_level - 1) * 30);

    int length = (int)(_length * Scale);
    int x0 = x;
    int x1 = x + length;
    int y0 = y;

    const int minHeight = 6;
    const int maxHeight = 32;
    const double referenceLoad = 10.0;

    auto getHeight = [&](double value) -> int {
        int h = minHeight + (int)(fabs(value) * (maxHeight - minHeight) / referenceLoad);
        if (h < minHeight) return minHeight;
        if (h > maxHeight) return maxHeight;
        return h;
        };

    int dyStart = getHeight(_valueStart);
    int dyEnd = getHeight(_valueEnd);

    int topYStart = y0 - dyStart;
    int topYEnd = y0 - dyEnd;

    // Trapez zeichnen (unten y0, oben y0 + dy)
    PointF trapezoid[4];
    if (fabs(_valueStart - _valueEnd) < 1e-6) {
        // Rechteck zeichnen für konstante Linienlast
        trapezoid[0] = PointF((REAL)x0, (REAL)y0);
        trapezoid[1] = PointF((REAL)x0, (REAL)topYStart);
        trapezoid[2] = PointF((REAL)x1, (REAL)topYStart);
        trapezoid[3] = PointF((REAL)x1, (REAL)y0);
    }
    else {
        // Trapez zeichnen für veränderliche Linienlast
        trapezoid[0] = PointF((REAL)x0, (REAL)y0);
        trapezoid[1] = PointF((REAL)x0, (REAL)topYStart);
        trapezoid[2] = PointF((REAL)x1, (REAL)topYEnd);
        trapezoid[3] = PointF((REAL)x1, (REAL)y0);
    }

    SolidBrush fill(Color(100, _fillR, _fillG, _fillB));
    Pen border(Color(255, 0, 0, 0), 1.0f);
    graphics.FillPolygon(&fill, trapezoid, 4);
    graphics.DrawPolygon(&border, trapezoid, 4);

    // Vertikale Linien innerhalb des Trapezes
    int pixelLength = x1 - x0;
    double DX = (_length / (((double)pixelLength / 9) + 1));
    double XC = DX;

    while ((XC * Scale) < (pixelLength - 1))
    {
        double rel = XC / _length;
        int xi = x0 + (int)(XC * Scale);
        int yi = (int)((1.0 - rel) * topYStart + rel * topYEnd);

        pDC->MoveTo(xi, y0);
        pDC->LineTo(xi, yi);

        XC += DX;
    }

    // Pfeile immer nach unten
    auto drawArrow = [&](int xArrow, int dyVal) {
        int tip = y0;                   // Basislinie (Ziel des Pfeils)
        int top = y0 - dyVal;           // Trapezoberkante (Start des Pfeils)

        pDC->MoveTo(xArrow, top);       // Schaft des Pfeils
        pDC->LineTo(xArrow, tip);

        // Pfeilspitze zeigt nach unten
        pDC->MoveTo(xArrow - 3, tip - 5);
        pDC->LineTo(xArrow, tip);
        pDC->MoveTo(xArrow + 3, tip - 5);
        pDC->LineTo(xArrow, tip);
        };

    drawArrow(x0, dyStart);
    drawArrow(x1, dyEnd);

    // Beschriftung rechts neben dem Trapez
    char buffer[64];
    sprintf_s(buffer, "%.2f -> %.2f", _valueStart, _valueEnd);
    pDC->SetTextAlign(TA_LEFT | TA_TOP);

    CSize textSize = pDC->GetTextExtent(buffer);
    int topYMin = min(topYStart, topYEnd);
    int textY = topYMin - textSize.cy - 4; // Text oberhalb der Trapezoberkante

    pDC->TextOut(x1 + 4, textY, buffer);   // Text platzieren

    _boundRect = CRectTracker(
        CRect(x0 - 9, textY - 4, x1 + textSize.cx + 7, y0 + 4),
        CRectTracker::dottedLine
    );
}

void LinearDistributedLoad::GetExtent(CDC* pDC, double& Start,
    double & Length, double Scale)
{
    Start = (int)(_position * Scale) - 9;
    char buffer[36];
    sprintf_s(buffer, "%.2f", _value);
    CSize textSize(pDC->GetTextExtent(buffer, (int)strlen(buffer)));
    Length = (int)(_length * Scale) + textSize.cx + 7;
}

