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
  std::vector<CPoint> m_Points; // 점 저장용 벡터
  void DrawCircleFromPoints(CDC& dc); // 원을 그리는 함수
  int CalculateCircle(const std::vector<CPoint>& points, CPoint& center); // 원 계산 함수

  afx_msg
  BOOL OnInitDialog();
  void OnLButtonDown(UINT nFlags, CPoint point); // 마우스 클릭 핸들러
  afx_msg void OnPaint(); // 화면 그리기 핸들러

  afx_msg void OnStnClickedIdsPictypeBitmap(); // Owner Draw 처리기
public:
  afx_msg void OnEnChangeThick();
  afx_msg void OnDeltaposThickSpin(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnBnClickedDrawResetBtn();

  afx_msg void OnBnClickedRandom();
};