#include "pch.h"
#include "framework.h"
#include "MFCTest.h"
#include "MFCTestDlg.h"
#include "afxdialogex.h"
#include "afx.h"
#include "opencv2/opencv.hpp"
using namespace cv;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMFCTestDlg::CMFCTestDlg(CWnd* pParent)
  : CDialogEx(IDD_MFCTEST_DIALOG, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCTestDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCTestDlg, CDialogEx)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_WM_CTLCOLOR()
  ON_WM_PAINT()
  ON_CONTROL_REFLECT(STN_CLICKED, &CMFCTestDlg::OnStnClickedIdsPictypeBitmap)
  ON_EN_CHANGE(IDC_EDIT1, &CMFCTestDlg::OnEnChangeThick)
  ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &CMFCTestDlg::OnDeltaposThickSpin)
  ON_BN_CLICKED(IDC_BUTTON1, &CMFCTestDlg::OnBnClickedDrawResetBtn)
  ON_BN_CLICKED(IDC_RANDOM, &CMFCTestDlg::OnBnClickedRandom)
  ON_BN_CLICKED(IDC_IMGOPN, &CMFCTestDlg::OnBnImgOpen)
  ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


BOOL CMFCTestDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();
  SetDlgItemInt(IDC_EDIT1, 1);//Edit Control 값에 기본값 초기화
  m_nThickness = 1;
  nParameterMinRadius = 20;
  //////////////////////////////Reset버튼관련
  m_btnReset.SubclassDlgItem(IDC_RESETBTN, this);
  m_btnReset.SetWindowTextW(_T(""));//Icon Aign 문제
  m_btnReset.SetImage(IDB_RESETUP, IDB_RESETDOWN);
  m_btnReset.SizeToContent();
  m_btnReset.MoveWindow(50, 400, 150, 70);
  m_btnReset.SetFaceColor(RGB(100, 110, 180), true);


  //////////////////////////////Random버튼관련
  m_btnRandom.SubclassDlgItem(IDC_RANDOM, this);
  m_btnRandom.SetWindowTextW(_T(""));//Icon Aign 문제
  m_btnRandom.SetImage(IDB_BNRANDOMUP, IDB_BNRANDOMDOWN);
  m_btnRandom.SizeToContent();
  m_btnRandom.MoveWindow(350, 400, 150, 70);
  m_btnRandom.SetFaceColor(RGB(100, 110, 180), true);

  //스핀 컨트롤 초기화
  CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin)
  {
    pSpin->SetRange(1, 100);//최소값1, 최대값100 
    pSpin->SetPos(10);//초기값10
  }

  m_BackGroundColor.CreateSolidBrush(RGB(100, 110, 180));
  //클라이언트 배경색상

  //m_bmpOK.LoadBitmap(IDB_OK1);
  //m_btnOK.SetBitmap(m_bmpOK);
  CMFCTestDlg::OnBnClickedRandom();
  CMFCTestDlg::OnBnClickedDrawResetBtn(); //최초 원이 그려지지 않는 현상 초기화
  OnPaint(); //새로고침
  return TRUE;
}

HBRUSH CMFCTestDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

  //Dlg 배경색 설정
  if(nCtlColor == CTLCOLOR_DLG)
    return m_BackGroundColor;
  return hbr;
}

struct Circle
{
  CPoint pCenter;
  int nDefultRadius = 0;
  //그려진 원을 저장
};

bool m_bCircleCreated = false;
std::vector<Circle> m_Circles;        //클릭으로 생성한 원
std::vector<Circle> m_DraggedCircles; //드래그로 생성한 원

void CMFCTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  int nRadius = 0;

  CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
  if(!pStaticCtrl)
    return;
  if(!m_Circles.empty() && !m_Points.empty() &&
      !m_DraggedCircles.empty())
  {
    m_Points.clear();
    m_Circles.clear();
    m_Points.clear();
    m_DraggedCircles.empty();
    Invalidate();
    //화면 갱신 및 초기화 영역
  }

  CRect CRectStatic;
  pStaticCtrl->GetClientRect(&CRectStatic);
  pStaticCtrl->ClientToScreen(&CRectStatic);
  ScreenToClient(&CRectStatic);

  if(!CRectStatic.PtInRect(point))
    return;

  //드래그로 원 고정
  if(m_bIsDragging)
  {
    m_DraggedCircles.clear();
    m_bIsDragging = false;
    ReleaseCapture();//마우스 드래그

    nRadius = static_cast<int>(sqrt(pow(m_EndPoint.x - m_StartPoint.x, 2) +
                                    pow(m_EndPoint.y - m_StartPoint.y, 2)));

    if(nRadius > 5)//최소 Radius
    {
      CPoint pCenter((m_StartPoint.x + m_EndPoint.x) / 2,
                     (m_StartPoint.y + m_EndPoint.y) / 2);

      pCenter = CircleToInStaticArea(pCenter,
                                     nRadius,
                                     CRectStatic);

      //원을 고정 리스트(m_DraggedCircles)에 추가
      m_DraggedCircles.push_back({pCenter, nRadius});

      //디버깅 메시지 (고정된 원 확인)
      //TRACE(_T("외접x(%d)외접y(%d)지름:(%d)\n"), pCenter.x, pCenter.y, nRadius);
    }
    //화면 갱신
    Invalidate(TRUE);
  }
  else
  {//드래그 할 때 잔상이 남는 부분을 개선 25.02.02
    m_StartPoint = AdjustPointToStaticArea(point, CRectStatic);
    m_Points.clear(); //도트 삭제
    m_Circles.clear(); //원삭제
    m_DraggedCircles.clear(); //드래그 원 초기화
    m_bCircleCreated = false; //드래그 변수 초기화
    m_bIsDragging = false; //드래그 변수 초기화
    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE); //다시 그림
    Invalidate();//새로고침
    UpdateWindow();//새로고침
    m_EndPoint = m_StartPoint; //스타트포인트 엔드포인트
    m_bIsDragging = true; //드래그 변수 초기화
    SetCapture();
  }

  CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCTestDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
  int nDotRadius = 5;
  int nOuterRadius = 5;
  CPoint pCircleCenter;
  CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
  if(!pStaticCtrl)
    return;

  CRect CRectStatic;
  pStaticCtrl->GetClientRect(&CRectStatic);
  pStaticCtrl->ClientToScreen(&CRectStatic);
  ScreenToClient(&CRectStatic);

  if(!CRectStatic.PtInRect(point))
    return;
  if(m_Points.size() == 3)
    m_Points.clear();
  //첫 번째 Dot을 찍을 때 기존 원 삭제 (Dot 원 및 Drag 원)
  if(m_Points.empty())
  {
    m_Circles.clear();       //기존 Dot 원 삭제
    m_DraggedCircles.clear(); //기존 Drag 원 삭제
    Invalidate();             //리프레시
  }
  CPoint pDotCenter = AdjustPointToStaticArea(point, CRectStatic);

  if(std::find(m_Points.begin(), m_Points.end(), pDotCenter) == m_Points.end())
    m_Points.push_back(pDotCenter);
  //중복 방지

  if(m_Points.size() == 3) //도트가 3개면 원 생성
  { 
    //m_Circles.clear();
    nOuterRadius = CalculateCircle(m_Points, 
                                   pCircleCenter, 
                                   CRectStatic);
    //Radius 0보다 클 경우
    if(nOuterRadius > 0)
    {
      //원을 스태틱 내부로 조정
      pCircleCenter = CircleToInStaticArea(pCircleCenter,
                                           nOuterRadius,
                                           CRectStatic);
      //원 추가
      m_Circles.push_back({pCircleCenter,
                           nOuterRadius});
    }
    //m_Points.clear();
  }

  //화면 갱신
  Invalidate();
  UpdateWindow();
  CDialogEx::OnRButtonDown(nFlags, point);
}

