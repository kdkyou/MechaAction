#pragma once

// 生成関数登録用マクロ：文字列で任意のクラスを生成するため
#define ObjectFactoryRegisterCreateFunction(_name) \
	KdGameObjectFactory::Instance().RegisterCreateFunction(#_name, []()\
mutable { return KdGameObjectFactory::Instance().CreateGameObject<_name>(); });\

class KdGameObject;

class CharacterBase;

class WeaponBase;

class KdGameObjectFactory
{
public:

	~KdGameObjectFactory() { Release(); }

	void RegisterCreateFunction(const std::string_view, const std::function <std::shared_ptr<KdGameObject>(void)> func);
	
	void RegisterCreateCharaFunction(const std::string_view, const std::function <std::shared_ptr<CharacterBase>(void)> func);

	void RegisterCreateWeaponFunction(const std::string_view, const std::function <std::shared_ptr<WeaponBase>(void)> func);

	template<class T>
	std::shared_ptr<T> CreateGameObject()
	{
		std::shared_ptr<T> spObj = std::make_shared<T>();

		spObj->Init();

		return spObj;
	}

	std::shared_ptr<KdGameObject> CreateGameObject(const std::string_view objName) const;
	
	// キャラ
	template<class E>
	std::shared_ptr<E> CreateCharacterBase()
	{
		std::shared_ptr<E> spObj = std::make_shared<E>();

		spObj->Init();

		return spObj;
	}
	std::shared_ptr<CharacterBase> CreateCharacterBase(const std::string_view objName) const;
	
	// 武器
	template<class W>
	std::shared_ptr<W> CreateWeaponBase()
	{
		std::shared_ptr<W> spObj = std::make_shared<W>();

		spObj->Init();

		return spObj;
	}
	std::shared_ptr<WeaponBase> CreateWeaponBase(const std::string_view objName) const;




	template<typename T>
	void RegisterGameObject(const std::string_view _name)
	{
		auto createFunc = []() -> std::shared_ptr<KdGameObject>
			{
				return std::make_shared<T>();
			};

		RegisterCreateFunction(_name, createFunc);
	}

	const std::vector<std::string> GetRegisterObjectList()
	{
		std::vector<std::string> list;
		for (auto pair : m_createFunctions)
		{
			list.push_back(pair.first.data());
		}

		return list;
	}

	//// キャラ
	//template<typename E>
	//void RegisterCharacterBase(const std::string_view _name)
	//{
	//	auto createFunc = []() -> std::shared_ptr<CharacterBase>
	//		{
	//			return std::make_shared<E>();
	//		};

	//	RegisterCreateCharaFunction(_name, createFunc);
	//}

	//const std::vector<std::string> GetRegisterCharaList()
	//{
	//	std::vector<std::string> list;
	//	for (auto pair : m_createCharaFunction)
	//	{
	//		list.push_back(pair.first.data());
	//	}

	//	return list;
	//}

	// 武器
	template<typename W>
	void RegisterWeaponBase(const std::string_view _name)
	{
		auto createFunc = []() -> std::shared_ptr<WeaponBase>
			{
				return std::make_shared<W>();
			};

		RegisterCreateWeaponFunction(_name, createFunc);
	}

	const std::vector<std::string> GetRegisterWeaponList()
	{
		std::vector<std::string> list;
		for (auto pair : m_createWeaponFunction)
		{
			list.push_back(pair.first.data());
		}

		return list;
	}




	static KdGameObjectFactory& Instance()
	{
		static KdGameObjectFactory instance;
		return instance;
	}

	void Release() { m_createFunctions.clear(); }

private:

	// GameObjectの生成関数：文字列検索可能
	std::unordered_map<std::string_view, std::function<std::shared_ptr<KdGameObject>(void)>> m_createFunctions;

	// CharacterBaseの生成関数：文字列検索可能
	std::unordered_map<std::string_view, std::function<std::shared_ptr<CharacterBase>(void)>> m_createCharaFunction;
	
	// CharacterBaseの生成関数：文字列検索可能
	std::unordered_map<std::string_view, std::function<std::shared_ptr<WeaponBase>(void)>> m_createWeaponFunction;

	KdGameObjectFactory() {}
};
