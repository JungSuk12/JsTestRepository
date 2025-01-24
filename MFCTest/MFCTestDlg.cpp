#include "pch.h"
#include "framework.h"
#include "MFCTest.h"
#include "MFCTestDlg.h"
#include "afxdialogex.h"
#include "afx.h"

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
  ON_WM_CTLCOLOR()
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
  SetDlgItemInt(IDC_EDIT1, 1); //Edit Control 값에 기본값 초기화
  m_nThickness = 1;

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
  //CButton pDotButtonImage;
  //CBitmap bmp;
  //bmp.LoadBitmap(IDB_BNRANDOMUP);
  //CButton* pButton = (CButton*)GetDlgItem(IDB_BNRANDOMUP);
  //pButton->SetBitmap(bmp);
  //m_BitmapButton.AutoLoad(IDB_BNRANDOMUP, this);

  //스핀 컨트롤 초기화
  CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin)
  {
    pSpin->SetRange(1, 100);//최소값1, 최대값100 
    pSpin->SetPos(10);//초기값10
  }
  m_BackGroundColor.CreateSolidBrush(RGB(100, 110, 180));
  //m_bmpOK.LoadBitmap(IDB_OK1);
 //m_btnOK.SetBitmap(m_bmpOK);
  return TRUE; 
}

HBRUSH CMFCTestDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

  //다이얼로그 배경색 설정
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

std::vector<Circle> m_Circles;//구조체
bool m_bCircleCreated = false;//추가적인 원 생성 방지 플래그

void CMFCTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
  CPoint pCenter;
  int nRadius = 0;

  if(m_bCircleCreated)
    return;
  //원이 이미 생성된 경우 리턴

  m_Points.push_back(point);
  if(m_Points.size() == 3 && !m_bCircleCreated)
  {
    //원 계산 영역
    std::vector<CPoint> ThreePoints =
    {//마우스 도트 포인트 3개
        m_Points[m_Points.size() - 3],
        m_Points[m_Points.size() - 2],
        m_Points[m_Points.size() - 1]
    };
    //CalculateCircle 함수로 3개의 포인트로 원 생성
    nRadius = CalculateCircle(ThreePoints, pCenter);
    if(nRadius > 0)
    {
      Circle newCircle = {pCenter, nRadius};
      m_Circles.push_back(newCircle);//메모리 재할당
      m_bCircleCreated = true; //원 생성 완료
    }
  }
  OnPaint(); //화면 갱신
  CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCTestDlg::OnPaint()
{
  CDialogEx::OnPaint();
  CWnd* pStaticCtrl = GetDlgItem(IDC_DRAWSTATIC);
  CClientDC CDC(pStaticCtrl);
  CRect CRect;
  CRgn CRegion;
  CBrush CDotBrush;
  CPoint pCenterPoint;
  int nRadius = 0;
  //변수 선언

  if(pStaticCtrl != nullptr)
  {
    pStaticCtrl->GetClientRect(&CRect);
    //원을 그려주는 영역 배경만 흰색으로 초기화
    CDC.FillSolidRect(CRect, RGB(255, 255, 255));
    //25.01.24 영역 벗어 나는 부분 수정
    CRegion.CreateRectRgn(CRect.left,
                          CRect.top,
                          CRect.right,
                          CRect.bottom);

    CDC.SelectClipRgn(&CRegion);

    //원을 생성할 기반 Rect
    for(const auto& point:m_Points)
    {
      if(CRect.PtInRect(point))
      {
        CBrush CDotBrush(RGB(0, 0, 0));//Dot 색상
        CBrush* pDotBrush = CDC.SelectObject(&CDotBrush);

        CDC.Ellipse(point.x - 5,
                    point.y - 5,
                    point.x + 5,
                    point.y + 5);

        CDC.SelectObject(pDotBrush);
      }
    }

    //점을 기반으로 큰 원 생성
    if(m_Points.size() >= 3)
    {
      nRadius = CalculateCircle(m_Points, pCenterPoint);
      if(nRadius > 0 &&
          pCenterPoint.x - nRadius >= CRect.left &&
          pCenterPoint.x + nRadius <= CRect.right &&
          pCenterPoint.y - nRadius >= CRect.top &&
          pCenterPoint.y + nRadius <= CRect.bottom)
      {
        CPen Cpen(PS_SOLID, m_nThickness, RGB(0, 0, 255));
        CPen* ColdPen = CDC.SelectObject(&Cpen);

        CDC.Ellipse(pCenterPoint.x - nRadius,
                    pCenterPoint.y - nRadius,
                    pCenterPoint.x + nRadius,
                    pCenterPoint.y + nRadius);

        CDC.SelectObject(ColdPen);
      }
    }

    CDC.SelectClipRgn(nullptr);
  }
}


