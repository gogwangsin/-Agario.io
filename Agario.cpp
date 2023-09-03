
#if 1

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include"resource.h"

#define UP WAY[0]
#define RIGHT WAY[1]
#define DOWN WAY[2]
#define LEFT WAY[3]


typedef struct ONE {
	POINT pivot;
	COLORREF color;
	BOOL WAY[4]; // 마우스위치에 따라 y,x 4각면 비교해서 방향 바꾸게 
	double radius;
	//	int sizex, sizey;
	double PX, PY; // 더해지는값
	int state;// 분열가능 사이즈?
}ONE;


typedef struct FOOD {

	POINT pivot;
	COLORREF color;
	double radius;
	POINT Goal;
	double PX, PY;
	int timer;
	POINT start;

}FOOD;


typedef struct TRAP {

	POINT pivot;
	double radius;
	double PX, PY;
	BOOL live; // 어떤 지점을 tan구해서 그 지점으로 이동하는데 그 지점이 계쏙 바뀌는겨 
	int randx, randy;

	
}TRAP;

ONE one;
ONE virus[20];
FOOD food[50]; // 이게 최대 
TRAP trap[10];

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"2023 Window Programming Agario.io 고광신";	
RECT client;
HPEN hpen, oldpen;
HBRUSH hbrush, oldbrush;
PAINTSTRUCT ps;
HDC	hdc;
HDC mdc;
HBITMAP hBitmap;
BOOL protect;
BOOL over;
BOOL start;


int FoodCount;
int TrapCount;
int Viruscount;

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

void crash(ONE* one, ONE virus[20], TRAP trap[10]);
void createVirus(HWND hwnd, ONE virus[20], int* countVirus);
void showVirus(HDC mdc, ONE virus[20]);
void getVirus(ONE* virus);
void beforeTrap(HDC mdc, TRAP* trap,int trapTimer);
void createTrap(HWND hwnd, TRAP trap[10]);
void showTrap(HDC mdc, TRAP trap[10]);
void getTrap(int randx, int randy, TRAP* trap);
void eatFood(ONE* one, FOOD food[50], ONE virus[20]);
BOOL isCollision(int x1, int y1, int x2, int y2, double radius1, double radius2);
void createFood(HWND hwnd, FOOD food[50]);
void showFood(HDC mdc, FOOD food[50]);
void createOne(HWND hwnd, ONE* one);
void showOne(HDC mdc, ONE* one);
void oneWay(int* mx, int* my, ONE* one);
void get(int mx, int my, ONE* one);
void setWay(int X, int Y, ONE* one);
BOOL InCircle(int x1, int y1, int x2, int y2);
double LengthPts(int x1, int y1, int x2, int y2);
int getRadius(ONE* one);
void OnPaint(HDC mdc, float x, float y, float intervalX, float intervalY);


int  WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_  LPSTR lpszCmdParam, _In_  int nCmdShow)
{
	srand((unsigned)time(NULL));

	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_QUESTION);
	WndClass.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	//--- 크기 변경 가능 (기존 (1024, 768)) 150,10,1200,900 / 350 50 800 800
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 350, 50, 800, 800, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}



void crash(ONE* one,ONE virus[20],TRAP trap[10])
{

	for (int i = 0; i < TrapCount; i++) // 트랩 충돌 - 트랩이 더 크면 '모드' 활성화로 '무적'
	{
		if (trap[i].live == TRUE) // 트랩 생겼을때 
		{
			if (isCollision(one->pivot.x, one->pivot.y, trap[i].pivot.x, trap[i].pivot.y, one->radius, trap[i].radius)) // 충돌했을때 
			{
				if(trap[i].radius>=one->radius && (trap[i].pivot.x - trap[i].radius <= one->pivot.x - one->radius && trap[i].pivot.x + trap[i].radius >= one->pivot.x + one->radius
					&& trap[i].pivot.y - trap[i].radius <= one->pivot.y - one->radius && trap[i].pivot.y + trap[i].radius >= one->pivot.y + one->radius)) // 트랩이 더 크고 안에있으면
				{
					protect = TRUE;
					break;
				}
				else if (trap[i].radius < one->radius)
				{
					one->radius /= 2.5; // 충돌 
					if (one->PX + 0.4 <= 3.0)
						one->PX += 0.4;
					else one->PX += 0.2;

					if (one->PY + 0.4 <= 3.0)
						one->PY += 0.4;
					else one->PY += 0.2;

					for (int k = i; k < TrapCount; k++)
					{
						trap[k] = trap[k + 1];
					}
					TrapCount--;
					protect = FALSE;
				}
				
			}
			else
			{
				protect = FALSE;
			}
		}
	}

	for (int i = 0; i < Viruscount; i++) // 바이러스 충돌 크면 뒤지고, 작으면 커짐 
	{
		if (isCollision(one->pivot.x, one->pivot.y, virus[i].pivot.x, virus[i].pivot.y, one->radius, virus[i].radius))
		{
			if (protect==FALSE && (one->radius < virus[i].radius))
			{
				if (over != TRUE)
				{
					virus[i].radius += one->radius / 2;
				}
				over = TRUE;
				break;
			}
			else if (one->radius > virus[i].radius)
			{
				one->radius += virus[i].radius / 4;

				if (one->PX - 0.2 >= 2.0)
					one->PX -= 0.2;
				else one->PX -= 0.1;

				if (one->PY - 0.2 >= 2.0)
					one->PY -= 0.2;
				else one->PY -= 0.1;

				for (int k = i; k < Viruscount; k++)
				{
					virus[k] = virus[k + 1];
				}
				Viruscount--;
			}
		}
	}

	for (int i = 0; i < TrapCount; i++)
	{
		if (trap[i].live == TRUE) // 트랩 생겼을때 
		{
			for (int k = 0; k < Viruscount; k++)
			{
				if (isCollision(virus[k].pivot.x, virus[k].pivot.y, trap[i].pivot.x, trap[i].pivot.y, virus[k].radius, trap[i].radius)) // 충돌했을때 
				{
					if (trap[i].radius < virus[k].radius)
					{
						virus[k].radius /= 3.0;

						if (virus[i].PX + 0.2 <= 3.0)
							virus[i].PX += 0.2;
						else virus[i].PX += 0.1;

						if (virus[i].PY + 0.2 <= 3.0)
							virus[i].PY += 0.2;
						else virus[i].PY += 0.1;


						for (int j = i; j < TrapCount; j++)
						{
							trap[j] = trap[j + 1];
						}
						TrapCount--;
					}
				}
			}
		}
	}
}

