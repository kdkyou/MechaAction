#include"ConcentrationLine.h"

void ConcentrationLine::Update()
{
	if (m_textures.size() <= 0) { return; }

	if (m_progress >= 1.0f) {
		m_isExpired = true;
		return;
	}

	auto  deltaTime = KdFPSController::GetInstance().GetDeltaTime();
	// 一秒に進行するフレーム数
	float p = 1.0f / m_duration * deltaTime;

	m_progress += p;

	static float interval = 0;

	interval += p;
	
	if (interval >= m_interval)
	{
		interval = 0;
		++m_num;
	}

}

void ConcentrationLine::PostUpdate()
{
	// 取得した枚数を超える場合０に
	if (m_num > m_textures.size() -1 )
	{
		m_num = 0;
	}

}


void ConcentrationLine::DrawSprite()
{

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	auto tex = m_textures[m_num];

	KdShaderManager::Instance().m_spriteShader.DrawTex(tex,0,0);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);

}

bool ConcentrationLine::SetParam(const std::vector<std::string>& pathes, float inteval, float duration)
{
	if (pathes.size() <= 0) { return false; }

	for (auto& path : pathes)
	{
		std::shared_ptr<KdTexture> texture = std::make_shared<KdTexture>();
		texture = KdAssets::Instance().m_textures.GetData(PATH+path+PNG);
		m_textures.push_back(texture);
	}

	m_interval = inteval;

	m_duration = duration;

	m_progress = 0.0f;
	m_num = 0;

	return true;
}