//Owner Draw에서 원을 그리는 함수
void CMFCTestDlg::DrawCircleFromPoints(CDC& dc)
{
  CRgn Rgn;
  CPoint p_Center;
  CRect C_RectPicture;
  int nRadius = 0;
  int nDotCount = 0;

  //Picture Control
  GetDlgItem(IDC_DRAWSTATIC)->GetClientRect(&C_RectPicture);
  GetDlgItem(IDC_DRAWSTATIC)->ClientToScreen(&C_RectPicture);
  ScreenToClient(&C_RectPicture);
  Rgn.CreateRectRgn(C_RectPicture.left, 
                    C_RectPicture.top,
                    C_RectPicture.right,
                    C_RectPicture.bottom);

  dc.SelectClipRgn(&Rgn);
  if(m_Points.size() == 3)
  {
    nRadius = CalculateCircle(m_Points, p_Center);
    if(nRadius > 0)
    {
      CPen pen(PS_SOLID,
               m_nThickness,
               RGB(0, 0, 0));

      CPen* pOldPen = dc.SelectObject(&pen);

      dc.Ellipse(p_Center.x - nRadius,
                 p_Center.y - nRadius,
                 p_Center.x + nRadius,
                 p_Center.y + nRadius);

      dc.SelectObject(pOldPen);
    }
  }
  //포인터 해제
  dc.SelectClipRgn(nullptr);
}
void CMFCTestDlg::DrawCircleFromDrag(CDC& dc)
{
  
}
//3개의 점으로 원을 계산하는 함수
int CMFCTestDlg::CalculateCircle(const std::vector<CPoint>& dCirclepoint, CPoint& dCenterPoint)
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

  dX1 = dCirclepoint[0].x;
  dY1 = dCirclepoint[0].y;
  dX2 = dCirclepoint[1].x;
  dY2 = dCirclepoint[1].y;
  dX3 = dCirclepoint[2].x;
  dY3 = dCirclepoint[2].y;

  if(dCirclepoint.size() != 3)
    return 0;

  dTriAngle = dX1 * (dY2 - dY3) + 
              dX2 * (dY3 - dY1) + 
              dX3 * (dY1 - dY2);

  if(fabs(dTriAngle) < 1e-8)
    return 0;

  dX = ((dX1 * dX1 + dY1 * dY1) * (dY2 - dY3) +
        (dX2 * dX2 + dY2 * dY2) * (dY3 - dY1) +
        (dX3 * dX3 + dY3 * dY3) * (dY1 - dY2)) /
        (2 * dTriAngle);
  dY = ((dX1 * dX1 + dY1 * dY1) * (dX3 - dX2) +
        (dX2 * dX2 + dY2 * dY2) * (dX1 - dX3) +
        (dX3 * dX3 + dY3 * dY3) * (dX2 - dX1)) /
        (2 * dTriAngle);

  dCenterPoint = CPoint(static_cast<int>(dX), static_cast<int>(dY));

  return static_cast<int>(sqrt((dX - dX1) * (dX - dX1) +
                               (dY - dY1) * (dY - dY1)));
}
void CMFCTestDlg::OnStnClickedIdsPictypeBitmap()
{
  //여기에 필요한 추가적인 처리를 넣을 수 있습니다.
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

  if(strThickness.IsEmpty()) 
    nThickness = 1;
  nThickness += pNMUpDown->iDelta;
  if(nThickness < 1)
    nThickness = 1;
  if(nThickness > 100)
    nThickness = 100;
  SetDlgItemInt(IDC_EDIT1, nThickness);
  m_nThickness = nThickness;
  OnPaint();//새로고침

  *pResult = 0;
}


