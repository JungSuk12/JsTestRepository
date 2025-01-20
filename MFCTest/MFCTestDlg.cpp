﻿#include "pch.h"
#include "framework.h"
#include "MFCTest.h"
#include "MFCTestDlg.h"
#include "afxdialogex.h"

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
  ON_WM_PAINT()
  ON_CONTROL_REFLECT(STN_CLICKED, &CMFCTestDlg::OnStnClickedIdsPictypeBitmap)
  ON_EN_CHANGE(IDC_EDIT1, &CMFCTestDlg::OnEnChangeThick)
  ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &CMFCTestDlg::OnDeltaposThickSpin)
  ON_BN_CLICKED(IDC_BUTTON1, &CMFCTestDlg::OnBnClickedDrawResetBtn)
  ON_BN_CLICKED(IDC_RANDOM, &CMFCTestDlg::OnBnClickedRandom)
END_MESSAGE_MAP()

BOOL CMFCTestDlg::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  //Edit Control에 기본값 1을 설정
  SetDlgItemInt(IDC_EDIT1, 1);
  m_nThickness = 1;

  //스핀 컨트롤 초기화
  CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if (pSpin) 
  {
    pSpin->SetRange(1, 100);  //최소값 1, 최대값 100
    pSpin->SetPos(10);       //초기값 10
  }

  return TRUE;  //포커스를 컨트롤에 설정하려면 TRUE를 반환
}
struct Circle 
{
  CPoint cCenter;
  int nDefultRadius = 0;
};
std::vector<Circle> m_Circles; //그려진 원을 저장

void CMFCTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  m_Points.push_back(point);
  
  Invalidate();//새로고침

  //점이 3개 이상이면 마지막 3개의 점으로 원을 계산하고 그리기
  if (m_Points.size() >= 3)
  {
    CPoint pCenterPoint;
    CDC* pDC;
    int nRadius = 0;
    std::vector<CPoint> LastThreePoints =
    {
        m_Points[m_Points.size() - 3],
        m_Points[m_Points.size() - 2],
        m_Points[m_Points.size() - 1]
    };

    nRadius = CalculateCircle(LastThreePoints, pCenterPoint);
    if (nRadius > 0)
    {
      pDC = GetDlgItem(IDC_DRAWSTATIC)->GetDC();
      pDC->Ellipse(pCenterPoint.x - nRadius,
                   pCenterPoint.y - nRadius,
                   pCenterPoint.x + nRadius,
                   pCenterPoint.y + nRadius);
      ReleaseDC(pDC);
      //원 데이터를 m_Circles에 저장
      if (m_Circles.empty()) //비어있는 경우에만 원 추가
      {
        Circle newCircle = { pCenterPoint, nRadius };
        m_Circles.push_back(newCircle);
      }
      Invalidate();//새로고침
    }
  }
  CDialogEx::OnLButtonDown(nFlags, point);
}

// 화면 그리기 핸들러
void CMFCTestDlg::OnPaint()
{
  CDialogEx::OnPaint();
  CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
  if (pStaticCtrl != nullptr) 
  {
    CClientDC dc(pStaticCtrl);
    CRect rect;
    pStaticCtrl->GetClientRect(&rect); //초기화

    //영역을 흰색 배경으로 초기화
    dc.FillSolidRect(rect, RGB(255, 255, 255));

    //영역을 PictureControl 내부로 설정
    CRgn rgn;
    rgn.CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
    dc.SelectClipRgn(&rgn);

    //원 그리기
    for (const auto& circle : m_Circles) 
    {
      //원의 중심이 PictureControl 내부에 있을 때만 그리기
      if (rect.PtInRect(circle.cCenter)) 
      {
        CPen pen(PS_SOLID, m_nThickness, RGB(0, 0, 0));  //두께와 색상 설정
        CPen* pOldPen = dc.SelectObject(&pen);

        for (const auto& pDotPoint : m_Points)
        {
          dc.Ellipse(pDotPoint.x - 5, //Dot 그리는 부분
                     pDotPoint.y - 5,
                     pDotPoint.x + 5,
                     pDotPoint.y + 5);
        }
        
        dc.Ellipse(circle.cCenter.x - circle.nDefultRadius,
                   circle.cCenter.y - circle.nDefultRadius,
                   circle.cCenter.x + circle.nDefultRadius,
                   circle.cCenter.y + circle.nDefultRadius);

        dc.SelectObject(pOldPen);
      }
    }

    //외곽에 찍힌 점 그리기
    for (const auto& pDotPoint : m_Points)
    {
      dc.Ellipse(pDotPoint.x - 3,
                 pDotPoint.y - 3,
                 pDotPoint.x + 3,
                 pDotPoint.y + 3);
    }
    dc.SelectClipRgn(nullptr);
  }
}

