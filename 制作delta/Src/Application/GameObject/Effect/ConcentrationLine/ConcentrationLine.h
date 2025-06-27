#pragma once

//リストに入れられたテクスチャを生成してから一定間隔で切り替え描画する

class ConcentrationLine : public KdGameObject
{
public :

	void Update()override;
	void PostUpdate()override;

	void DrawSprite()override;

	// パラメータの設定 パスには.pngを入れなくていい 画像の切り替え
	bool SetParam(const std::vector<std::string>& pathes, float inteval,float duration);

private:

	// 画像格納
	std::vector<std::shared_ptr<KdTexture>>	 m_textures;
	// 間隔
	float									 m_interval;
	// 継続時間
	float									 m_duration;
	// 進行時間
	float									 m_progress;
	// 何番目を指すか
	int										 m_num;
	
};

//パス
#define PATH "Asset/Textures/GameObject/"
#define PNG ".png"