void CMFCTestDlg::OnEnChangeThick()
{
  static bool isUpdating = false;
  if(isUpdating) 
    return;
  isUpdating = true;
  CString strThickness;
  int nThickness;
  strThickness = "";
  nThickness = 0;

  CSpinButtonCtrl* pSpin;
  GetDlgItemText(IDC_EDIT1, strThickness);

  if(!strThickness.IsEmpty())
    nThickness = _ttoi(strThickness);

  //두께 값의 최소/최대값 제한
  if(nThickness < 1)
    nThickness = 1;
  if(nThickness > 100)
    nThickness = 100;

  //변수에 두께 값 업데이트
  m_nThickness = nThickness;

  //스핀 컨트롤 값 업데이트
  pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin && ::IsWindow(pSpin->m_hWnd))
    pSpin->SetPos(nThickness);

  //에디트 컨트롤 값 업데이트
  SetDlgItemInt(IDC_EDIT1, nThickness);
  isUpdating = false;
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
  static bool isUpdating = false;
  if(isUpdating) 
    return;
  isUpdating = true;
  pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin && ::IsWindow(pSpin->m_hWnd)) 
    pSpin->SetPos(nThickness);

  //에디트 컨트롤 값 업데이트
  SetDlgItemInt(IDC_EDIT1, nThickness);

  //새로고침
  OnPaint();
  isUpdating = false;
}

void CMFCTestDlg::OnBnClickedDrawResetBtn()
{
  //초기화
  m_Points.clear();
  m_Circles.clear();
  m_bCircleCreated = false;//원 초기화

  OnPaint();//새로고침

  //화면 새로고침 이후 변수 정리
  m_nThickness = 1;
  SetDlgItemInt(IDC_EDIT1, m_nThickness);
  CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);
  if(pSpin)
    pSpin->SetPos(m_nThickness);
}

void CMFCTestDlg::OnBnClickedRandom()
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

  nMaxRadius = min(C_RectPicture.Width(),
                   C_RectPicture.Height()) / 4; 
  //Random 원의 최소 크기 지정
  nRadius = rand() % nMaxRadius + 10; 
  //Random 원의 최대 크기 지정
  nCenterX = rand() % (C_RectPicture.Width() - 2 * nRadius) + nRadius;
  //Random 원의 Width 값. 
  nCenterY = rand() % (C_RectPicture.Height() - 2 * nRadius) + nRadius;
  //Random 원의 Height 값. 

  CPoint pCenter(nCenterX, //Random 원의 Center점 지정
                 nCenterY);
  Circle RandomCircle = {pCenter, //Random 원의 Center점 지정
                         nRadius};
  //크기를 고정시키고 싶으면 nRadius 값을 Parameter 값에서 가져오면 된다.

  m_Circles.push_back(RandomCircle);
  //원의 외곽에 3개의 점 생성

  for(int i = 0; i < 3; i++)
  {
    dAngle = rand() % 360 * (3.14 / 180.0);
    nPointX = static_cast<int>(nCenterX + nRadius * cos(dAngle));
    nPointY = static_cast<int>(nCenterY + nRadius * sin(dAngle));
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
      m_Points.push_back(CPoint(nPointX, nPointY));
    else
      --i;
  }
  OnPaint();
}