void CMFCTestDlg::OnPaint()
{
  CPaintDC dc(this);
  CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
  CRgn Rgn;
  int nRadius = 0;

  if(pStaticCtrl != nullptr)
  {
    CRect ClientRect;
    pStaticCtrl->GetClientRect(&ClientRect);
    Rgn.CreateRectRgn(ClientRect.left,
                      ClientRect.top,
                      ClientRect.right,
                      ClientRect.bottom);
    //원을 그릴 수 있는 클라이언트 크기

    dc.SelectClipRgn(&Rgn);
    //드래그 중인 원 실시간 표시
    if(m_bIsDragging)
    {
      nRadius = static_cast<int>(sqrt(pow(m_EndPoint.x - m_StartPoint.x, 2) +
                                      pow(m_EndPoint.y - m_StartPoint.y, 2)));

      CPoint pCenter((m_StartPoint.x + m_EndPoint.x) / 2,
                     (m_StartPoint.y + m_EndPoint.y) / 2);

      CPen pen(PS_DASH, 2, RGB(255, 0, 0));
      CPen* pOldPen = dc.SelectObject(&pen);

      dc.Ellipse(pCenter.x - nRadius,
                 pCenter.y - nRadius,
                 pCenter.x + nRadius,
                 pCenter.y + nRadius);

      dc.SelectObject(pOldPen);
    }

    //원을 그려도 도트 사라지지 않도록
    if(!m_Points.empty())
    {
      CPen dotPen(PS_SOLID, 1, RGB(255, 0, 0));
      //도트 색상
      CPen* pOldPen = dc.SelectObject(&dotPen);
      CBrush dotBrush(RGB(255, 0, 0));
      CBrush* pOldBrush = dc.SelectObject(&dotBrush);

      for(const auto& point : m_Points)
      {
        dc.Ellipse(point.x - 3,
                   point.y - 3,
                   point.x + 3,
                   point.y + 3);
      }

      dc.SelectObject(pOldPen);
      dc.SelectObject(pOldBrush);
    }

    //드래그로 원을 생성 할 경우 조건문
    if(!m_DraggedCircles.empty())
    {
      m_Points.clear();  //도트 제거
    }

    //도트로 생성된 원을 중복 없이 그림
    if(!m_Circles.empty())
    {
      CPen pen(PS_SOLID, m_nThickness, RGB(0, 0, 255));
      CPen* pOldPen = dc.SelectObject(&pen);

      for(const auto& circle : m_Circles)
      {
        dc.Ellipse(circle.pCenter.x - circle.nDefultRadius,
                   circle.pCenter.y - circle.nDefultRadius,
                   circle.pCenter.x + circle.nDefultRadius,
                   circle.pCenter.y + circle.nDefultRadius);
      }
      dc.SelectObject(pOldPen);
    }

    //드래그로 생성된 원을 그리기
    if(!m_DraggedCircles.empty())
    {
      CPen pen(PS_SOLID, m_nThickness, RGB(0, 255, 0));
      CPen* pOldPen = dc.SelectObject(&pen);

      for(const auto& circle : m_DraggedCircles)
      {
        dc.Ellipse(circle.pCenter.x - circle.nDefultRadius,
                   circle.pCenter.y - circle.nDefultRadius,
                   circle.pCenter.x + circle.nDefultRadius,
                   circle.pCenter.y + circle.nDefultRadius);
        //드래그 원 Radius 계산
      }
      dc.SelectObject(pOldPen);//Object에 드래그 원 그리는 부분
    }

    dc.SelectClipRgn(nullptr);
  }
}

void CMFCTestDlg::OnDrawCircle(CDC& CDC, CRect& CRect)
{
  int nRadius = 0;
  if(m_Circles.empty()) //원이 없을 경우 리턴
    return;
  for(const auto& circle : m_Circles)
  {
    CPoint pCenterPoint = AdjustPointToStaticArea(circle.pCenter, CRect);
    nRadius = circle.nDefultRadius;

    if(nRadius > 0 &&
      pCenterPoint.x - nRadius >= CRect.left &&
      pCenterPoint.x + nRadius <= CRect.right &&
      pCenterPoint.y - nRadius >= CRect.top &&
      pCenterPoint.y + nRadius <= CRect.bottom)
    {//Rect 내부에서 원 생성
      CPen Cpen(PS_SOLID, m_nThickness, RGB(0, 0, 255));
      CPen* pOldPen = CDC.SelectObject(&Cpen);

      CDC.Ellipse(pCenterPoint.x - nRadius,
                  pCenterPoint.y - nRadius,
                  pCenterPoint.x + nRadius,
                  pCenterPoint.y + nRadius);
      //원 생성
      CDC.SelectObject(pOldPen);
    }
  }
}

