#include "EditorData.h"

#include "../../Application/Scene/SceneManager.h"
#include "../../Application/GameObject/Camera/CameraManager.h"
#include "../../Application/GameObject/UI/UIManager.h"

#include "../../Application/GameObject/Character/CharacterBase.h"



bool EditorData::OpenFileDialog(std::string& filepath, const std::string& title, const char* filters)
{
	// 現在のカレントディレクトリ保存
	auto current = std::filesystem::current_path();
	// ファイル名のみ
	auto filename = std::filesystem::path(filepath).filename();

	// 結果用
	static char fname[1000];
	strcpy_s(fname, sizeof(fname), filename.string().c_str());

	// デフォルトフォルダ
	std::string dir;
	if (filepath.size() == 0)
	{
		dir = current.string() + "\\";
	}
	else {
		auto path = std::filesystem::absolute(filepath);
		dir = path.parent_path().string() + "\\";
	}

	OPENFILENAMEA o;
	ZeroMemory(&o, sizeof(o));

	o.lStructSize = sizeof(o);									// 構造体サイズ
	o.hwndOwner = nullptr;										// 親ウィンドウのハンドル
	o.lpstrInitialDir = dir.c_str();							// 初期フォルダー
	o.lpstrFile = fname;										// 取得したファイル名を保存するバッファ
	o.nMaxFile = sizeof(fname);									// 取得したファイル名を保存するバッファサイズ
	o.lpstrFilter = filters;									// (例) "TXTファイル(*.TXT)\0*.TXT\0全てのファイル(*.*)\0*.*\0";
	o.lpstrDefExt = "";
	o.lpstrTitle = title.c_str();
	o.nFilterIndex = 1;
	if (GetOpenFileNameA(&o))
	{
		// カレントディレクトリを元に戻す
		std::filesystem::current_path(current);
		// 相対パスへ変換
		filepath = std::filesystem::relative(fname).string();
		return true;
	}
	std::filesystem::current_path(current);	// カレントディレクトリを元に戻す
	return false;
}

bool EditorData::SaveFileDialog(std::string& filepath, const std::string& title, const char* filters, const std::string& defExt)
{
	// 現在のカレントディレクトリ保存
	auto current = std::filesystem::current_path();
	// ファイル名のみ
	auto filename = std::filesystem::path(filepath).filename();

	// 結果用
	static char fname[1000];
	strcpy_s(fname, sizeof(fname), filename.string().c_str());

	// デフォルトフォルダ
	std::string dir;
	if (filepath.size() == 0)
	{
		dir = current.string() + "\\";
	}
	else {
		auto path = std::filesystem::absolute(filepath);
		dir = path.parent_path().string() + "\\";
	}

	OPENFILENAMEA o;
	ZeroMemory(&o, sizeof(o));

	o.lStructSize = sizeof(o);									// 構造体サイズ
	o.hwndOwner = nullptr;										// 親ウィンドウのハンドル
	o.lpstrInitialDir = dir.c_str();						// 初期フォルダー
	o.lpstrFile = fname;										// 取得したファイル名を保存するバッファ
	o.nMaxFile = sizeof(fname);									// 取得したファイル名を保存するバッファサイズ
	o.lpstrFilter = filters;									// (例) "TXTファイル(*.TXT)\0*.TXT\0全てのファイル(*.*)\0*.*\0";
	o.lpstrDefExt = defExt.c_str();
	o.lpstrTitle = title.c_str();
	o.nFilterIndex = 1;
	o.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	if (GetSaveFileNameA(&o))
	{
		// カレントディレクトリを元に戻す
		std::filesystem::current_path(current);
		// 相対パスへ変換
		filepath = std::filesystem::relative(fname).string();
		return true;
	}
	std::filesystem::current_path(current);	// カレントディレクトリを元に戻す
	return false;
}


// 

void EditorData::Initialize()
{
	
}


