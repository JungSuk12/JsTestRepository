#pragma once

#include <vector>

class CMFCTestDlg : public CDialogEx
{
public:
  CMFCTestDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_MFCTEST_DIALOG };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  int m_nThickness = 1;

protected:
  HICON m_hIcon;
  DECLARE_MESSAGE_MAP()

 
private:
  std::vector<CPoint> m_Points;
  void DrawCircleFromPoints(CDC& dc);
  void OnDrawCircle(CDC& CDC, CRect& CRect);
  void DrawCircleFromDrag(CDC& dc);
  void CMFCTestDlg::DrawTriangleFromPoints(CDC& dc);
  int CalculateCircle(const std::vector<CPoint>& points, CPoint& center, CRect& CRectStatic);
  int nParameterMinRadius;
  int m_nPrevRadius = 0;
  //void CMFCTestDlg::RecalculateCircle();
  CBrush m_BackGroundColor;
  CBitmap m_BtnImageTest;
  CBitmap m_BtnImage;
  CBitmapButton m_btnImage;
  CMFCButton m_btnReset;
  CMFCButton m_btnRandom;
  CPoint CMFCTestDlg::AdjustPointToStaticArea(const CPoint& point, const CRect& rect);
  CPoint CMFCTestDlg::CircleToInStaticArea(CPoint pCenter, int& nRadius, const CRect& CRect);
  void CMFCTestDlg::AdjustCircleRadiusToStaticArea(CPoint& pCenter, int& nRadius, const CRect& CRect);
  void CMFCTestDlg::LimitRadiusCircle(CPoint& pCenter, int& nRadius, const CRect& CRect);
  BOOL CMFCTestDlg::OnEraseBkgnd(CDC* pDC);
  CPen* m_CirclePen;
  CRgn m_CircleRgn;
  CPen m_CirclrPen;
  CPen* m_pOldPen;
  CPoint m_StartPoint;
  CPoint m_EndPoint;
  bool m_bIsDragging = false;
  afx_msg
    BOOL OnInitDialog();
  HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  //HBRUSH OnDialogColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  void OnLButtonDown(UINT nFlags, CPoint point);
  //void CMFCTestDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
  void CMFCTestDlg::OnMouseMove(UINT nFlags, CPoint point);
  void CMFCTestDlg::OnLButtonUp(UINT nFlags, CPoint point);
  void CMFCTestDlg::OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnPaint();
  afx_msg void OnStnClickedIdsPictypeBitmap();
  afx_msg void OnEnChangeThick();
  afx_msg void OnDeltaposThickSpin(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void UpdateControls(int nThickness);
  afx_msg void OnBnClickedDrawResetBtn();
  afx_msg void OnBnClickedRandom();
  afx_msg void OnBnImgOpen();

public:
  afx_msg void OnBnClickedImgopn();

};