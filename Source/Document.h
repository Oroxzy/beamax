#ifndef __Document__
#define __Document__
#include <map> // falls nicht vorhanden

class Document : public CDocument
{
private:
    double _modulusOfElasticity;
    double _momentOfInertia;
    double _axialArea;

    double _maxMoment;
    double _maxShear;
    double _maxReaction;

public:
    double _beamLength;
    CObList _objectList;
    CObList _shearForceList;
    CObList _bendingMomentList;
    CObList _displacementList;
    CObject* _selected;

public:
    virtual void Serialize(CArchive& Archive);
    virtual BOOL OnNewDocument();
    BOOL InsertObject(ObjectCast* item);
    void DeleteObject(ObjectCast* item);
    int SortLoadLevels(CDC* pDC, double scaleX);
    BOOL Analyse();

    // Zugriff auf Analyse-Ergebnisse (für View)
    double GetMaxMoment() const { return _maxMoment; }
    double GetMaxShear() const { return _maxShear; }
    double GetMaxReaction() const { return _maxReaction; }

    // automation interface
    BOOL CreateBeam(double length, double modulusOfElasticity, double momentOfInertia, double axialArea);
    BOOL CreateFixedSupport(double position);
    BOOL CreateHingedSupport(double position);
    BOOL CreateRollerSupport(double position);
    BOOL CreatePointLoad(double position, double value);
    BOOL CreateLinearDistributedLoad(double position, double value, double length);
    double GetShearForce(double position);
    double GetBendingMoment(double position);
    double GetDisplacement(double position);
    double GetSupportReactionAt(double position);

protected:
    Document();
    ~Document();
    afx_msg void OnProperties();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(Document)
    std::map<double, double> _supportReactions;

};

#endif