void CMFCTestDlg::DrawCircleFromPoints(CDC& dc)
{
  CRgn Rgn;
  CPoint p_Center;
  CRect C_RectPicture;
  int nRadius = 0;

  //Picture Control영역 가져오기
  GetDlgItem(IDC_DRAWSTATIC)->GetClientRect(&C_RectPicture);
  GetDlgItem(IDC_DRAWSTATIC)->ClientToScreen(&C_RectPicture);
  ScreenToClient(&C_RectPicture);
  Rgn.CreateRectRgn(C_RectPicture.left,
                    C_RectPicture.top,
                    C_RectPicture.right,
                    C_RectPicture.bottom);
  //Region 사각형 생성
  dc.SelectClipRgn(&Rgn);

  if(m_Points.size() == 3)
  {
    nRadius = CalculateCircle(m_Points, p_Center, C_RectPicture);
    if(nRadius > 0)
    {
      //원을 리스트에 추가
      Circle newCircle;
      newCircle.pCenter = p_Center;
      newCircle.nDefultRadius = nRadius;
      m_Circles.push_back(newCircle);

      //기존 원 유지
      for(const auto& CCircle : m_Circles)
      {
        CPen pen(PS_SOLID, m_nThickness, RGB(0, 0, 0)); //두께와 색상 설정
        CPen* pOldPen = dc.SelectObject(&pen);

        dc.Ellipse(CCircle.pCenter.x - CCircle.nDefultRadius,
                   CCircle.pCenter.y - CCircle.nDefultRadius,
                   CCircle.pCenter.x + CCircle.nDefultRadius,
                   CCircle.pCenter.y + CCircle.nDefultRadius);
        //원 생성
        dc.SelectObject(pOldPen);
      }
    }
  }

  dc.SelectClipRgn(nullptr);
}

//3개의 점으로 원을 계산하는 함수
int CMFCTestDlg::CalculateCircle(const std::vector<CPoint>& points, CPoint& center, CRect& CRectStatic)
{
  int nMinX = 0;
  int nMaxX = 0;
  int nMinY = 0;
  int nMaxY = 0;
  int nDist = 0;
  int nRadius = 0;
  int nMaxDist = 0;
  int nTempRadius = 0;
  int nFabsRadius = 0;
  int nMaxRadiusX = 0;
  int nMaxRadiusY = 0;
  int nMaxRadius = 0;
  double dX1 = 0.0;
  double dY1 = 0.0;
  double dX2 = 0.0;
  double dY2 = 0.0;
  double dX3 = 0.0;
  double dY3 = 0.0;
  double dFabs = 0.0;
  double dFinalX = 0.0;
  double dFinalY = 0.0;
  double dTripleLine = 0.0;
  double dSqrt = 0.0;
  double dMaxDist = 0.0;
  //변수 선언 및 초기화

  if(points.size() != 3)
    return 0; //예외 처리:반드시 3개의 점이 필요

  //중심점 계산 (세 점의 무게중심)
  center.x = (points[0].x + 
              points[1].x +
              points[2].x) / 3;

  center.y = (points[0].y +
              points[1].y + 
              points[2].y) / 3;

  //Radius계산
  for(const auto& point:points)
  {
    nDist = static_cast<int>(sqrt(pow(point.x - center.x, 2) +
                                  pow(point.y - center.y, 2)));

    nMaxDist = std::max(nMaxDist, nDist);
  }
  nRadius = nMaxDist;

  
  center = CircleToInStaticArea(center, nRadius, CRectStatic);

  
  nMaxRadiusX = std::min(center.x -
                         CRectStatic.left,
                         CRectStatic.right -
                         center.x);

  nMaxRadiusY = std::min(center.y -
                         CRectStatic.top,
                         CRectStatic.bottom -
                         center.y);
  //반지름이 스태틱을 벗어나지 않도록 제한
  nMaxRadius = std::min(nMaxRadiusX, nMaxRadiusY);

  nRadius = std::min(nRadius, nMaxRadius);//반지름을 제한

  return std::max(nRadius, 10);//최소 반지름 보장

  dX1 = points[0].x;
  dY1 = points[0].y;
  dX2 = points[1].x;
  dY2 = points[1].y;
  dX3 = points[2].x;
  dY3 = points[2].y;

  dFabs = dX1 * (dY2 - dY3) +
          dX2 * (dY3 - dY1) +
          dX3 * (dY1 - dY2);

  if(fabs(dFabs) < 1e-5) //예외 처리
  {
    //디버깅 메세지
    //TRACE(_T("예외 처리하여 큰 원 생성.\n"));
    center.x = (points[0].x + points[2].x) / 2;
    center.y = (points[0].y + points[2].y) / 2;

    nFabsRadius = static_cast<int>(sqrt(pow(points[2].x - points[0].x, 2) +
                                        pow(points[2].y - points[0].y, 2)) / 2);

    return max(nFabsRadius, 10); //원의 Radius 계산 부분
  }

  dFinalX = ((dX1 * dX1 + dY1 * dY1) * (dY2 - dY3) +
             (dX2 * dX2 + dY2 * dY2) * (dY3 - dY1) +
             (dX3 * dX3 + dY3 * dY3) * (dY1 - dY2)) / (2 * dFabs);

  dFinalY = ((dX1 * dX1 + dY1 * dY1) * (dX3 - dX2) +
             (dX2 * dX2 + dY2 * dY2) * (dX1 - dX3) +
             (dX3 * dX3 + dY3 * dY3) * (dX2 - dX1)) / (2 * dFabs);
  //3개의 좌표로 중심점 좌표 계산
  if(abs(dFinalX) > 10000 || abs(dFinalY) > 10000)
  {
    //TRACE(_T("중심값이 너무커서 보정\n"));
    //디버깅 메세지
    dFinalX = (points[0].x + points[1].x + points[2].x) / 3;
    dFinalY = (points[0].y + points[1].y + points[2].y) / 3;
  }

  center = CPoint(static_cast<int>(dFinalX),
                  static_cast<int>(dFinalY));

  nRadius = static_cast<int>(sqrt((dFinalX - dX1) * (dFinalX - dX1) +
                                  (dFinalY - dY1) * (dFinalY - dY1)));
  //세 점을 기점으로 Radius값을 지정
  if(nRadius < 10)
    nRadius = 10;
  //디버깅 메세지
  //TRACE(_T("외접x(%d)외접y(%d)지름:(%d)\n"), center.x, center.y, nRadius);
  return nRadius;
}

