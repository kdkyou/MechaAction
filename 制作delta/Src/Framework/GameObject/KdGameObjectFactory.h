#pragma once

// 生成関数登録用マクロ：文字列で任意のクラスを生成するため
#define ObjectFactoryRegisterCreateFunction(_name) \
	KdGameObjectFactory::Instance().RegisterCreateFunction(#_name, []()\
mutable { return KdGameObjectFactory::Instance().CreateGameObject<_name>(); });\

class KdGameObject;

class KdGameObjectFactory
{
public:

	~KdGameObjectFactory() { Release(); }

	void RegisterCreateFunction(const std::string_view, const std::function <std::shared_ptr<KdGameObject>(void)> func);

	template<class T>
	std::shared_ptr<T> CreateGameObject()
	{
		std::shared_ptr<T> spObj = std::make_shared<T>();

		spObj->Init();

		return spObj;
	}

	std::shared_ptr<KdGameObject> CreateGameObject(const std::string_view objName) const;

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

	static KdGameObjectFactory& Instance()
	{
		static KdGameObjectFactory instance;
		return instance;
	}

	void Release() { m_createFunctions.clear(); }

private:

	// GameObjectの生成関数：文字列検索可能
	std::unordered_map<std::string_view, std::function<std::shared_ptr<KdGameObject>(void)>> m_createFunctions;

	KdGameObjectFactory() {}
};