void eatFood(ONE* one, FOOD food[50],ONE virus[20])
{
	for (int i = 0; i < FoodCount; i++)
	{
		if (isCollision(one->pivot.x, one->pivot.y, food[i].pivot.x, food[i].pivot.y, one->radius, food[i].radius))
		{
			one->radius++;
			if(one->PX - 0.025>= 2.5) one->PX -= 0.025;
			if (one->PY - 0.025>= 2.5) one->PY -= 0.025;
			for (int k = i; k+1 < FoodCount; k++)
			{
				food[k] = food[k+1];
			}
			FoodCount--;
			break;
		}
	}

	for (int i = 0; i < FoodCount; i++)
	{
		for (int k = 0; k < Viruscount; k++)
		{
			if (isCollision(virus[k].pivot.x, virus[k].pivot.y, food[i].pivot.x, food[i].pivot.y, virus[k].radius, food[i].radius))
			{
				virus[k].radius++; // 3
				if (virus[k].PX - 0.05 >= 1.5)
					virus[k].PX -= 0.05;
				if (virus[k].PY - 0.05 >= 1.5)
					virus[k].PY -= 0.05;
				for (int j = i; j + 1 < FoodCount; j++)
				{
					food[j] = food[j + 1];
				}
				FoodCount--;
				i--;
				break;
			}
		}
	}
}
void get(int mx, int my, ONE* one)
{
	double xdiff = ((double)mx - (double)one->pivot.x);
	double ydiff = ((double)my - (double)one->pivot.y);
	double degree = atan2(ydiff, xdiff);

	if (!InCircle(mx, my, one->pivot.x, one->pivot.y))
	{
		if ((one->pivot.x + cos(degree) * (one->PX)) - one->radius >= 0 && (one->pivot.x + cos(degree) * (one->PX)) + one->radius <= client.right)
			one->pivot.x += cos(degree) * (one->PX);

		if ((one->pivot.y + sin(degree) * (one->PY)) - one->radius >= 0 && (one->pivot.y + sin(degree) * (one->PY)) + one->radius <= client.bottom)
			one->pivot.y += sin(degree) * (one->PY);

		if (one->pivot.x - one->radius < 0) one->pivot.x++;
		if (one->pivot.x + one->radius > client.right) one->pivot.x--;
		if (one->pivot.y - one->radius < 0) one->pivot.y++;
		if (one->pivot.y + one->radius > client.bottom) one->pivot.y--;
	}

}
void getTrap(int randx, int randy, TRAP* trap)
{

	double xdiff = ((double)randx - (double)trap->pivot.x);
	double ydiff = ((double)randy - (double)trap->pivot.y);
	double degree = atan2(ydiff, xdiff);

	if (!InCircle(randx, randy, trap->pivot.x, trap->pivot.y))
	{
		if ((trap->pivot.x+ cos(degree) * (trap->PX)) - trap->radius >= 0 && (trap->pivot.x + cos(degree) * (trap->PX)) + trap->radius <= client.right)
			trap->pivot.x += cos(degree) * (trap->PX);

		if ((trap->pivot.y+ sin(degree) * (trap->PY)) - trap->radius >= 0 && (trap->pivot.y+sin(degree) * (trap->PY)) + trap->radius <= client.bottom)
			trap->pivot.y += sin(degree) * (trap->PY);
	}
}
void getVirus(ONE* virus)
{
	POINT distance; // 최소값 저장 

	double rad[50] = { 0 };

	for (int i = 0; i < FoodCount; i++)
	{
		rad[i] = LengthPts(food[i].pivot.x, food[i].pivot.y, virus->pivot.x, virus->pivot.y);
	}

	double temp[50] = { 0 };
	double tem = 0;

	for (int i = 0; i < FoodCount; i++)
	{
		temp[i] = rad[i];
	}
	for (int i=0; i < FoodCount; i++)
	{
		for (int k = i+1; k < FoodCount; k++)
		{
			if (temp[i] > temp[k]) // 앞에게 뒤에거 보다 크면 정렬
			{
				tem = temp[i];
				temp[i] = temp[k];
				temp[k] = tem;
			}
		}
	}

	 tem = temp[0];

	int k = 0;
	for (k = 0; k < FoodCount; k++)
	{
		if (rad[k] == tem)break;
	}

	tem = LengthPts(virus->pivot.x, virus->pivot.y, one.pivot.x, one.pivot.y);

	
	if (protect==FALSE && (FoodCount==0 || (virus->radius-5 > one.radius))) 
	{
		if (FoodCount == 0 || tem < virus->radius + 500) // 반지름 + 원과 거리가 >= 음식최소거리보다 클때 
		{
			distance.x = one.pivot.x;
			distance.y = one.pivot.y;
		}
		else
		{
			distance.x = food[k].pivot.x;
			distance.y = food[k].pivot.y;
		}

	}
	else
	{
		distance.x = food[k].pivot.x;
		distance.y = food[k].pivot.y;
	}
	

	
	double xdiff = ((double)distance.x - (double)virus->pivot.x);
	double ydiff = ((double)distance.y - (double)virus->pivot.y);
	double degree = atan2(ydiff, xdiff);
	

	if ((virus->pivot.x + cos(degree) * (virus->PX)) - virus->radius >= 0 && (virus->pivot.x + cos(degree) * (virus->PX)) + virus->radius <= client.right)
		virus->pivot.x += cos(degree) * (virus->PX);

	if ((virus->pivot.y + sin(degree) * (virus->PY)) - virus->radius >= 0 && (virus->pivot.y + sin(degree) * (virus->PY)) + virus->radius <= client.bottom)
		virus->pivot.y += sin(degree) * (virus->PY);

	if (virus->pivot.x - virus->radius < 0) virus->pivot.x += 5;
	if (virus->pivot.x + virus->radius > client.right) virus->pivot.x -= 5;
	if (virus->pivot.y - virus->radius < 0) virus->pivot.y += 5;
	if (virus->pivot.y + virus->radius > client.bottom) virus->pivot.y -= 5;

}