void CMFCTestDlg::OnStnClickedIdsPictypeBitmap()
{
  //여기에 필요한 추가적인 처리를 넣을 수 있습니다.
}

void CMFCTestDlg::OnDeltaposThickSpin(NMHDR* pNMHDR, LRESULT* pResult)
{//두께 Parameter 함수
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

  //현재 Edit Control의 값을 가져옴
  CString strThickness;
  int nThickness;
  strThickness = "";
  nThickness = 0;
  //변수 초기화
  GetDlgItemText(IDC_EDIT1, strThickness);
  nThickness = _ttoi(strThickness);

  if(strThickness.IsEmpty())
    nThickness = 1;
  nThickness += pNMUpDown->iDelta;
  if(nThickness < 1)
    nThickness = 1;
  if(nThickness > 100)
    nThickness = 100;
  //두께 최소 최대값 조건문
  SetDlgItemInt(IDC_EDIT1, nThickness);
  m_nThickness = nThickness;

  //에디트 컨트롤 값 업데이트
  Invalidate();
  UpdateWindow();
  //화면update.

  *pResult = 0;
}


void CMFCTestDlg::OnEnChangeThick()
{
  static bool bUpdate;
  int nThickness;
  CString strThickness;
  bUpdate = true;
  CSpinButtonCtrl* pSpin;
  if(bUpdate)
    return;
  strThickness = "";
  nThickness = 0;
  //변수 선언 및초기화
  GetDlgItemText(IDC_EDIT1, strThickness);

  if(!strThickness.IsEmpty())
    nThickness = _ttoi(strThickness);


  if(nThickness < 1)
    nThickness = 1;
  if(nThickness > 100)
    nThickness = 100;
  //두께 값의 Min/Max 제한
  
  m_nThickness = nThickness;
  pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin && ::IsWindow(pSpin->m_hWnd))
    pSpin->SetPos(nThickness);
  //스핀 컨트롤 값 업데이트

  SetDlgItemInt(IDC_EDIT1, nThickness);
  bUpdate = false;
  //에디트 컨트롤 값 업데이트
}

void CMFCTestDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
  *pResult = 0;
}

void CMFCTestDlg::UpdateControls(int nThickness)
{
  CSpinButtonCtrl* pSpin;
  static bool bUpdate = false;
  //변수 선언 및 초기화

  if(bUpdate)
    return;
  bUpdate = true;
  pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin && ::IsWindow(pSpin->m_hWnd))
    pSpin->SetPos(nThickness);

  SetDlgItemInt(IDC_EDIT1, nThickness);
  //에디트 컨트롤 값 업데이트

  Invalidate();
  UpdateWindow();
  bUpdate = false;
  //화면 업데이트
}

