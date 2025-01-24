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
  void DrawCircleFromDrag(CDC& dc);
  int CalculateCircle(const std::vector<CPoint>& points, CPoint& center);
  CBrush m_BackGroundColor;
  CBitmap m_BtnImageTest;
  CBitmap m_BtnImage;
  CBitmapButton m_btnImage;
  CMFCButton m_btnReset;
  CMFCButton m_btnRandom;

  CRgn m_CircleRgn;
  CPen m_CirclrPen;
  CPen* m_pOldPen;

  afx_msg
  BOOL OnInitDialog();
  HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  //HBRUSH OnDialogColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnPaint();
  afx_msg void OnStnClickedIdsPictypeBitmap();
  afx_msg void OnEnChangeThick();
  afx_msg void OnDeltaposThickSpin(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void UpdateControls(int nThickness);
  afx_msg void OnBnClickedDrawResetBtn();
  afx_msg void OnBnClickedRandom();
};