void createTrap(HWND hwnd, TRAP trap[10])
{
	GetClientRect(hwnd, &client);
	for (int i = 0; i < TrapCount; i++)
	{
		trap[i].radius = 30;
		trap[i].PX = 3.5;
		trap[i].PY = 3.5;
		trap[i].pivot.x = rand() % client.right;
		if (trap[i].pivot.x < trap[i].radius) trap[i].pivot.x = trap[i].radius;
		if (trap[i].pivot.x + trap[i].radius > client.right) trap[i].pivot.x -= trap[i].radius;
		trap[i].pivot.y = rand() % client.bottom;
		if (trap[i].pivot.y < trap[i].radius) trap[i].pivot.y = trap[i].radius;
		if (trap[i].pivot.y + trap[i].radius > client.bottom) trap[i].pivot.y -= trap[i].radius;
		trap[i].live = TRUE;
		trap[i].randx = rand() % client.right;
		trap[i].randy = rand() % client.bottom;
	}

}
void createOne(HWND hwnd, ONE* one)
{
	GetClientRect(hwnd, &client);

	one->radius = 10; // 반지름 15 시작 원 둘레 30파이 
	one->pivot.x = rand() % client.right;
	if (one->pivot.x < one->radius) one->pivot.x = one->radius;
	if (one->pivot.x + one->radius > client.right) one->pivot.x -= one->radius;
	one->pivot.y = rand() % client.bottom;
	if (one->pivot.y < one->radius) one->pivot.y = one->radius;
	if (one->pivot.y + one->radius > client.bottom) one->pivot.y -= one->radius;
	one->color = RGB(255, 178, 245);
	one->PX = 3.0;
	one->PY = 3.0;
}
void createFood(HWND hwnd,FOOD food[50])
{
	GetClientRect(hwnd, &client);

	for (int i = 0; i < FoodCount; i++)
	{
		food[i].radius = 7; // 반지름 15 시작 원 둘레 30파이 
		food[i].pivot.x = rand() % client.right;
		if (food[i].pivot.x < food[i].radius) food[i].pivot.x = food[i].radius;
		if (food[i].pivot.x + food[i].radius > client.right) food[i].pivot.x -= food[i].radius;
		food[i].pivot.y = rand() % client.bottom;
		if (food[i].pivot.y < food[i].radius) food[i].pivot.y = food[i].radius;
		if (food[i].pivot.y + food[i].radius > client.bottom) food[i].pivot.y -= food[i].radius;
		food[i].color = RGB(rand()%255, rand() % 255, rand() % 255);
	}
}
void createVirus(HWND hwnd, ONE virus[20],int* countVirus)
{
	GetClientRect(hwnd, &client);

	virus[*countVirus].radius = 10; // 반지름 15 시작 원 둘레 30파이 
	virus[*countVirus].pivot.x = rand() % client.right;
	if (virus[*countVirus].pivot.x < virus[*countVirus].radius) virus[*countVirus].pivot.x = virus[*countVirus].radius;
	if (virus[*countVirus].pivot.x + virus[*countVirus].radius > client.right) virus[*countVirus].pivot.x -= virus[*countVirus].radius;
	virus[*countVirus].pivot.y = rand() % client.bottom;
	if (virus[*countVirus].pivot.y < virus[*countVirus].radius) virus[*countVirus].pivot.y = virus[*countVirus].radius;
	if (virus[*countVirus].pivot.y + virus[*countVirus].radius > client.bottom) virus[*countVirus].pivot.y -= virus[*countVirus].radius;
	virus[*countVirus].color = RGB(255, 0, 0);
	virus[*countVirus].PX = 3.0;
	virus[*countVirus].PY = 3.0;
	(*countVirus)++;
}
void getFood(FOOD* food)
{

	if (food->PX != 0 || food->PY != 0) // 도착전일때 
	{
		double xdiff = ((double)food->Goal.x - (double)food->start.x);
		double ydiff = ((double)food->Goal.y - (double)food->start.y);
		double degree = atan2(ydiff, xdiff);


		if ((food->pivot.x + cos(degree) * (food->PX)) - food->radius >= 0 && (food->pivot.x + cos(degree) * (food->PX)) + food->radius <= client.right)
		{
			if (food->timer < 80)
			{
				food->pivot.x += cos(degree) * (food->PX);
				food->timer++;
			}
			else
			{
				food->timer = 0;
				food->PX = 0;
				food->PY = 0;
			}
		}
		if ((food->pivot.y + sin(degree) * (food->PY)) - food->radius >= 0 && (food->pivot.y + sin(degree) * (food->PY)) + food->radius <= client.bottom)
		{
			if (food->timer < 80)
			{
				food->pivot.y += sin(degree) * (food->PY);
				food->timer++;
			}
			else
			{
				food->timer = 0;
				food->PX = 0;
				food->PY = 0;
			}			
		}
	}
	
}



