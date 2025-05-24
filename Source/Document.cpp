#include <stdio.h> 
#include <math.h>
#include "stdafx.h"
#include "Application.h"
#include "Dialog.h"
#include "Object.h"
#include "Document.h"
#include <cmath>


IMPLEMENT_DYNCREATE(Document, CDocument)

BEGIN_MESSAGE_MAP(Document, CDocument)
    ON_COMMAND(IDM_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

Document::Document()
{
    EnableAutomation();
    AfxOleLockApp();

    // default values
    _beamLength = ((Application*)AfxGetApp())->_beamLength;
    _modulusOfElasticity = 210000 * 1000;
    _momentOfInertia = 0.0000836;
    _axialArea = 0.00538;

    // no object is selected
    _selected = NULL;
}

Document::~Document()
{
    CoFreeUnusedLibraries();
    AfxOleUnlockApp();
}

BOOL Document::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
    {
        return FALSE;
    }

    ((Application*)AfxGetApp())->_beamLength = 10;

    if (((Application*)AfxGetApp())->_beamCreate)
    {
        ((Application*)AfxGetApp())->_beamCreate = TRUE;
        return TRUE;
    }
    else
    {
        ((Application*)AfxGetApp())->_beamCreate = TRUE;
        return FALSE;
    }
}

void Document::Serialize(CArchive& archive)
{
    if (archive.IsLoading())
    {
        archive >> _beamLength;
        archive >> _modulusOfElasticity >> _momentOfInertia;
        _objectList.Serialize(archive);
    }
    else
    {
        archive << _beamLength;
        archive << _modulusOfElasticity << _momentOfInertia;
        _objectList.Serialize(archive);
    }
    Analyse();
}

BOOL Document::InsertObject(ObjectCast* item)
{
    if (item == NULL)
        return FALSE;

    if (CLASSOF(item, "HingedSupport") || CLASSOF(item, "FixedSupport") || CLASSOF(item, "RollerSupport"))
    {
        ObjectCast* object;
        POSITION position = _objectList.GetHeadPosition();
        while (position != NULL)
        {
            object = (ObjectCast*)_objectList.GetNext(position);
            if (object != NULL)
            {
                if (CLASSOF(object, "HingedSupport") || CLASSOF(object, "FixedSupport") || CLASSOF(object, "RollerSupport")) {
                    if (fabs(((SupportCast*)object)->GetPosition() - ((SupportCast*)item)->GetPosition()) < EPSILON)
                    {
                        return FALSE;
                    }
                }
            }
        }
    }

    if (_objectList.IsEmpty())
    {
        _objectList.AddHead(item);
    }
    else
    {
        _objectList.AddTail(item);
    }
    Analyse();
    UpdateAllViews(NULL);
    return TRUE;
}

void Document::DeleteObject(ObjectCast* item)
{
    POSITION position = _objectList.GetHeadPosition();
    position = _objectList.Find(item);
    if (position != NULL)
    {
        ObjectCast* itemToDelete = (ObjectCast*)_objectList.GetAt(position);
        _objectList.RemoveAt(position);
        delete itemToDelete;
    }
}

