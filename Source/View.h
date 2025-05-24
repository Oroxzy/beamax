#include <gdiplus.h>
#include <gdipluseffects.h>
#ifndef __View__
#define __View__

class View : public CScrollView
{
private:
    Document* _document;
    CMenu* _popupMenu;
    BOOL _viewContinuousBeam;
    BOOL _viewShearForce;
    BOOL _viewBendingMoment;
    BOOL _viewDisplacement;
    BOOL _viewNumericalValues;

public:
    View();
    virtual ~View();

    // message handler
protected:
    virtual void OnInitialUpdate();
    virtual void OnDraw(CDC* pDC);
    virtual BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT flags, CPoint point);
    afx_msg void OnRButtonDown(UINT flags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
    afx_msg void OnCreateFixedSupport();
    afx_msg void OnEditFixedSupport();
    afx_msg void OnCreateHingedSupport();
    afx_msg void OnEditHingedSupport();
    afx_msg void OnCreateRollerSupport();
    afx_msg void OnEditRollerSupport();
    afx_msg void OnCreatePointLoad();
    afx_msg void OnEditPointLoad();
    afx_msg void OnCreateLinearDistributedLoad();
    afx_msg void OnEditLinearDistributedLoad();
    afx_msg void OnDelete();
    afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
    afx_msg void OnOneLevelUp();
    afx_msg void OnUpdateOneLevelUp(CCmdUI* pCmdUI);
    afx_msg void OnOneLevelDown();
    afx_msg void OnUpdateOneLevelDown(CCmdUI* pCmdUI);
    afx_msg void OnViewContinuousBeam();
    afx_msg void OnUpdateViewContinuousBeam(CCmdUI* pCmdUI);
    afx_msg void OnViewShearForce();
    afx_msg void OnUpdateViewShearForce(CCmdUI* pCmdUI);
    afx_msg void OnViewBendingMoment();
    afx_msg void OnUpdateViewBendingMoment(CCmdUI* pCmdUI);
    afx_msg void OnViewDisplacement();
    afx_msg void OnUpdateViewDisplacement(CCmdUI* pCmdUI);
    afx_msg void OnViewNumericalValues();
    afx_msg void OnUpdateViewNumericalValues(CCmdUI* pCmdUI);

    // printer handler
    virtual BOOL OnPreparePrinting(CPrintInfo* pPrintInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pPrintInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pPrintInfo);
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pPrintInfo);

    // Graphik- und Zeichenfunktionen für Balkenansichten

    // Hilfsfunktionen für Polynom-Analyse
    inline double GetMaximum(Section* object);                   // Liefert x-Koordinate des Maximums eines Polynoms
    inline double SolvePolynom(double x, Section* object);       // Berechnet den y-Wert eines Polynoms an Stelle x

    // Allgemeine Zeichenhilfen
    inline BOOL IsRectEmpty(CDC* pDC, int x1, int y1, int x2, int y2); // Prüft, ob ein Rechteck leer ist (nützlich für Optimierungen)
    void LPtoDP(CRect& rect);                                     // Wandelt logische in Geräteeinheiten um
    void Draw(CDC* pDC, CDC* pDrawDC);                            // Führt den eigentlichen Zeichenvorgang aus (Rahmen, Koordinaten etc.)

    // Einzelwert-Visualisierung
    void DrawValue(CDC* pDC, int x, int y, BOOL mirror, double value, COLORREF color); // Zeichnet einen Zahlenwert (z. B. Moment, Verschiebung)
    void DrawReactionValue(CDC* pDC, int x, int y, double value, COLORREF color);      // Zeichnet eine Auflagerkraft oder Lagerreaktion

    // Hauptfunktionen zur Ergebnisdarstellung
    void DrawView(CDC* pDC, int beamX, int beamY, double scaleX, int viewHeight, BOOL mirror, BOOL values, double unitScale, char* unitName, char* viewName, CObList* sectionList); // Zeichnet eine einzelne Ergebnisansicht
    void DrawResults(CDC* pDC, int beamX, int beamY, double scaleX, int viewHeight, int views); // Zeichnet alle aktivierten Ergebnisansichten

    // Neue Hilfsfunktionen aus Refactoring (DrawView intern verwendet)
    void DrawBeam(CDC* pDC, int beamX, int beamY, double scaleX);
    void DrawViewName(CDC* pDC, int beamX, int beamY, const char* viewName);
    CString GetUnitLabel(const char* viewName, const char* unitName, CObList* sectionList, double& maxVal, double& minVal);
    void DrawUnitLabel(CDC* pDC, int beamX, int beamY, double scaleX, CString label);
    double CalculateMaxValue(CObList* sectionList);
    void DrawCurves(Gdiplus::Graphics& graphics, CObList* sectionList, int beamX, int beamY, double scaleX, double scaleY, const char* viewName);
    void DrawNumericalValues(CDC* pDC, CObList* sectionList, int beamX, int beamY,
        double scaleX, double scaleY, BOOL mirror,
        double unitScale, double maxForColor, const char* viewName);

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(View)
};

#endif