//Owner Draw에서 원을 그리는 함수
void CMFCTestDlg::DrawCircleFromPoints(CDC& dc)
{
  CRgn Rgn;
  CPoint p_Center;
  int nRadius = 0;
  int nDotCount = 0;
  CRect C_RectPicture;
  //Picture Control의 클라이언트 영역 가져오기
  GetDlgItem(IDC_DRAWSTATIC)->GetClientRect(&C_RectPicture);
  GetDlgItem(IDC_DRAWSTATIC)->ClientToScreen(&C_RectPicture);
  ScreenToClient(&C_RectPicture);
  Rgn.CreateRectRgn(C_RectPicture.left, 
                    C_RectPicture.top,
                    C_RectPicture.right,
                    C_RectPicture.bottom);
  dc.SelectClipRgn(&Rgn);
  if (m_Points.size() == 3)
  {
    nRadius = CalculateCircle(m_Points, p_Center);
    if (nRadius > 0)
    {
      //설정된 두께로 원을 그림
      CPen pen(PS_SOLID, m_nThickness, RGB(0, 0, 0));  //두께와 색상 설정
      CPen* pOldPen = dc.SelectObject(&pen);

      dc.Ellipse(p_Center.x - nRadius,
                 p_Center.y - nRadius,
                 p_Center.x + nRadius,
                 p_Center.y + nRadius);

      dc.SelectObject(pOldPen);
    }
  }

	//마우스로 찍은 위치표시
  for (const auto& pDotPoint : m_Points)
  {
    dc.Ellipse(pDotPoint.x - 3,
               pDotPoint.y - 3,
               pDotPoint.x + 3,
               pDotPoint.y + 3);
  }

  //포인터 해제
  dc.SelectClipRgn(nullptr);
}

//3개의 점으로 원을 계산하는 함수
int CMFCTestDlg::CalculateCircle(const std::vector<CPoint>& Circlepoint, CPoint& CenterPoint)
{
  double dX1 = 0.0;
  double dX2 = 0.0;
  double dX3 = 0.0;
  double dY1 = 0.0;
  double dY2 = 0.0;
  double dY3 = 0.0;
  double dTriAngle = 0.0;
  double dX = 0.0;
  double dY = 0.0;
  
  dX1 = Circlepoint[0].x, 
  dY1 = Circlepoint[0].y;
  dX2 = Circlepoint[1].x, 
  dY2 = Circlepoint[1].y;
  dX3 = Circlepoint[2].x, 
  dY3 = Circlepoint[2].y;
  //변수 초기화영역


  if (Circlepoint.size() != 3) 
    return 0;
  //Circlepoint 3 이하인경우 리턴

  dTriAngle = dX1 * (dY2 - dY3) + dX2 *
             (dY3 - dY1) + dX3 * 
             (dY1 - dY2);

  if (fabs(dTriAngle) < 1e-6)
    return 0;
  //삼각형 외곽 면적 계산

  dX = ((dX1 * dX1 + dY1 * dY1) * (dY2 - dY3) +
        (dX2 * dX2 + dY2 * dY2) * (dY3 - dY1) +
        (dX3 * dX3 + dY3 * dY3) * (dY1 - dY2)) /
        (2 * dTriAngle);
  dY = ((dX1 * dX1 + dY1 * dY1) * (dX3 - dX2) +
        (dX2 * dX2 + dY2 * dY2) * (dX1 - dX3) +
        (dX3 * dX3 + dY3 * dY3) * (dX2 - dX1)) /
        (2 * dTriAngle);
	//중심점 계산 (이부분은 GPT 도움을 받아서 작성하였습니다.)

  CenterPoint = CPoint(static_cast<int>(dX),
                       static_cast<int>(dY));
  //중심점 dX, dY
  return static_cast<int>(sqrt((dX - dX1) * 
                               (dX - dX1) +
                               (dY - dY1) *
                               (dY - dY1)));
}
void CMFCTestDlg::OnStnClickedIdsPictypeBitmap()
{
  // 여기에 필요한 추가적인 처리를 넣을 수 있습니다.
}

