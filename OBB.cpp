//---------------------------------------------
// OBB vs OBB 衝突判定プログラム
// 2020-11-16 Monday
// 2021-08-24 Easy Check Systemに 改変
// ツイストケース checkしない
//---------------------------------------------
#include "OBB.h"

//Debug 2021-08-23 Monday
Vector3 dist;
FLOAT sepA;
FLOAT sepB;
FLOAT eru;
// 分離軸に投影された軸成分から投影線分長を算出
Vector3 fzero2 = Vector3(0, 0, 0);
//Vector3 ezero2 = XMLoadFloat3(&fzero2);

FLOAT LenSegOnSeparateAxis(Vector3 Sep, Vector3 e1, 
	Vector3 e2,Vector3 e3/*=ezero2*/)
{
	// 3つの内積の絶対値の和で投影線分長を計算
	// 分離軸Sepは標準化されていること
	FLOAT r1 =Sep.Dot(e1);
	FLOAT r2 = Sep.Dot(e2);

	Vector3 fe3;
	fe3=e3;
	
	FLOAT r3 = (fe3.x == 0 && fe3.y == 0 && fe3.z == 0) ?
		0 :
		(FLOAT)(Sep.Dot(e3));

	return r1 + r2 + r3;
}


bool CollisionOBB(COBB &obb1, COBB &obb2)
{
	bool ret[3] = { false,false,false };
	// 各方向ベクトルの確保
	// （N***:標準化方向ベクトル）
	Vector3 NAe1 = obb1.GetDirect(0), Ae1 = NAe1 * obb1.GetLen_W(0);
	Vector3 NAe2 = obb1.GetDirect(1), Ae2 = NAe2 * obb1.GetLen_W(1);
	Vector3 NAe3 = obb1.GetDirect(2), Ae3 = NAe3 * obb1.GetLen_W(2);
	Vector3 NBe1 = obb2.GetDirect(0), Be1 = NBe1 * obb2.GetLen_W(0);
	Vector3 NBe2 = obb2.GetDirect(1), Be2 = NBe2 * obb2.GetLen_W(1);
	Vector3 NBe3 = obb2.GetDirect(2), Be3 = NBe3 * obb2.GetLen_W(2);
	Vector3 Interval = obb1.GetPos_W() - obb2.GetPos_W();

	//2021-08-23 Debug
	dist = Interval;

	// 分離軸 : Ae1
	FLOAT rA = Ae1.Length();
	FLOAT rB = LenSegOnSeparateAxis(NAe1, Be1, Be2, Be3);
	FLOAT L = Interval.Dot(NAe1);
	//2021-08-23 Debug
	sepA = rA;
	sepB = rB;
	eru = L;

	if (L > rA + rB)
		return false; // 衝突していない

	 // 分離軸 : Ae2
	rA = Ae2.Length();
	rB = LenSegOnSeparateAxis(NAe2, Be1, Be2, Be3);
	L = Interval.Dot(NAe2);
	
	//2021-08-23 Debug
	sepA = rA;
	sepB = rB;
	eru = L;

	if (L > rA + rB)

		return false;

	// 分離軸 : Ae3
	//rA = D3DXVec3Length(&Ae3);
	//rB = LenSegOnSeparateAxis(NAe3, Be1, Be2, Be3);
	//L = fabs(D3DXVec3Dot(&Interval, &NAe3));
	
	rA = Ae3.Length();
	rB = LenSegOnSeparateAxis(NAe3, Be1, Be2, Be3);
	L = Interval.Dot(NAe3);
	
	//2021-08-23 Debug
	sepA = rA;
	sepB = rB;
	eru = L;

	if (L > rA + rB)
		return false;

	//2021-08-23 Debug Only
	//return true;

	// 分離軸 : Be1
	//rA = LenSegOnSeparateAxis(&NBe1, &Ae1, &Ae2, &Ae3);
	//rB = D3DXVec3Length(&Be1);
	//L = fabs(D3DXVec3Dot(&Interval, &NBe1));
	
	rA = LenSegOnSeparateAxis(NBe1, Ae1, Ae2, Ae3);
	rB = Be1.Length();
	L = Interval.Dot(NBe1);

	//2021-08-23 Debug
	sepA = rA;
	sepB = rB;
	eru = L;

	if (L > rA + rB)
		return false;

	// 分離軸 : Be2
	/*rA = LenSegOnSeparateAxis(&NBe2, &Ae1, &Ae2, &Ae3);
	rB = D3DXVec3Length(&Be2);
	L = fabs(D3DXVec3Dot(&Interval, &NBe2));
	*/
	rA = LenSegOnSeparateAxis(NBe2, Ae1, Ae2, Ae3);
	rB = Be2.Length();
	L = Interval.Dot(NBe2);

	//2021-08-23 Debug
	sepA = rA;
	sepB = rB;
	eru = L;

	if (L > rA + rB)
		return false;

	// 分離軸 : Be3
	//rA = LenSegOnSeparateAxis(&NBe3, &Ae1, &Ae2, &Ae3);
	//rB = D3DXVec3Length(&Be3);
	//L = fabs(D3DXVec3Dot(&Interval, &NBe3));
	rA = LenSegOnSeparateAxis(NBe3, Ae1, Ae2, Ae3);
	rB = Be3.Length();
	L = Interval.Dot(NBe3);
	
	//2021-08-23 Debug
	sepA = rA;
	sepB = rB;
	eru = L;

	if (L > rA + rB)
		return false;
	return true;//2021-08-24 Tuesday
	
	//以下ツイストしているケース
	// 分離軸 : C11
	/*D3DXVECTOR3 Cross;
	D3DXVec3Cross(&Cross, &NAe1, &NBe1);
	rA = LenSegOnSeparateAxis(&Cross, &Ae2, &Ae3);
	rB = LenSegOnSeparateAxis(&Cross, &Be2, &Be3);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Vector3 Cross;
	Cross=XMVector3Cross(NAe1, NBe1);

	Vector3 fzero=Vector3(0,0,0);
	Vector3 ezero;
	ezero=fzero;

	rA = LenSegOnSeparateAxis(Cross, Ae2, Ae3, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be2, Be3, ezero);
	L = Interval.Dot(Cross);

	if (L > rA + rB)
		return false;

	// 分離軸 : C12
	/*D3DXVec3Cross(&Cross, &NAe1, &NBe2);
	rA = LenSegOnSeparateAxis(&Cross, &Ae2, &Ae3);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be3);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	
	Cross=XMVector3Cross(NAe1, NBe2);
	rA = LenSegOnSeparateAxis(Cross, Ae2, Ae3, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be1, Be3, ezero);
	L = Interval.Dot(Cross);

	if (L > rA + rB)
		return false;

	// 分離軸 : C13
	//D3DXVec3Cross(&Cross, &NAe1, &NBe3);
	//rA = LenSegOnSeparateAxis(&Cross, &Ae2, &Ae3);
	//rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be2);
	//L = fabs(D3DXVec3Dot(&Interval, &Cross));
	
	Cross = XMVector3Cross(NAe1, NBe3);
	rA = LenSegOnSeparateAxis(Cross, Ae2, Ae3, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be1, Be2, ezero);
	L = Interval.Dot(Cross);

	if (L > rA + rB)
		return false;

	// 分離軸 : C21
	/*D3DXVec3Cross(&Cross, &NAe2, &NBe1);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae3);
	rB = LenSegOnSeparateAxis(&Cross, &Be2, &Be3);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Cross = XMVector3Cross(NAe2, NBe1);
	rA = LenSegOnSeparateAxis(Cross, Ae1, Ae3, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be2, Be3, ezero);
	L = Interval.Dot(Cross);
	   	
	if (L > rA + rB)
		return false;

	// 分離軸 : C22
	/*D3DXVec3Cross(&Cross, &NAe2, &NBe2);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae3);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be3);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Cross = XMVector3Cross(NAe2, NBe2);
	rA = LenSegOnSeparateAxis(Cross, Ae1, Ae3, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be1, Be3, ezero);
	L = Interval.Dot(Cross);

	if (L > rA + rB)
		return false;

	// 分離軸 : C23
	/*D3DXVec3Cross(&Cross, &NAe2, &NBe3);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae3);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be2);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Cross = XMVector3Cross(NAe2, NBe3);
	rA = LenSegOnSeparateAxis(Cross, Ae1, Ae3, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be1, Be2, ezero);
	L = Interval.Dot(Cross);
	   	
	if (L > rA + rB)
		return false;

	// 分離軸 : C31
	/*D3DXVec3Cross(&Cross, &NAe3, &NBe1);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae2);
	rB = LenSegOnSeparateAxis(&Cross, &Be2, &Be3);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Cross = XMVector3Cross(NAe3, NBe1);
	rA = LenSegOnSeparateAxis(Cross, Ae1, Ae2, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be2, Be3, ezero);
	L = Interval.Dot(Cross);
	   	
	if (L > rA + rB)
		return false;

	// 分離軸 : C32
	/*D3DXVec3Cross(&Cross, &NAe3, &NBe2);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae2);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be3);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Cross = XMVector3Cross(NAe3, NBe2);
	rA = LenSegOnSeparateAxis(Cross, Ae1, Ae2, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be1, Be3, ezero);
	L = Interval.Dot(Cross);
	   	
	if (L > rA + rB)
		return false;

	// 分離軸 : C33
	/*D3DXVec3Cross(&Cross, &NAe3, &NBe3);
	rA = LenSegOnSeparateAxis(&Cross, &Ae1, &Ae2);
	rB = LenSegOnSeparateAxis(&Cross, &Be1, &Be2);
	L = fabs(D3DXVec3Dot(&Interval, &Cross));
	*/
	Cross = XMVector3Cross(NAe3, NBe3);
	rA = LenSegOnSeparateAxis(Cross, Ae1, Ae2, ezero);
	rB = LenSegOnSeparateAxis(Cross, Be1, Be2, ezero);
	L = Interval.Dot(Cross);
	   	
	if (L > rA + rB)
		return false;

	// 分離平面が存在しないので「衝突している」
	
	return true;
}

