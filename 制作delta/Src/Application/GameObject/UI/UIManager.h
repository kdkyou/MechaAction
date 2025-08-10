#pragma once


class NumberUI;

class UIManager
{
public:

	enum class CreateType {
		No,
		Draw,
		Number,
		Guage,

	};

	void UIInit();

	void SceneUICreate();

	void PreUpdate();
	void Update();

	void DrawSprite();

	void SetPlayerHP(int hp) { m_playerHP = hp; }
	void SetRightAmmo(int ammo) { m_playerRightWeaponAmmo = ammo; }
	void SetLeftAmmo(int ammo) { m_playerLeftWeaponAmmo = ammo; }

	void Editor_ImGui();

	void CreateUI();

	void AddUI(std::shared_ptr<KdGameObject> obj);

private:
	
	UIManager() { }

	void Deserialize(const std::string& path);
	
	int m_playerHP = 0;
	int m_playerRightWeaponAmmo = 0;
	int m_playerLeftWeaponAmmo = 0;

	CreateType m_nowCreateType = CreateType::No;
	
	std::weak_ptr<NumberUI> m_hpUI;
	std::weak_ptr<NumberUI> m_rightWeaponUI;
	std::weak_ptr<NumberUI> m_leftWeaponUI;

	std::list<std::shared_ptr<KdGameObject>> m_uiList;

public:

	static UIManager& GetInstance()
	{
		static UIManager instance;
		return instance;
	}



};