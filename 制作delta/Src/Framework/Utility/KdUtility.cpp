#include "Framework/KdFramework.h"

#include "KdUtility.h"

// viewから画像情報を取得する
void KdGetTextureInfo(ID3D11View* view, D3D11_TEXTURE2D_DESC& outDesc)
{
	outDesc = {};

	ID3D11Resource* res;
	view->GetResource(&res);

	ID3D11Texture2D* tex2D;
	if (SUCCEEDED(res->QueryInterface<ID3D11Texture2D>(&tex2D)))
	{
		tex2D->GetDesc(&outDesc);
		tex2D->Release();
	}
	res->Release();
}

bool ConvertRectToUV(const KdTexture* srcTex, const Math::Rectangle& src, Math::Vector2& uvMin, Math::Vector2& uvMax)
{
	if (!srcTex) { return false; }

	uvMin.x = src.x / (float)srcTex->GetInfo().Width;
	uvMin.y = src.y / (float)srcTex->GetInfo().Height;

	uvMax.x = ( src.width  / (float)srcTex->GetInfo().Width) + uvMin.x;
	uvMax.y = ( src.height / (float)srcTex->GetInfo().Height) + uvMin.y;

	return true;
}

float EaseInOutSine(float progress)
{
	return (float)(-(std::cos(M_PI * progress) - 1.0f) / 2.0f);
}

float BulletDamage(const Math::Vector3& _startPos, const Math::Vector3& _endPos, float _damage, float _enableLength, float _dampingInterval, float _dampingRate)
{
	// 開始地点と着弾地点から距離を判定
	auto length = (_endPos - _startPos).Length();

	// 有効射程内なら元のダメージを返す
	if (_enableLength >= length) { return _damage; }

	//有効射程からどれだけ離れているか
	auto overLength = length - _enableLength;

	// 係数を離れた距離と減衰間隔から取得
	// 何乗するか
	int calcDumping = (overLength / _dampingInterval);

	// ダメージ計算
	float nowDamage = _damage * std::pow(_dampingRate, calcDumping);

	// 念のための最低値
	if (nowDamage < 0.0f)
	{
		nowDamage = 0.0f;
	}

	return nowDamage;
}