void CMFCTestDlg::OnDeltaposThickSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

  //현재 Edit Control의 값을 가져옴
  CString strThickness;
  int nThickness;
  strThickness = "";
	nThickness = 0;
  GetDlgItemText(IDC_EDIT1, strThickness);
  nThickness = _ttoi(strThickness);

  if (strThickness.IsEmpty()) 
  {
    nThickness = 1;  //Default값 설정 부분
  }
  nThickness += pNMUpDown->iDelta;  
  if (nThickness < 1)
    nThickness = 1;
  if (nThickness > 100)
    nThickness = 100;
  SetDlgItemInt(IDC_EDIT1, nThickness);
  m_nThickness = nThickness;
  Invalidate();//새로고침

  *pResult = 0;
}


void CMFCTestDlg::OnEnChangeThick()
{
  CString strThickness;
  int nThickness;
	strThickness = "";
	nThickness = 0;

  GetDlgItemText(IDC_EDIT1, strThickness);
  nThickness = _ttoi(strThickness);

	//두께 값의 최소/최대값 Limit
  if (nThickness < 1)
    nThickness = 1;
  if (nThickness > 100)
    nThickness = 100;

	//변수에 두께 값 Update
  m_nThickness = nThickness;

  //스핀 컨트롤 값
  CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if (pSpin) 
  {
    pSpin->SetPos(nThickness);
  }
  SetDlgItemInt(IDC_EDIT1, nThickness);
  Invalidate();//새로고침
}

void CMFCTestDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
  *pResult = 0;
}


void CMFCTestDlg::OnBnClickedDrawResetBtn()
{
  //데이터 초기화
  m_Points.clear();
  m_Circles.clear();

  Invalidate();//새로고침

  //화면 새로고침 이후 변수 정리
  m_nThickness = 1; //기본 두께값 초기화
  SetDlgItemInt(IDC_EDIT1, m_nThickness);
  CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if (pSpin)
    pSpin->SetPos(m_nThickness);
}

void CMFCTestDlg::OnBnClickedRandom()
{
  CRect C_RectPicture;
  double dX = 0.0;
  double dY = 0.0;
  double dAngle = 0.0;
  double dRadiaus = 0.0;
  double dPointX = 0.0;
  double dPointY = 0.0;
  m_Points.clear(); 
  
  //변수 선언 및 초기화

  GetDlgItem(IDC_DRAWSTATIC)->GetClientRect(&C_RectPicture);

  //랜덤 3개 포인트 함수 생성
  for (auto& aCircle : m_Circles)
  {
    //원 Center점 
    dX = rand() % (C_RectPicture.Width() - 2 * aCircle.nDefultRadius) + aCircle.nDefultRadius;
    dY = rand() % (C_RectPicture.Height() - 2 * aCircle.nDefultRadius) + aCircle.nDefultRadius;

    aCircle.cCenter = CPoint(dX, dY);

    //원 외곽에 3개
    for (int i = 0; i < 3; ++i)
    {
      //각도를 랜덤하게 설정 (0~360도 범위에서 랜덤)
      dAngle = rand() % 360;

      // 각도에 맞는 x, y 좌표 계산 (원 외곽에 점을 찍기 위해 원의 반지름을 사용)
      dRadiaus = dAngle * (3.14159265 / 180.0); // 각도를 라디안으로 변환
      dPointX = aCircle.cCenter.x + aCircle.nDefultRadius * cos(dRadiaus);
      dPointY = aCircle.cCenter.y + aCircle.nDefultRadius * sin(dRadiaus);

      // 점을 m_Points 벡터에 추가
      m_Points.push_back(CPoint(static_cast<int>(dPointX), static_cast<int>(dPointY)));
    }
  }

  Invalidate(); // 새로고침
}

//완성도도 떨어지고, 요청사항을 충족하지 못한 부분도 있고,
//버그도 많은 부분에 대해서 스스로도 인지 하고 있습니다.
//영상을 시청하고 매뉴얼을 참고하여 작성했어야 했는데, 그러지 못한 점에 대해서 죄송합니다.
//본래 고집이 많은 성격이라 이런 저런 방법을 시도하여 테스트를 하는 타입니다.
//이번 과제를 통해 MFC / C++ 프로그래밍에 대해 많은 것을 배울 수 있었습니다.
//합격 불합격 여부를 떠나서 좋은 경험을 하게 해주셔서 감사합니다.
