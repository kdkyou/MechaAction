#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

//============================
//ヘルプツールチップ表示関数
// ・text			… 表示文字列
// ・showIcom		… true:(?)を表示しその上にマウスがきたら表示
// 　　　　　　　　　　false:現在のコントロールの上にマウスがきたら表示
//============================
inline void ImGuiShowHelp(const std::string& text, bool showIcon = false)
{
	if (showIcon) {
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(text.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

//============================
// ログウィンドウクラス
//============================
class ImGuiLogWindow {
public:
	// ログをクリア
	void    Clear() { m_Buf.clear(); }

	// 文字列を追加
	template<class... Args>
	void AddLog(const std::string& fmt, Args... args)
	{
		// 改行を加える
		std::string str = fmt;
		str += "\n";
		m_Buf.appendf(str.c_str(), args...);
		m_ScrollToBottom = true;
	}

	// ウィンドウ描画
	void ImGuiUpdate(const std::string& title, bool* p_opened = NULL)
	{
		ImGui::Begin(title.c_str(), p_opened);
		ImGui::TextUnformatted(m_Buf.begin());
		if (m_ScrollToBottom) {
			ImGui::SetScrollHereY(1.0f);
		}
		m_ScrollToBottom = false;
		ImGui::End();
	}

private:
	ImGuiTextBuffer     m_Buf;
	bool                m_ScrollToBottom = false;
};
