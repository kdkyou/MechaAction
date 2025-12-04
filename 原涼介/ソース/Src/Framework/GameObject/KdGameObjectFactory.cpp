#include "KdGameObjectFactory.h"

void KdGameObjectFactory::RegisterCreateFunction(const std::string_view str, const std::function<std::shared_ptr<KdGameObject>(void)> func)
{
	m_createFunctions[str.data()] = func;
}

std::shared_ptr<KdGameObject> KdGameObjectFactory::CreateGameObject(const std::string_view objName) const
{
	auto creater = m_createFunctions.find(objName);

	if (creater == m_createFunctions.end())
	{
		assert(0 && "GameObjectFactoryに未登録のゲームオブジェクトクラスです");

		return nullptr;
	}

	return  creater->second();
}

void KdGameObjectFactory::RegisterCreateCharaFunction(const std::string_view str, const std::function<std::shared_ptr<CharacterBase>(void)> func)
{
	m_createCharaFunction[str.data()] = func;
}


std::shared_ptr<CharacterBase> KdGameObjectFactory::CreateCharacterBase(const std::string_view objName) const
{
	auto creater = m_createCharaFunction.find(objName);

	if (creater == m_createCharaFunction.end())
	{
		assert(0 && "GameObjectFactoryに未登録のキャラクタークラスです");

		return nullptr;
	}

	return  creater->second();
}

void KdGameObjectFactory::RegisterCreateWeaponFunction(const std::string_view str, const std::function<std::shared_ptr<WeaponBase>(void)> func)
{
	m_createWeaponFunction[str.data()] = func;
}

std::shared_ptr<WeaponBase> KdGameObjectFactory::CreateWeaponBase(const std::string_view objName) const
{
	auto creater = m_createWeaponFunction.find(objName);

	if (creater == m_createWeaponFunction.end())
	{
		assert(0 && "GameObjectFactoryに未登録のウェポンクラスです");

		return nullptr;
	}

	return  creater->second();
}