void CMFCTestDlg::OnBnClickedDrawResetBtn()
{
  m_Points.clear();
  m_Circles.clear();
  //전역변수 클리어

  m_DraggedCircles.clear();
  m_bCircleCreated = false;
  m_bIsDragging = false;
  //드래그 변수 초기화

  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
  Invalidate();
  UpdateWindow();
  //화면 업데이트
}

void CMFCTestDlg::OnBnClickedRandom()
{
  double dAngle = 0.0;
  int nMaxRadius = 0;
  int nRadius = 0;
  int nCenterX = 0;
  int nCenterY = 0;
  int nPointX = 0;
  int nPointY = 0;
  CRect C_RectPicture;
  //변수 선언 및 초기화

  GetDlgItem(IDC_DRAWSTATIC)->GetClientRect(&C_RectPicture);

  //랜덤 원 생성 범위 설정
  nMaxRadius = std::max(C_RectPicture.Width(),
                        C_RectPicture.Height()) / 4;

  nRadius = rand() % nMaxRadius + 10;

  nCenterX = rand() % (C_RectPicture.Width() - 2 * nRadius) + nRadius;  

  nCenterY = rand() % (C_RectPicture.Height() - 2 * nRadius) + nRadius;

  CPoint pCenter(nCenterX, nCenterY);

  //기존 원과 DOT을 삭제하고 새로운 원을 추가
  m_DraggedCircles.clear(); //25.02.02 드래그 원 클리어 예외처리 추가
  m_Circles.clear();
  m_Points.clear(); //이전 DOT 제거
  Invalidate();
  //랜덤 원 추가
  m_Circles.push_back({pCenter, nRadius - 5});

  //새로운 원에 맞게 3개의 DOT을 원의 외곽에 균등 배치
  for(int i = 0; i < 3; i++)
  {
    //원 둘레에서 3개의 균등한 각도를 랜덤 배치
    dAngle = (i * 120 + (rand() % 20 - 10)) * (3.14 / 180.0);

    nPointX = static_cast<int>(nCenterX + (nRadius - 5) * cos(dAngle));
    nPointY = static_cast<int>(nCenterY + (nRadius - 5) * sin(dAngle));

    //DOT이 STATIC 내부에 위치하도록 보정
    if(nPointX < C_RectPicture.left)
      nPointX = C_RectPicture.left;
    if(nPointX > C_RectPicture.right)
      nPointX = C_RectPicture.right;
    if(nPointY < C_RectPicture.top)
      nPointY = C_RectPicture.top;
    if(nPointY > C_RectPicture.bottom)
      nPointY = C_RectPicture.bottom;

    CPoint newDot(nPointX, nPointY);
    m_Points.push_back(newDot);
  }

  //화면 갱신
  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
  m_Points.clear();
  //25.02.02 Random 버튼 이후에도 Dot가 계속 생성이 되는 현상 예외처리

}

void CMFCTestDlg::DrawTriangleFromPoints(CDC& dc)
{ //테스트용 함수
  if(m_Points.size() != 3)
    return;

  CPen pen(PS_SOLID, 2, RGB(255, 0, 0));
  CPen* pOldPen = dc.SelectObject(&pen);

  dc.MoveTo(m_Points[0]);
  dc.LineTo(m_Points[1]);
  dc.LineTo(m_Points[2]);
  dc.LineTo(m_Points[0]);//삼각형 완성
  dc.SelectObject(pOldPen);
}