int Document::SortLoadLevels(CDC* pDC, double scaleX)
{
    int maximumLevel = 1;
    LoadCast* currentObject = NULL;

    // set the level of all object to zero
    ObjectCast* object = NULL;
    POSITION position = _objectList.GetHeadPosition();
    while (position != NULL)
    {
        object = (ObjectCast*)_objectList.GetNext(position);
        if ((object != NULL) && (CLASSOF(object, "PointLoad") || CLASSOF(object, "LinearDistributedLoad")))
        {
            ((LoadCast*)object)->SetLevel(0);
        }
    }

    // sort all load objects
    do
    {
        currentObject = NULL;
        double currentStart = 0;
        double currentLength = 0;

        // scan for the biggest load object at level 0
        ObjectCast* object;
        POSITION position = _objectList.GetHeadPosition();
        while (position != NULL)
        {
            object = (ObjectCast*)_objectList.GetNext(position);
            if ((object != NULL) && (CLASSOF(object, "PointLoad") || CLASSOF(object, "LinearDistributedLoad")))
            {
                double start;
                double length;
                ((LoadCast*)object)->GetExtent(pDC, start, length, scaleX);
                if ((((LoadCast*)object)->GetLevel() == 0) && (length > currentLength))
                {
                    currentObject = (LoadCast*)object;
                    currentStart = start;
                    currentLength = length;
                }
            }
        }

        // exit if no object found
        if (currentObject == NULL)
        {
            return maximumLevel;
        }

        // increase the level of the object until it fits 
        int currentLevel = 0;
        BOOL match = FALSE;
        do
        {
            match = FALSE;
            currentLevel++;
            currentObject->SetLevel(currentLevel);
            ObjectCast* object;
            POSITION position = _objectList.GetHeadPosition();
            while (position != NULL)
            {
                object = (ObjectCast*)_objectList.GetNext(position);
                if ((object != NULL) && (object != currentObject) && (CLASSOF(object, "PointLoad") || CLASSOF(object, "LinearDistributedLoad")))
                {
                    int level;
                    double start;
                    double length;
                    level = ((LoadCast*)object)->GetLevel();
                    ((LoadCast*)object)->GetExtent(pDC, start, length, scaleX);
                    if (level == currentLevel)
                    {
                        if ((currentStart < start + length) && (currentStart + currentLength > start)) {
                            match = TRUE;
                        }
                    }
                }
            }
        } while (match);

        // update maximum level
        if (maximumLevel < currentLevel) maximumLevel = currentLevel;
    } while (currentObject != NULL);

    return maximumLevel;
}

