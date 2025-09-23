#pragma once

class NumberUI;

#include "../Effect/Fade/Fade.h"

class UIManager
{
public:

	enum class CreateType {
		No,
		Draw,
		Number,
		Guage,
		Blink,
		Map,
	};

	void UIInit();

	void SceneUICreate();
	void SceneUICreate(const std::string& path);

	void PreUpdate();
	void Update();

	void DrawSprite();

	void SetPlayerHP(int hp) { m_playerHP = hp; }
	void SetRightAmmo(int ammo) { m_playerRightWeaponAmmo = ammo; }
	void SetLeftAmmo(int ammo) { m_playerLeftWeaponAmmo = ammo; }
	void SetRightAmmoOne(int ammo) { m_playerRightWeaponAmmoOne = ammo; }
	void SetLeftAmmoOne(int ammo) { m_playerLeftWeaponAmmoOne = ammo; }

	void Editor_ImGui();

	void CreateUI();

	void ListClear() { m_uiList.clear(); }
	
	void AddUI(std::shared_ptr<KdGameObject> obj);

	//												 true = In
	void SetFade(Fade::FadeType type, float time, bool OutorIn = true);
	const bool IsFadeComplete()const;

private:
	
	UIManager() { UIInit(); }

	void Deserialize(const std::string& path);
	
	int m_playerHP = 0;
	int m_playerRightWeaponAmmo = 0;
	int m_playerLeftWeaponAmmo = 0;
	int m_playerRightWeaponAmmoOne = 0;
	int m_playerLeftWeaponAmmoOne = 0;

	CreateType m_nowCreateType = CreateType::No;
	
	std::weak_ptr<NumberUI> m_hpUI;
	std::weak_ptr<NumberUI> m_rightWeaponUI;
	std::weak_ptr<NumberUI> m_rightWeaponOneUI;
	std::weak_ptr<NumberUI> m_leftWeaponUI;
	std::weak_ptr<NumberUI> m_leftWeaponOneUI;

	std::shared_ptr<Fade> m_fade;

	std::list<std::shared_ptr<KdGameObject>> m_uiList;

public:

	static UIManager& GetInstance()
	{
		static UIManager instance;
		return instance;
	}



};