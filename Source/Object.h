#ifndef __Object__
#define __Object__

#include "stdafx.h"
#include <gdiplus.h>
using namespace Gdiplus;

class ObjectCast : public CObject
{
protected:
    int _type = 0;
    CRectTracker _boundRect;

public:
    virtual void Serialize(CArchive& archive);
    virtual void Draw(CDC* pDC, int x, int y, double Scale);
    virtual CRectTracker& GetBoundRect();
    void SetType(int type);
    int GetType();

    DECLARE_SERIAL(ObjectCast)
};

class SupportCast : public ObjectCast
{
protected:
    double _position = 0.0;

public:
    virtual void Serialize(CArchive& archive);
    void SetPosition(double value);
    double GetPosition();

    DECLARE_SERIAL(SupportCast)
};

class FixedSupport : public SupportCast
{
public:
    virtual void Serialize(CArchive& archive);
    virtual void Draw(CDC* pDC, int x, int y, double Scale);

    DECLARE_SERIAL(FixedSupport)
};

class HingedSupport : public SupportCast
{
public:
    virtual void Serialize(CArchive& archive);
    virtual void Draw(CDC* pDC, int x, int y, double Scale);

    DECLARE_SERIAL(HingedSupport)
};

class RollerSupport : public SupportCast
{
public:
    virtual void Serialize(CArchive& archive);
    virtual void Draw(CDC* pDC, int x, int y, double Scale);

    DECLARE_SERIAL(RollerSupport)
};

class LoadCast : public ObjectCast
{
protected:
    int _level;
    double _position;
    double _value;
    double _length;

public:
    virtual void Serialize(CArchive& archive);
    virtual void GetExtent(CDC* pDC, double& Start, double& Length, double Scale);
    void SetPosition(double value);
    double GetPosition();
    void SetValue(double value);
    double GetValue();
    void SetLength(double value);
    double GetLength();
    void SetLevel(int value);
    int GetLevel();

    DECLARE_SERIAL(LoadCast)
};

class PointLoad : public LoadCast
{
public:
    PointLoad();
    virtual void Serialize(CArchive& archive);
    virtual void Draw(CDC* pDC, int x, int y, double Scale);
    virtual void GetExtent(CDC* pDC, double& Start, double& Length, double Scale);

    DECLARE_SERIAL(PointLoad)
};

class LinearDistributedLoad : public LoadCast
{
    DECLARE_SERIAL(LinearDistributedLoad)

private:
    BYTE _fillR, _fillG, _fillB;
    double _valueStart;
    double _valueEnd;

public:
    LinearDistributedLoad(); // für Serialize
    LinearDistributedLoad(double position, double valueStart, double valueEnd, double length);

    virtual void Serialize(CArchive& archive) override;
    virtual void Draw(CDC* pDC, int x, int y, double Scale) override;
    virtual void GetExtent(CDC* pDC, double& Start, double& Length, double Scale) override;

    Gdiplus::Color GetFillColor() const;

    // Diese Methoden HIER hinzufügen:
    void SetValues(double start, double end) { _valueStart = start; _valueEnd = end; }
    double GetValueStart() const { return _valueStart; }
    double GetValueEnd() const { return _valueEnd; }
    void SetValueStart(double value) { _valueStart = value; }
    void SetValueEnd(double value) { _valueEnd = value; }
};

#endif