BOOL Document::Analyse()
{
    Beam* analysis = new Beam();

    // create beam
    analysis->SetLength(_beamLength);
    analysis->SetEI(_modulusOfElasticity, _momentOfInertia);

    // transfer objects
    ObjectCast* object;
    POSITION position = _objectList.GetHeadPosition();
    while (position != NULL)
    {
        object = (ObjectCast*)_objectList.GetNext(position);
        if (object != NULL)
        {
            if (CLASSOF(object, "FixedSupport"))
                analysis->CreateFixedSupport(((SupportCast *)object)->GetPosition());
            if (CLASSOF(object, "HingedSupport"))
                analysis->CreateHingedSupport(((SupportCast *)object)->GetPosition());
            if (CLASSOF(object, "RollerSupport"))
                analysis->CreateRollerSupport(((SupportCast *)object)->GetPosition());
            if (CLASSOF(object, "PointLoad"))
            {
                double pos = ((LoadCast*)object)->GetPosition();
                double val = ((LoadCast*)object)->GetValue();
                analysis->CreatePointLoad(pos, val);
            }
            if (CLASSOF(object, "LinearDistributedLoad"))
            {
                LinearDistributedLoad* ldl = (LinearDistributedLoad*)object;
                analysis->CreateLinearDistributedLoad(ldl->GetPosition(), ldl->GetValueStart(), ldl->GetValueEnd(), ldl->GetLength());
            }
        }
    }

    // start analysis
    analysis->Analyse();

    // delete old results
    _shearForceList.RemoveAll();
    _bendingMomentList.RemoveAll();
    _displacementList.RemoveAll();

    // retrieve results
    Section* shearForce;
    Section* bendingMoment;
    Section* displacement;
    HRESULT result = 0;
    do {
        shearForce = new Section;
        bendingMoment = new Section;
        displacement = new Section;
        result = analysis->GetNextSection(shearForce, bendingMoment, displacement);

        // NICHT mehr shearForce verändern!
        _shearForceList.AddHead((CObject*)shearForce);
        _bendingMomentList.AddHead((CObject*)bendingMoment);
        _displacementList.AddHead((CObject*)displacement);
    } while (result == S_OK);


    // Biegemomente: Max/Min aus Polynomen
    _maxMoment = 0.0;
    _minMoment = DBL_MAX;  // <--- Wichtig!

    POSITION p = _bendingMomentList.GetHeadPosition();
    while (p != NULL) {
        Section* s = (Section*)_bendingMomentList.GetNext(p);
        if (s) {
            for (double x = 0; x <= s->Length; x += 0.01) {
                double val = s->A4 * pow(x, 4) + s->A3 * pow(x, 3) + s->A2 * x * x + s->A1 * x + s->A0;

                if (fabs(val) > fabs(_maxMoment)) _maxMoment = val;

                if (fabs(val) > EPSILON && fabs(val) < fabs(_minMoment)) _minMoment = val;
            }
        }
    }

    // Querkraft: Max/Min aus linearer Funktion
    p = _shearForceList.GetHeadPosition();
    _maxShear = 0.0;
    _minShear = DBL_MAX;

    while (p != NULL) {
        Section* s = (Section*)_shearForceList.GetNext(p);
        if (s) {
            // Extremwerte der linearen Funktion Q(x) = A1 * x + A0 liegen am Rand
            double valStart = s->A0;
            double valEnd = s->A1 * s->Length + s->A0;

            double maxAbs = fmax(fabs(valStart), fabs(valEnd));
            double minAbs = fmin(fabs(valStart), fabs(valEnd));

            if (maxAbs > _maxShear) _maxShear = maxAbs;
            if (minAbs < _minShear && minAbs > EPSILON) _minShear = minAbs;
        }
    }

    _supportReactions.clear();

    analysis->_sections.Reset();
    while (!analysis->_sections.IsEmpty())
    {
        SupportNode* node = (SupportNode*)analysis->_sections.GetItem();
        double pos = node->GetPosition();
        double R = node->GetForce()(3, 0);

        // Suche Punktlasten im Analysemodell
        analysis->_loads.Reset();
        while (!analysis->_loads.IsEmpty())
        {
            LoadNode* ln = (LoadNode*)analysis->_loads.GetItem();
            if (ln->GetLength() == 0.0 && fabs(ln->GetStart() - pos) < EPSILON && ln->IsSupportLoad())
            {
                R += ln->GetValue();
            }
            if (!analysis->_loads.Next()) break;
        }

        _supportReactions[pos] = R;
        if (!analysis->_sections.Next()) break;
    }

    // Max Reaktion berechnen
    _maxReaction = 0.0;

    for (auto& r : _supportReactions) {
        if (fabs(r.second) > _maxReaction)
            _maxReaction = fabs(r.second);
    }

    delete analysis;

    return TRUE;
}

void Document::OnProperties()
{
    PropertiesDialog dialog;

    dialog.SetModulusOfElasticity(_modulusOfElasticity / 1000);
    dialog.SetMomentOfInertia(_momentOfInertia * (100 * 100 * 100 * 100));
    dialog.SetAxialArea(_axialArea * (100 * 100));

    if (dialog.DoModal() == IDOK)
    {
        _modulusOfElasticity = dialog.GetModulusOfElasticity() * 1000;
        _momentOfInertia = dialog.GetMomentOfInertia() / (100 * 100 * 100 * 100);
        _axialArea = dialog.GetAxialArea() / (100 * 100);
    }

    UpdateAllViews(NULL);
}

BOOL Document::CreateBeam(double length, double modulusOfElasticity, double momentOfInertia, double axialArea)
{
    _beamLength = length;
    _modulusOfElasticity = modulusOfElasticity;
    _momentOfInertia = momentOfInertia;
    _axialArea = axialArea;
    return TRUE;
}

BOOL Document::CreateFixedSupport(double position)
{
    SupportCast * SupportObject = NULL;
    if ((position != 0) && (position != _beamLength)) return FALSE;
    SupportObject = new FixedSupport;
    if (SupportObject == NULL) return FALSE;
    SupportObject->SetPosition(position);
    if (!InsertObject(SupportObject))
    {
        delete SupportObject;
        return FALSE;
    }
    _selected = SupportObject;
    return TRUE;
}

