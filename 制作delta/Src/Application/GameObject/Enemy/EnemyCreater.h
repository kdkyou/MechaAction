#pragma once

class EnemyCreater
{
public :

	void EnemysCreate(const std::string& filePath);

	void Editor_ImGui();

private:

	EnemyCreater() {}


public:

	static EnemyCreater& GetInstance()
	{
		static EnemyCreater instance;
		return instance;
	}

};