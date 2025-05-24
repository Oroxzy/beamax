#include "stdafx.h"
#include "Object.h"
#include <math.h>
#include <gdiplus.h>
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
    _value = 0.0;
    _length = 0.0;
    _level = 0;

    _fillR = PASTELL_MIN + rand() % PASTELL_SPAN;
    _fillG = PASTELL_MIN + rand() % PASTELL_SPAN;
    _fillB = PASTELL_MIN + rand() % PASTELL_SPAN;
}

LinearDistributedLoad::LinearDistributedLoad(double position, double value, double length)
{
    _position = position;
    _value = value;
    _length = length;
    _level = 0;

    // Pastellfarbe generieren
    _fillR = PASTELL_MIN + rand() % PASTELL_SPAN;
    _fillG = PASTELL_MIN + rand() % PASTELL_SPAN;
    _fillB = PASTELL_MIN + rand() % PASTELL_SPAN;
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
        ar << _position << _value << _length;
        ar << _fillR << _fillG << _fillB;
    }
    else
    {
        ar >> _position >> _value >> _length;
        ar >> _fillR >> _fillG >> _fillB;
    }
}

void LinearDistributedLoad::Draw(CDC* pDC, int x, int y, double Scale)
{
    Graphics graphics(pDC->GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetCompositingQuality(CompositingQualityHighQuality);

    char buffer[36];

    x = x + (int)(_position * Scale);
    y = y - ((_level - 1) * 30);

    int length = (int)(_length * Scale);
    int height = 30;

    // Bereits gespeicherte Farbe verwenden
    SolidBrush pastelBrush(Color(FILL_ALPHA, _fillR, _fillG, _fillB));
    graphics.FillRectangle(&pastelBrush, x, y - height, length, height);

    pDC->MoveTo(x, y - 30);
    pDC->LineTo(x + length, y - 30);

    pDC->MoveTo(x, y - 30);
    pDC->LineTo(x, y - 1);
    pDC->MoveTo(x - 4, y - 9);
    pDC->LineTo(x, y - 3);
    pDC->MoveTo(x + 4, y - 9);
    pDC->LineTo(x, y - 3);

    pDC->MoveTo(x + length, y - 30);
    pDC->LineTo(x + length, y - 1);
    pDC->MoveTo(x + length - 4, y - 9);
    pDC->LineTo(x + length, y - 3);
    pDC->MoveTo(x + length + 4, y - 9);
    pDC->LineTo(x + length, y - 3);

    double DX = (_length / (((double)length / 9) + 1));
    double XC = 0;
    XC += DX;

    while ((XC * Scale) < (length - 1))
    {
        pDC->MoveTo(x + ((int)(XC * Scale)), y - 27);
        pDC->LineTo(x + ((int)(XC * Scale)), y - 4);
        XC += DX;
    }

    sprintf_s(buffer, "%.2f", _value);
    pDC->SetTextAlign(TA_LEFT | TA_TOP);
    pDC->TextOut(x + length + 2, y - 30, buffer);

    CSize textSize(pDC->GetTextExtent(buffer, (int)strlen(buffer)));

    _boundRect = CRectTracker(CRect(x - 9, y - 34,
        x + length + textSize.cx + 7, y + 4),
        CRectTracker::dottedLine);
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