void CMFCTestDlg::OnBnClickedImgopn()
{
  CRect C_RectPicture;
  int nRadius = 0;
  int nMaxRadius = 0;
  int nCenterX = 0;
  int nCenterY = 0;
  int nPointX = 0;
  int nPointY = 0;
  bool bValid = true;
  double dAngle = 0.0;
  double dDistance = 0.0;
  //========================변수 선언==========================

  GetDlgItem(IDC_DRAWSTATIC)->GetClientRect(&C_RectPicture);

  m_Points.clear();
  m_Circles.clear();

  nMaxRadius = std::max(C_RectPicture.Width(),
                        C_RectPicture.Height()) / 4;
  //Random 원의 최소 크기 지정
  nRadius = rand() % nMaxRadius + 10;
  //Random 원의 최대 크기 지정
  nCenterX = rand() % (C_RectPicture.Width() - 2 * 
                       nRadius) + nRadius;
  //Random 원의 Width 값. 
  nCenterY = rand() % (C_RectPicture.Height() - 2 * 
                       nRadius) + nRadius;
  //Random 원의 Height 값. 

  CPoint pCenter(nCenterX, //Random 원의 Center점 지정
                 nCenterY);
  Circle RandomCircle = {pCenter, //Random 원의 Center점 지정
                         nRadius};
  //크기는 nRadius값에서 할당

  m_Circles.push_back(RandomCircle);
  //원의 외곽에 3개의 점 생성

  for(int i = 0; i < 3; i++)
  {
    dAngle = rand() % 360 * (3.14 / 180.0);
    nPointX = static_cast<int>(nCenterX +
                               nRadius *
                               cos(dAngle));
    nPointY = static_cast<int>(nCenterY +
                               nRadius *
                               sin(dAngle));
    //좌표값은 Static내부로
    bValid = true;//공식으로 원의 외곽지점
    for(const auto& existingPoint : m_Points)
    {//점 3개가 너무 일직선 상에 있으면 원이 생성이되지않는 버그 확인
      dDistance = sqrt(pow(existingPoint.x - nPointX, 2) +
                       pow(existingPoint.y - nPointY, 2));
      if(dDistance < 10)
      {
        bValid = false;
        break;
      }
    }
    if(bValid)
      m_Points.push_back(CPoint(nPointX,
                                nPointY));
    else
      --i;
  }
  Invalidate();
  UpdateWindow();
  //새로고침
}

void CMFCTestDlg::OnBnImgOpen()
{

}

CPoint CMFCTestDlg::AdjustPointToStaticArea(const CPoint& point, const CRect& rect)
{
  CPoint CPpoint = point;

  //X좌표 조정
  if(point.x < rect.left)
    CPpoint.x = rect.left;
  else if(point.x > rect.right)
    CPpoint.x = rect.right;

  //Y좌표 조정
  if(point.y < rect.top)
    CPpoint.y = rect.top;
  else if(point.y > rect.bottom)
    CPpoint.y = rect.bottom;

  return CPpoint;
}

CPoint CMFCTestDlg::CircleToInStaticArea(CPoint pCenter, int& nRadius, const CRect& CRect)
{
  int nMaxRadiusX = 0;
  int nMaxRadiusY = 0;
  int nMaxAllowedRadius = 0;

  if(pCenter.x - nRadius < CRect.left)
    pCenter.x = CRect.left + nRadius;
  if(pCenter.x + nRadius > CRect.right)
    pCenter.x = CRect.right - nRadius;
  if(pCenter.y - nRadius < CRect.top)
    pCenter.y = CRect.top + nRadius;
  if(pCenter.y + nRadius > CRect.bottom)
    pCenter.y = CRect.bottom - nRadius;
  //==============위치좌표 보정===========


  //최대 반지름 제한
  nMaxRadiusX = std::min(pCenter.x -
                         CRect.left,
                         CRect.right -
                         pCenter.x);

  nMaxRadiusY = std::min(pCenter.y - 
                         CRect.top, 
                         CRect.bottom - 
                         pCenter.y);

  nMaxAllowedRadius = std::min(nMaxRadiusX, nMaxRadiusY);

  if(nRadius > nMaxAllowedRadius)
    nRadius = nMaxAllowedRadius;

  return pCenter;
}

void CMFCTestDlg::OnMouseMove(UINT nFlags, CPoint point)
{
  int nRadius = 0;
  if(m_bIsDragging)
  {
    CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
    if(!pStaticCtrl) return;

    CRect CRectStatic;
    pStaticCtrl->GetClientRect(&CRectStatic);
    pStaticCtrl->ClientToScreen(&CRectStatic);
    ScreenToClient(&CRectStatic);

    //마우스 위치 보정
    point = AdjustPointToStaticArea(point, CRectStatic);

    //잔상 제거예외처리
    CRect prevRect(m_StartPoint.x - m_nPrevRadius,
                   m_StartPoint.y - m_nPrevRadius,
                   m_StartPoint.x + m_nPrevRadius,
                   m_StartPoint.y + m_nPrevRadius);

    //Radius
    m_EndPoint = point;
    nRadius = static_cast<int>(sqrt(pow(m_EndPoint.x - m_StartPoint.x, 2) +
                                    pow(m_EndPoint.y - m_StartPoint.y, 2)));

    //중심점
    CPoint pCenter((m_StartPoint.x + m_EndPoint.x) / 2,
                   (m_StartPoint.y + m_EndPoint.y) / 2);

    CRect newRect(pCenter.x - nRadius,
                  pCenter.y - nRadius,
                  pCenter.x + nRadius,
                  pCenter.y + nRadius);

    //이전 원 설정을 False로 바꿔 잔상을 방지 예외처리
    InvalidateRect(&prevRect, TRUE);
    InvalidateRect(&newRect, FALSE);
    m_nPrevRadius = nRadius;
  }

  CDialogEx::OnMouseMove(nFlags, point);
}


void CMFCTestDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
  int nRadius = 0;
  CPoint pCenter;
  CRect CRectStatic;

  if(m_bIsDragging)
  {
    m_bIsDragging = false;
    ReleaseCapture(); //마우스 캡처 해제

    CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
    if(!pStaticCtrl)
      return;

    pStaticCtrl->GetClientRect(&CRectStatic);
    pStaticCtrl->ClientToScreen(&CRectStatic);
    ScreenToClient(&CRectStatic);

    if(!CRectStatic.PtInRect(point))
      return; //Static 영역 내부에서만 원 생성이 되도록 예외처리
    

    point = AdjustPointToStaticArea(point, CRectStatic);
    m_EndPoint = point;

    //Radius 계산
    nRadius = static_cast<int>(sqrt(pow(m_EndPoint.x - m_StartPoint.x, 2) +
                                    pow(m_EndPoint.y - m_StartPoint.y, 2)));

    if(nRadius > 5) //Min Radius
    {
      pCenter = CPoint((m_StartPoint.x + m_EndPoint.x) / 2,
                       (m_StartPoint.y + m_EndPoint.y) / 2);
      pCenter = CircleToInStaticArea(pCenter, nRadius, CRectStatic);

      //Drag 원이 생성될 때 기존 Dot 원 및 Dot 삭제
      if(!m_Circles.empty()&&!m_Points.empty()&&!m_DraggedCircles.empty())
      {
        m_Points.clear();
        m_Circles.clear();
        m_Points.clear();
        m_DraggedCircles.empty();
        Invalidate();
        //전부 삭제 및 화면 갱신
      }

      //기존 Drag 원을 삭제하고 새로운 원을 추가
      m_DraggedCircles.clear();
      m_DraggedCircles.push_back({pCenter, nRadius});

      //화면 갱신
      Invalidate();
      UpdateWindow();
    }
  }

  CDialogEx::OnLButtonUp(nFlags, point);
}

void CMFCTestDlg::AdjustCircleRadiusToStaticArea(CPoint& pCenter, int& nRadius, const CRect& CRect)
{
  int nMaxRadiusX = 0;
  int nMaxRadiusY = 0;
  int nMaxAllowedRadius = 0;

  //Static 내부에서 그려질수 있는 반지름 계산
  nMaxRadiusX = min(pCenter.x -
                    CRect.left,
                    CRect.right -
                    pCenter.x);

  nMaxRadiusY = min(pCenter.y -
                    CRect.top,
                    CRect.bottom -
                    pCenter.y);

  nMaxAllowedRadius = min(nMaxRadiusX,
                          nMaxRadiusY);

  nRadius = min(nRadius,
                nMaxAllowedRadius);
  //Static 내부에서 그려질수 있는 중심좌표 계산
}

void CMFCTestDlg::LimitRadiusCircle(CPoint& pCenter, int& nRadius, const CRect& CRect)
{
  int nMaxRadiusX = 0;
  int nMaxRadiusY = 0;
  int nMaxAllowedRadius = 0;
  //Static 내부에서 그려질수 있는 최대 반지름 계산
  nMaxRadiusX = min(pCenter.x -
                    CRect.left,
                    CRect.right -
                    pCenter.x);

  nMaxRadiusY = min(pCenter.y -
                    CRect.top,
                    CRect.bottom - 
                    pCenter.y);

  nMaxAllowedRadius = min(nMaxRadiusX,
                          nMaxRadiusY);

  //중심좌표와 반지름을 Static 내부에서 비율 맞춰 조절
  nRadius = min(nRadius, 
                nMaxAllowedRadius);

  //디버깅 메시지 추가
  //TRACE(_T("반지름 조정: 중심(%d, %d), 반지름: %d\n"),
  //pCenter.x, pCenter.y, nRadius);
}

BOOL CMFCTestDlg::OnEraseBkgnd(CDC* pDC)
{
  return TRUE; //화면 깜빡문제 해결
}