void EditorData::UpdateImGui()
{
	

	// ImGui Demo ウィンドウ表示 ※すごく参考になるウィンドウです。imgui_demo.cpp参照。
	ImGui::ShowDemoWindow(nullptr);


	//===========================
	// システム
	//===========================
	if (ImGui::Begin("System", 0, ImGuiWindowFlags_MenuBar))
	{
		// メニュー 
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Fail"))
			{

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		ImGui::Checkbox((const char*)u8"エディター", &m_editorMode);
		/*
		if (m_editorMode)
		{
			ImGui::Text(u8"エディターモード");
		}
		else
		{
			ImGui::Text(u8"ゲームモード");
		}
		*/

		// ImGuizmo
		if (ImGui::CollapsingHeader("Gizmo", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::RadioButton("Local", GizmoMode == ImGuizmo::LOCAL)) {
				GizmoMode = ImGuizmo::LOCAL;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("World", GizmoMode == ImGuizmo::WORLD)) {
				GizmoMode = ImGuizmo::WORLD;
			}

			if (ImGui::RadioButton((const char*)u8"座標", GizmoOperation == ImGuizmo::TRANSLATE)) {
				GizmoOperation = ImGuizmo::TRANSLATE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton((const char*)u8"回転", GizmoOperation == ImGuizmo::ROTATE)) {
				GizmoOperation = ImGuizmo::ROTATE;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton((const char*)u8"Scale", GizmoOperation == ImGuizmo::SCALE)) {
				GizmoOperation = ImGuizmo::SCALE;
			}
		}
	}
	ImGui::End();

	//===========================
	// Inspectorウィンドウ
	//===========================
	if (ImGui::Begin("Inspector"))
	{
		auto nowSelectedObj = SelectObj.lock();
		if (nowSelectedObj != nullptr)
		{
			nowSelectedObj->Editor_ImGui();

			/*ImGui::Separator();

			if (ImGui::Button(""))
			{

			}*/

		}

	}
	ImGui::End();

	//===========================
	// Hierarchyウィンドウ
	//===========================
	if(ImGui::Begin((const char*)u8"オブジェクトリスト", 0, 0))
	{
		static int sel = 0;	// 
		if (ImGui::BeginListBox((const char*)u8"リスト選択"))
		{
			if (ImGui::Selectable("None")) { sel = 0; }
			if (ImGui::Selectable((const char*)u8"カレント")) { sel = 1; }
			if (ImGui::Selectable((const char*)u8"プレイヤー")) { sel = 2; }
			if (ImGui::Selectable((const char*)u8"エネミー")) { sel = 3; }
			if (ImGui::Selectable((const char*)u8"地形")) { sel = 4; }
			ImGui::EndListBox();
		}

		if (sel == 1)
		{
		if (ImGui::BeginListBox((const char*)u8"カレントリスト"))
			{
				for (auto& obj : SceneManager::Instance().GetObjList())
				{
					if (ImGui::Selectable((const char*)u8""))
					{
						SelectObj = obj;
					}
				}
				ImGui::EndListBox();
			}
		}

		if (sel == 2)
		{
			if (ImGui::BeginListBox((const char*)u8"プレイヤーリスト"))
			{
				for (auto& obj : SceneManager::Instance().GetPlayerList())
				{
					if (ImGui::Selectable((const char*)u8"取得"))
					{
						SelectObj = obj;
					}
				}
			ImGui::EndListBox();
			}
		}

		if (sel == 3)
		{
			if (ImGui::BeginListBox((const char*)u8"エネミーリスト"))
			{
				for (auto& obj : SceneManager::Instance().GetEnemyList())
				{
					if (ImGui::Selectable((const char*)u8"取得"))
					{
						SelectObj = obj;
					}
				}
				ImGui::EndListBox();
			}
		}

		if (sel == 4)
		{
			if (ImGui::BeginListBox((const char*)u8"地形リスト"))
			{
				for (auto& obj : SceneManager::Instance().GetTerrainList())
				{
					if (ImGui::Selectable((const char*)u8"取得"))
					{
						SelectObj = obj;
					}
				}
				ImGui::EndListBox();
			}
		}
		
	}
	ImGui::End();


	if (ImGui::Begin((const char*)u8"オブジェクト生成",0,0))
	{
		SceneManager::Instance().Edit_ImGui();
	}
	ImGui::End();
	
	if (ImGui::Begin((const char*)u8"カメラ制御",0,0))
	{
		CameraManager::Instance().Editor_ImGui();
	}
	ImGui::End();
	
	if (ImGui::Begin((const char*)u8"UIエディター", 0, 0))
	{
		UIManager::GetInstance().Editor_ImGui();
	}
	ImGui::End();

	//===========================
	// ログウィンドウ
	//===========================
	//m_logWindow.ImGuiUpdate("Log Window");

	
	

	////===========================
	//// GameViewウィンドウ
	////===========================
	//// エディターモード時は、ゲーム画面を表示
	//if (EditorData::GetInstance().m_editorMode)
	//{
	//	if (ImGui::Begin("Game View", 0, 0))
	//	{
	//		auto tex = KdFramework::GetInstance().m_renderingData.m_resultTex;

	//		ImVec2 wSize = ImGui::GetWindowSize();

	//		wSize.y = wSize.x / tex->GetAspectRatio();
	//		ImGui::Image((ImTextureID)tex->GetSRView().Get(), { wSize.x, wSize.y });
	//	}
	//	ImGui::End();
	//}

	//===========================
	// シェーダーウィンドウ
	//===========================
	if (ImGui::Begin("Render Settings", 0, 0))
	{
		// 
		RenderSetting::GetInstance().Editor_ImGui();
	}
	ImGui::End();

	//===========================
	// アセットウィンドウ
	//===========================
	//if (ImGui::Begin(u8"Asset Manager", 0, ImGuiWindowFlags_MenuBar))
	//{
	//	// メニューバー
	//	if (ImGui::BeginMenuBar())
	//	{
	//		// メニュー一項目(横に並ぶ)
	//		if (ImGui::BeginMenu(u8"ファイル操作"))
	//		{
	//			// メニュー項目(縦)
	//			if (ImGui::MenuItem(u8"Metaファイル作成"))
	//			{
	//				KdAssetManager::GetInstance().CreateMetaFileForAllAssets();
	//			}

	//			// メニュー項目(縦)
	//			if (ImGui::MenuItem(u8"実行時データ更新"))
	//			{
	//				KdAssetManager::GetInstance().CreateRuntimeData();
	//			}

	//			ImGui::Separator();

	//			if (ImGui::MenuItem(u8"使用Assetのコピー"))
	//			{
	//				KdAssetManager::GetInstance().CopyAssetsKeepFolders();
	//			}

	//			// メニュー項目
	//			if (ImGui::MenuItem(u8"Metaファイル削除"))
	//			{
	//				KdAssetManager::GetInstance().DeleteAllMetaFiles();
	//			}




	//			ImGui::EndMenu();
	//		}

	//		ImGui::EndMenuBar();
	//	}


	//	// 画面本体
	//	KdAssetManager::GetInstance().Editor_ImGui();

	//}
	//ImGui::End();

}