void showVirus(HDC mdc, ONE virus[20])
{
	for (int i = 0; i < Viruscount; i++)
	{
		hbrush = CreateSolidBrush(virus[i].color);
		oldbrush = (HBRUSH)SelectObject(mdc, hbrush);
		hpen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		oldpen = (HPEN)SelectObject(mdc, hpen);
		Ellipse(mdc, virus[i].pivot.x - virus[i].radius, virus[i].pivot.y - virus[i].radius, virus[i].pivot.x + virus[i].radius, virus[i].pivot.y + virus[i].radius);
		SelectObject(mdc, oldbrush);
		SelectObject(mdc, oldpen);
		DeleteObject(hpen);
		DeleteObject(hbrush);
	}
}
void beforeTrap(HDC mdc, TRAP* trap,int trapTimer)
{
	HPEN hpen, oldpen;
	HBRUSH hbrush, oldbrush;
	
	hpen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	oldpen = (HPEN)SelectObject(mdc, hpen);
	hbrush = CreateSolidBrush(RGB(209, 178, 255));
	oldbrush = (HBRUSH)SelectObject(mdc, hbrush);

	Ellipse(mdc, trap->pivot.x - 60 + trapTimer, trap->pivot.y - 60 + trapTimer, trap->pivot.x + 60- trapTimer, trap->pivot.y + 60 - trapTimer);

	SelectObject(mdc, oldpen);
	SelectObject(mdc, oldbrush);
	DeleteObject(hpen);
	DeleteObject(hbrush);


} 
void showTrap(HDC mdc, TRAP trap[10])
{
	HPEN hpen, oldpen;
	HBRUSH hbrush, oldbrush;
	hpen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	oldpen = (HPEN)SelectObject(mdc, hpen);
	hbrush = CreateHatchBrush(HS_DIAGCROSS, RGB(0, 0, 255));
	oldbrush = (HBRUSH)SelectObject(mdc, hbrush);
	for (int i = 0; i < TrapCount; i++)
	{
		if (trap[i].live==TRUE)
		{
			Ellipse(mdc, trap[i].pivot.x - trap[i].radius, trap[i].pivot.y - trap[i].radius, trap[i].pivot.x + trap[i].radius, trap[i].pivot.y + trap[i].radius);
		}
	}
	SelectObject(mdc, oldpen);
	DeleteObject(hpen);
	SelectObject(mdc, hbrush);
	DeleteObject(hbrush);
}
BOOL isCollision(int x1,int y1,int x2,int y2, double radius1,double radius2)
{
	double length = LengthPts(x1, y1, x2, y2);

	if (length <= radius1 + radius2)
	{
		return TRUE;
	}
	else return FALSE;
}
void showFood(HDC mdc, FOOD food[50])
{
	HPEN hpen, oldpen;
	HBRUSH hbrush, oldbrush;
	hpen = CreatePen(PS_SOLID, 3,RGB(0,0,0));
	oldpen = (HPEN)SelectObject(mdc, hpen);
	for (int i = 0; i < FoodCount; i++)
	{
		hbrush = CreateSolidBrush(food[i].color);
		oldbrush = (HBRUSH)SelectObject(mdc, hbrush);
		Ellipse(mdc, food[i].pivot.x - food[i].radius, food[i].pivot.y - food[i].radius, food[i].pivot.x + food[i].radius, food[i].pivot.y + food[i].radius);
		SelectObject(mdc, hbrush);
		DeleteObject(hbrush);
	}
	SelectObject(mdc, oldpen);
	DeleteObject(hpen);
}
void showOne(HDC mdc, ONE* one)
{
	hbrush = CreateSolidBrush(one->color);
	oldbrush = (HBRUSH)SelectObject(mdc, hbrush);
	hpen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	oldpen = (HPEN)SelectObject(mdc, hpen);
	if (over == FALSE && start==TRUE)
		Ellipse(mdc, one->pivot.x - one->radius, one->pivot.y - one->radius, one->pivot.x + one->radius, one->pivot.y + one->radius);
	SelectObject(mdc, oldbrush);
	SelectObject(mdc, oldpen);
	DeleteObject(hpen);
	DeleteObject(hbrush);

}
void oneWay(int* mx, int* my, ONE* one) 
{
	if (!InCircle(*mx, *my, one->pivot.x, one->pivot.y)) 
	{
		setWay(*mx, *my, one); 
	}
	else 
	{
		one->LEFT = FALSE;
		one->UP = FALSE;
		one->DOWN = FALSE;
		one->RIGHT = FALSE;
	}
} 
void setWay(int X, int Y, ONE* one) // 방향 설정해주는애 . virus도 해야함 
{
	double xdiff = ((double)X - (double)one->pivot.x);
	double ydiff = ((double)Y - (double)one->pivot.y);
	
	
	if (xdiff == 0 && ydiff != 0)
	{
		if (ydiff > 0) // 아래로 
		{ 
			one->DOWN = TRUE;
			one->UP = FALSE;
			one->LEFT = FALSE;
			one->RIGHT = FALSE;
		}
		else if (ydiff < 0) // 위로 
		{
			one->UP = TRUE;
			one->DOWN = FALSE;
			one->LEFT = FALSE;
			one->RIGHT = FALSE;
		}
	}
	else if (ydiff == 0 && xdiff !=0)
	{
		if (xdiff > 0) 
		{
			one->RIGHT = TRUE; // 우측 
			one->LEFT = FALSE;
			one->DOWN = FALSE;
			one->UP = FALSE;
		}
		else if (xdiff < 0)
		{
			one->LEFT = TRUE; // 좌측 
			one->DOWN = FALSE;
			one->UP = FALSE;
			one->RIGHT = FALSE;
		}
	}
	else if (xdiff > 0 && ydiff < 0) // 1
	{
		one->RIGHT = TRUE;
		one->DOWN = FALSE;
		one->LEFT = FALSE;
		one->UP = TRUE;
	}
	else if (xdiff > 0 && ydiff > 0) // 4
	{
		one->RIGHT = TRUE;
		one->DOWN = TRUE;
		one->UP = FALSE;
		one->LEFT = FALSE;
	}
	else if (xdiff < 0 && ydiff < 0) // 2
	{
		one->LEFT = TRUE;
		one->UP = TRUE;
		one->DOWN = FALSE;
		one->RIGHT = FALSE;
	}
	else if (xdiff < 0 && ydiff > 0) // 3 
	{
		one->LEFT = TRUE;
		one->UP = FALSE;
		one->DOWN = TRUE;
		one->RIGHT = FALSE;
	}

}
BOOL InCircle(int x1, int y1, int x2, int y2)
{
	double isRadius = (double)getRadius(&one);

	if (LengthPts(x1, y1, x2, y2) < isRadius/2)
	{
		return TRUE;
	}
	else return FALSE;
}
double LengthPts(int x1, int y1, int x2, int y2)
{
	double a = sqrt(((double)x2 - (double)x1) * ((double)x2 - (double)x1) + ((double)y2 - (double)y1) * ((double)y2 - (double)y1));
	return a;
}
int getRadius(ONE* one)
{
	return one->radius;
} 
void OnPaint(HDC mdc, float x, float y, float intervalX, float intervalY)
{
	hpen = CreatePen(PS_SOLID, 2, RGB(134, 229, 127));
	oldpen = (HPEN)SelectObject(mdc, hpen);
	for (int i = 0; i < x; i++)
	{
		MoveToEx(mdc, i * intervalX, 0, NULL);
		LineTo(mdc, i * intervalX, (y - 1) * intervalY);
	}
	for (int i = 0; i < y; i++)
	{
		MoveToEx(mdc, 0, i * intervalY, NULL); // X주면 귀여워짐
		LineTo(mdc, (x - 1) * intervalX, i * intervalY);
	}
	SelectObject(mdc, oldpen);
	DeleteObject(hpen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static POINT mouse;
	static int mx, my;
	mouse.x = mx;
	mouse.y = my;

	static int BOARD = 10;
	float BOARDX = 11;
	float BOARDY = 11;
	float intervalX = (client.right - client.left) / BOARD;
	float intervalY = (client.bottom - client.top) / BOARD;
	
	static TCHAR printRadius[100] = TEXT("Radius : ");
	static TCHAR oneRadius[100];
	static TCHAR printTime[100] = TEXT("TIME : ");
	static TCHAR gameTime[100] = { 0 };
	static double playtime;
	static BOOL showTime =TRUE;
	static BOOL pause = FALSE;
	static BOOL ViewPort=FALSE;

	static RECT view;
	static int trapTimer;

	static double xdiff, ydiff, degree;


	if (trapTimer == 30  ) //2.5초
	{
		KillTimer(hwnd, 5);
		trap[TrapCount].live = TRUE;
		TrapCount++;
		trapTimer = 0;

	}

	switch (iMessage) {
	case WM_CREATE:
		GetClientRect(hwnd, &client);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		mdc = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, client.right, client.bottom);
		SelectObject(mdc, (HBITMAP)hBitmap);
		hbrush = CreateSolidBrush(RGB(206, 251, 201));
		oldbrush = (HBRUSH)SelectObject(mdc, hbrush);

		SetMapMode(mdc, MM_ANISOTROPIC);
		SetWindowExtEx(mdc, one.pivot.x, one.pivot.y, NULL);
		GetClientRect(hwnd, &client);
		SetViewportExtEx(mdc, one.pivot.x, one.pivot.y, NULL);

		Rectangle(mdc, 0, 0, client.right, client.bottom);
		SelectObject(mdc, oldbrush); DeleteObject(hbrush);
		OnPaint(mdc, BOARDX, BOARDY, intervalX,intervalY);
		SetBkColor(mdc, RGB(206, 251, 201));
		if (showTime)
		{
			_stprintf(oneRadius, TEXT("%.2lf"), one.radius);
			_stprintf(gameTime, TEXT("%4.2lf"), playtime);
			TextOut(mdc, client.right / 2, client.top + 40, printTime, _tcslen(printTime));
			TextOut(mdc, client.right / 2+ 45, client.top + 40, gameTime, _tcslen(gameTime));
			TextOut(mdc, client.right / 2, client.top + 20, printRadius, _tcslen(printRadius));
			TextOut(mdc, client.right / 2+60, client.top + 20, oneRadius, _tcslen(oneRadius));
		}
		if (start == TRUE && trap[TrapCount].live == FALSE && trapTimer!=0)
			beforeTrap(mdc, &trap[TrapCount],trapTimer);
		eatFood(&one, food,virus);
		showOne(mdc, &one);
		showVirus(mdc, virus);
		showFood(mdc, food);
		showTrap(mdc, trap);


		if (over == TRUE)
		{
			KillTimer(hwnd, 0);
			KillTimer(hwnd, 1);
			KillTimer(hwnd, 2);
			KillTimer(hwnd, 3);
			KillTimer(hwnd, 4);
			KillTimer(hwnd, 5);
			KillTimer(hwnd, 6);
			SetBkColor(mdc, RGB(206, 251, 201));
			TextOut(mdc, client.right/2-80, client.bottom / 2-200, TEXT("최종 플레이 타임 : "), _tcslen(TEXT("최종 플레이 타임 : ")));
			TextOut(mdc, client.right/2+50, client.bottom / 2-200, gameTime, _tcslen(gameTime));
			SetBkColor(mdc, RGB(255, 255, 255));
		}

		// mdc에 다 저장해놨음

		if (start == TRUE && ViewPort == TRUE)
		{
			view.left = one.pivot.x - 300 ;
			view.top = one.pivot.y - 300 ;
			view.right = 600 + 2 * one.radius;
			view.bottom = 600 + 2 * one.radius; // 너비 높이 
			if (view.left < 0) view.left = 0;
			if (view.top < 0)view.top = 0;
			if (view.left + 600 + 2 * one.radius > client.right) view.left = client.right - 600 - 2 * one.radius;
			if (view.top + 600 + 2 * one.radius > client.bottom) view.top = client.bottom - 600 - 2 * one.radius;

			StretchBlt(hdc, 0, 0, client.right, client.bottom, mdc, view.left, view.top,view.right,view.bottom, SRCCOPY);
		}
		else BitBlt(hdc, 0, 0, client.right, client.bottom, mdc, 0, 0, SRCCOPY);

		DeleteDC(mdc);
		DeleteObject(hBitmap);
		EndPaint(hwnd, &ps);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_MENU_START: 
			SetTimer(hwnd, 0, 10, NULL); // 플레이 타임 -> 나중에 게임 시작할때로 타이머 되게 바꿔야함
			SetTimer(hwnd, 1, 0, NULL); // 주인공 + 이동타이머 
			SetTimer(hwnd, 2, 200, NULL); // 먹이 10초 마다 생김 
			SetTimer(hwnd, 3, 5000, NULL); // 트랩 방향 변환 타임 
			SetTimer(hwnd, 4, rand() % 5000 + 3000, NULL); // 트랩생성시기 
	
			SetTimer(hwnd, 6, rand() % 3000 + 7000, NULL); // 바이러스 생성

			TrapCount = rand() % 5+1; // 임의의 개수 
			FoodCount = 30;
			Viruscount = 1;

			createOne(hwnd, &one);
			createFood(hwnd, food);
			createTrap(hwnd, trap);
			createVirus(hwnd, virus, &Viruscount);
			start = TRUE;
			break;

		case ID_MENU_PAUSE:
			if (pause == FALSE)
			{
				KillTimer(hwnd, 0);
				KillTimer(hwnd, 1);
				KillTimer(hwnd, 2);
				KillTimer(hwnd, 3);
				KillTimer(hwnd, 4);
				KillTimer(hwnd, 5);
				KillTimer(hwnd, 6);
				pause = TRUE;
			}
			else
			{
				SetTimer(hwnd, 0, 10, NULL); // 플레이 타임 -> 나중에 게임 시작할때로 타이머 되게 바꿔야함
				SetTimer(hwnd, 1, 0, NULL); // 주인공 + 이동타이머 
				SetTimer(hwnd, 2, 200, NULL); // 먹이 0.2초 마다 생김 
				SetTimer(hwnd, 3, 5000, NULL); // 트랩 방향 변환 타임 
				SetTimer(hwnd, 4, rand() % 5000 + 3000, NULL); // 트랩생성시기 
				if (trap[TrapCount].live == FALSE)
					SetTimer(hwnd, 5, 0, NULL); // 애니메이션 3초 
				SetTimer(hwnd, 6, rand() % 3000 + 7000, NULL); // 바이러스 생성
				pause = FALSE;
			}
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case ID_MENU_QUIT:
			KillTimer(hwnd, 0);
			KillTimer(hwnd, 1);
			KillTimer(hwnd, 2);
			KillTimer(hwnd, 3);
			KillTimer(hwnd, 4);
			KillTimer(hwnd, 5);
			KillTimer(hwnd, 6);
			over = TRUE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case ID_MENU_RESET:
			showTime = TRUE;
			start = FALSE;
			over = FALSE;
			ViewPort = FALSE;
			playtime = 0;

			KillTimer(hwnd, 0);
			KillTimer(hwnd, 1);
			KillTimer(hwnd, 2);
			KillTimer(hwnd, 3);
			KillTimer(hwnd, 4);
			KillTimer(hwnd, 5);
			KillTimer(hwnd, 6);

			one.pivot.x = -100;
			one.pivot.y = -100;
			one.radius = 0;

			trapTimer = 0;
			FoodCount = 0;
			Viruscount = 0;
			TrapCount = 0; // 임의의 개수 
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case ID_MOVE_ONE: 
			ViewPort = TRUE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case ID_MOVE_TWO:
			ViewPort = FALSE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case ID_PRINT_INFOR: 
			if (showTime == FALSE) showTime = TRUE; 
			else showTime = FALSE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		InvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_CHAR:

		if (wParam == 'n' || wParam == 'N') // 게임시작 
		{
			SetTimer(hwnd, 0, 10, NULL); // 플레이 타임 -> 나중에 게임 시작할때로 타이머 되게 바꿔야함
			SetTimer(hwnd, 1, 0, NULL); // 주인공 + 이동타이머 
			SetTimer(hwnd, 2, 200, NULL); // 먹이 10초 마다 생김 
			SetTimer(hwnd, 3, 5000, NULL); // 트랩 방향 변환 타임 
			SetTimer(hwnd, 4, rand() % 5000 + 3000, NULL); // 트랩생성시기 
		 
			SetTimer(hwnd, 6, rand() % 3000 + 7000, NULL); // 바이러스 생성

			TrapCount = rand() % 5+1; // 임의의 개수 
			FoodCount = 30;
			Viruscount = 1;

			createOne(hwnd, &one);
			createFood(hwnd, food);
			createTrap(hwnd, trap);
			createVirus(hwnd, virus, &Viruscount);
			start = TRUE;
		}
		else if (wParam == 's' || wParam == 'S') // 게임 멈춤 
		{
			if (pause == FALSE)
			{
				KillTimer(hwnd, 0);
				KillTimer(hwnd, 1);
				KillTimer(hwnd, 2);
				KillTimer(hwnd, 3);
				KillTimer(hwnd, 4);
				KillTimer(hwnd, 5);
				KillTimer(hwnd, 6);
				pause = TRUE;
			}
			else
			{
				SetTimer(hwnd, 0, 10, NULL); // 플레이 타임 -> 나중에 게임 시작할때로 타이머 되게 바꿔야함
				SetTimer(hwnd, 1, 0, NULL); // 주인공 + 이동타이머 
				SetTimer(hwnd, 2, 200, NULL); // 먹이 10초 마다 생김 
				SetTimer(hwnd, 3, 5000, NULL); // 트랩 방향 변환 타임 
				SetTimer(hwnd, 4, rand() % 5000 + 3000, NULL); // 트랩생성시기 
				if (trap[TrapCount].live == FALSE)
					SetTimer(hwnd, 5, 0, NULL);
				SetTimer(hwnd, 6, rand() % 3000 + 7000, NULL); // 바이러스 생성
				pause = FALSE;
			}
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		else if (wParam == 'q' || wParam == 'Q') // 게임 종료 -> 최종플레이 시작 출력 
		{
			KillTimer(hwnd, 0);
			KillTimer(hwnd, 1);
			KillTimer(hwnd, 2);
			KillTimer(hwnd, 3);
			KillTimer(hwnd, 4);
			KillTimer(hwnd, 5);
			KillTimer(hwnd, 6);
			over = TRUE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		else if (wParam == '1')
		{
			ViewPort = TRUE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		else if (wParam == '2')
		{
			ViewPort = FALSE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		else if (wParam == 'p' || wParam == 'P') // 크기,플레이 시간 출력 
		{
			if (showTime == FALSE) showTime = TRUE;
			else showTime = FALSE;
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		else if (wParam == 'r' || wParam == 'R') // 리셋 
		{
			showTime = TRUE;
			start = FALSE;
			over = FALSE;
			ViewPort = FALSE;
			playtime = 0;

			KillTimer(hwnd, 0);
			KillTimer(hwnd, 1);
			KillTimer(hwnd, 2);
			KillTimer(hwnd, 3);
			KillTimer(hwnd, 4);
			KillTimer(hwnd, 5);
			KillTimer(hwnd, 6);

			one.pivot.x = -100;
			one.pivot.y = -100;
			one.radius = 0;
			trapTimer = 0;

			FoodCount = 0;
			Viruscount = 0;
			TrapCount = 0; // 임의의 개수 
			
		}

		InvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_TIMER:

		switch (wParam)
		{
		case 0: 
			playtime += 0.01;
			break; // 플레이 타임 
		case 1:
			 get(mx, my, &one); // 움직이는거 
			 for (int i = 0; i < TrapCount; i++)
			 {
				 getTrap(trap[i].randx, trap[i].randy, &trap[i]);
			 }
			 for (int i = 0; i < Viruscount; i++)
			 {
				 getVirus(&virus[i]);
			 }
			 for (int i = 0; i < FoodCount; i++)
			 {
				 getFood(&food[i]);
			 }
			 crash(&one, virus, trap);
			 InvalidateRect(hwnd, NULL, FALSE);
			 break;
		case 2:
			if (FoodCount >= 50) break;
			food[FoodCount].radius = 7; // 반지름 15 시작 원 둘레 30파이 
			food[FoodCount].pivot.x = rand() % client.right;
			if (food[FoodCount].pivot.x < 7) food[FoodCount].pivot.x = 7;
			if (food[FoodCount].pivot.x + food[FoodCount].radius > client.right) food[FoodCount].pivot.x -= 7;
			food[FoodCount].pivot.y = rand() % client.bottom;
			if (food[FoodCount].pivot.y < 7) food[FoodCount].pivot.y = 7;
			if (food[FoodCount].pivot.y + food[FoodCount].radius > client.bottom) food[FoodCount].pivot.y -= 7;
			food[FoodCount].color = RGB(rand() % 255, rand() % 255, rand() % 255);
			FoodCount++;
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case 3:
			for (int i = 0; i < TrapCount; i++)
			{
				trap[i].randx = rand() % client.right;
				trap[i].randy = rand() % client.bottom;
			}
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case 4:
			if (TrapCount >= 10) break;
			trap[TrapCount].radius = 30;
			trap[TrapCount].PX = 3.5;
			trap[TrapCount].PY = 3.5;
			trap[TrapCount].pivot.x = rand() % client.right;
			if (trap[TrapCount].pivot.x < trap[TrapCount].radius) trap[TrapCount].pivot.x = trap[TrapCount].radius;
			if (trap[TrapCount].pivot.x + trap[TrapCount].radius > client.right) trap[TrapCount].pivot.x -= trap[TrapCount].radius;
			trap[TrapCount].pivot.y = rand() % client.bottom;
			if (trap[TrapCount].pivot.y < trap[TrapCount].radius) trap[TrapCount].pivot.y = trap[TrapCount].radius;
			if (trap[TrapCount].pivot.y + trap[TrapCount].radius > client.bottom) trap[TrapCount].pivot.y -= trap[TrapCount].radius;
			trap[TrapCount].live = FALSE;
			trap[TrapCount].randx = rand() % client.right;
			trap[TrapCount].randy = rand() % client.bottom;
			
			for (int i = 0; i+1 < FoodCount; i++)
			{
				food[i] = food[i + 1];
			}
			FoodCount--;

			if (trap[TrapCount].live == FALSE)
				SetTimer(hwnd, 5, 0, NULL);
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case 5: 
			trapTimer++;
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case 6:
			if (Viruscount >= 20) break;
			createVirus(hwnd, virus, &Viruscount);
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		}
		InvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_MOUSEMOVE:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		oneWay(&mx, &my, &one); 
		InvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_RBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (one.radius <= 15)
		{
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}

		one.radius -= 2;
		if (one.PX + 0.05 <= 3.5) one.PX += 0.05;
		if (one.PY + 0.05 <= 3.5) one.PY += 0.05;


		if (FoodCount >=49)
		{
			for (int i = 0; i + 1 < FoodCount; i++)
			{
				if (i + 1 <= 49)
					food[i] = food[i + 1];
			}
			FoodCount--;
		}
		food[FoodCount].Goal.x = mx;
		food[FoodCount].Goal.y = my;
		food[FoodCount].radius = 7; // 반지름 15 시작 원 둘레 30파이

		xdiff = ((double)mx - (double)one.pivot.x);
		ydiff = ((double)my - (double)one.pivot.y);
		degree = atan2(ydiff, xdiff);

		if (!InCircle(mx, my, one.pivot.x, one.pivot.y))
		{
			food[FoodCount].pivot.x = one.pivot.x + cos(degree) * (one.radius + 10); // 먹이 반지름 반큼 더 
			food[FoodCount].start.x = food[FoodCount].pivot.x;
			food[FoodCount].pivot.y = one.pivot.y + sin(degree) * (one.radius + 10);
			food[FoodCount].start.y = food[FoodCount].pivot.y;
		}

		if (food[FoodCount].pivot.x < 7) food[FoodCount].pivot.x = 7;
		if (food[FoodCount].pivot.x + food[FoodCount].radius > client.right) food[FoodCount].pivot.x -= 7;
		if (food[FoodCount].pivot.y < 7) food[FoodCount].pivot.y = 7;
		if (food[FoodCount].pivot.y + food[FoodCount].radius > client.bottom) food[FoodCount].pivot.y -= 7;
		food[FoodCount].color = RGB(rand() % 255, rand() % 255, rand() % 255);
		food[FoodCount].PX = 4.0;
		food[FoodCount].PY = 4.0;
		food[FoodCount].timer = 0;
		FoodCount++;
		InvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hwnd, iMessage, wParam, lParam));
}

#endif