BOOL Document::CreateHingedSupport(double position)
{
    SupportCast * SupportObject = NULL;
    if ((position < 0) || (position > _beamLength)) return FALSE;
    SupportObject = new HingedSupport;
    if (SupportObject == NULL) return FALSE;
    SupportObject->SetPosition(position);
    if (!InsertObject(SupportObject))
    {
        delete SupportObject;
        return FALSE;
    }
    _selected = SupportObject;
    return TRUE;
}

BOOL Document::CreateRollerSupport(double position)
{
    SupportCast * SupportObject = NULL;
    if ((position < 0) || (position > _beamLength)) return FALSE;
    SupportObject = new RollerSupport;
    if (SupportObject == NULL) return FALSE;
    SupportObject->SetPosition(position);
    if (!InsertObject(SupportObject))
    {
        delete SupportObject;
        return FALSE;
    }
    _selected = SupportObject;
    return TRUE;
}

BOOL Document::CreatePointLoad(double position, double value)
{
    LoadCast* loadObject = NULL;
    if ((position < 0) || (position > _beamLength)) return FALSE;
    loadObject = new PointLoad;
    if (loadObject == NULL) return FALSE;
    loadObject->SetValue(value);
    loadObject->SetPosition(position);
    if (!InsertObject(loadObject))
    {
        delete loadObject;
        return FALSE;
    }
    _selected = loadObject;
    return TRUE;
}

BOOL Document::CreateLinearDistributedLoad(double position, double valueStart, double valueEnd, double length)
{
    LinearDistributedLoad* loadObject = new LinearDistributedLoad(position, valueStart, valueEnd, length);
    if (!loadObject)
        return FALSE;

    if ((length < 0) || (position < 0) || ((position + length) > _beamLength))
    {
        delete loadObject;
        return FALSE;
    }

    if (!InsertObject(loadObject))
    {
        delete loadObject;
        return FALSE;
    }

    _selected = loadObject;
    return TRUE;
}


double Document::GetShearForce(double position)
{
    Section* object;
    POSITION pointer = _shearForceList.GetHeadPosition();
    while (pointer != NULL)
    {
        object = (Section*)_shearForceList.GetNext(pointer);
        if (object != NULL)
        {
            if ((position >= object->Start) && (position <= (object->Start + object->Length)))
            {
                double x = position - object->Start;
                return (object->A4 * x * x * x * x + object->A3 * x * x * x + object->A2 * x * x + object->A1 * x + object->A0);
            }
        }
    }
    return 0;
}

double Document::GetBendingMoment(double position)
{
    Section* object;
    POSITION pointer = _bendingMomentList.GetHeadPosition();
    while (pointer != NULL)
    {
        object = (Section*)_bendingMomentList.GetNext(pointer);
        if (object != NULL)
        {
            if ((position >= object->Start) && (position <= (object->Start + object->Length)))
            {
                double x = position - object->Start;
                return (object->A4 * x * x * x * x + object->A3 * x * x * x + object->A2 * x * x + object->A1 * x + object->A0);
            }
        }
    }
    return 0;
}

double Document::GetDisplacement(double position)
{
    Section* object;
    POSITION pointer = _displacementList.GetHeadPosition();
    while (pointer != NULL)
    {
        object = (Section*)_displacementList.GetNext(pointer);
        if (object != NULL)
        {
            if ((position >= object->Start) &&
                (position <= (object->Start + object->Length)))
            {
                double x = position - object->Start;
                return (object->A4 * x * x * x * x + object->A3 * x * x * x + object->A2 * x * x + object->A1 * x + object->A0);
            }
        }
    }
    return 0;
}

double Document::GetSupportReactionAt(double position)
{
    for (std::map<double, double>::const_iterator it = _supportReactions.begin(); it != _supportReactions.end(); ++it)
    {
        if (fabs(it->first - position) < EPSILON)
        {
            return it->second;
        }
    }
    return 0.0;
}


