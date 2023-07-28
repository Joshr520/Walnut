#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include <GLFW/glfw3.h>
#include "Walnut/UI/UI.h"

#include "Walnut/ImGui/ImGuiTheme.h"

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();

		UI_DrawAboutModal();
	}

	void UI_DrawAboutModal()
	{
		if (!m_AboutModalOpen)
			return;

		ImGui::OpenPopup("About");
		m_AboutModalOpen = ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (m_AboutModalOpen)
		{
			auto image = Walnut::Application::Get().GetApplicationIcon();
			ImGui::Image(image->GetDescriptorSet(), { 48, 48 });

			ImGui::SameLine();
			Walnut::UI::ShiftCursorX(20.0f);

			ImGui::BeginGroup();
			ImGui::Text("Walnut application framework");
			ImGui::Text("by Studio Cherno.");
			ImGui::EndGroup();

			if (Walnut::UI::ButtonCentered("Close"))
			{
				m_AboutModalOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}
private:
	bool m_AboutModalOpen = false;
};

void UI_DrawTitlebar(float& outTitlebarHeight)
{
	Walnut::Application& app = Walnut::Application::Get();

	const float titlebarHeight = 58.0f;
	const bool isMaximized = app.IsMaximized();
	float titlebarVerticalOffset = isMaximized ? -6.0f : 0.0f;
	const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

	ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + titlebarVerticalOffset));
	const ImVec2 titlebarMin = ImGui::GetCursorScreenPos();
	const ImVec2 titlebarMax = { ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.y * 2.0f,
								 ImGui::GetCursorScreenPos().y + titlebarHeight };
	auto* bgDrawList = ImGui::GetBackgroundDrawList();
	auto* fgDrawList = ImGui::GetForegroundDrawList();
	bgDrawList->AddRectFilled(titlebarMin, titlebarMax, Walnut::UI::Colors::Theme::titlebar);
	// DEBUG TITLEBAR BOUNDS
	// fgDrawList->AddRect(titlebarMin, titlebarMax, Walnut::UI::Colors::Theme::invalidPrefab);

	// Logo
	{
		const int logoWidth = 48;// m_LogoTex->GetWidth();
		const int logoHeight = 48;// m_LogoTex->GetHeight();
		const ImVec2 logoOffset(16.0f + windowPadding.x, 5.0f + windowPadding.y + titlebarVerticalOffset);
		const ImVec2 logoRectStart = { ImGui::GetItemRectMin().x + logoOffset.x, ImGui::GetItemRectMin().y + logoOffset.y };
		const ImVec2 logoRectMax = { logoRectStart.x + logoWidth, logoRectStart.y + logoHeight };
		fgDrawList->AddImage(Walnut::Application::Get().GetApplicationIcon()->GetDescriptorSet(), logoRectStart, logoRectMax);
	}

	ImGui::BeginHorizontal("Titlebar", { ImGui::GetWindowWidth() - windowPadding.y * 2.0f, ImGui::GetFrameHeightWithSpacing() });

	static float moveOffsetX;
	static float moveOffsetY;
	const float w = ImGui::GetContentRegionAvail().x;
	const float buttonsAreaWidth = 105.0f;

	// Title bar drag area
		// On Windows we hook into the GLFW win32 window internals
	ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + titlebarVerticalOffset)); // Reset cursor pos
	// DEBUG DRAG BOUNDS
	//fgDrawList->AddRect(ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + w - buttonsAreaWidth, ImGui::GetCursorScreenPos().y + titlebarHeight), UI::Colors::Theme::invalidPrefab);
	ImGui::InvisibleButton("##titleBarDragZone", ImVec2(w - buttonsAreaWidth, titlebarHeight));

	app.SetTitleBarHovered(ImGui::IsItemHovered());

	if (isMaximized)
	{
		float windowMousePosY = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
		if (windowMousePosY >= 0.0f && windowMousePosY <= 5.0f)
			app.SetTitleBarHovered(true); // Account for the top-most pixels which don't register
	}

	{
		// Centered Window title
		ImVec2 currentCursorPos = ImGui::GetCursorPos();
		ImVec2 textSize = ImGui::CalcTextSize(app.GetAppSpec().Name.c_str());
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - textSize.x * 0.5f, 2.0f + windowPadding.y + 6.0f));
		ImGui::Text("%s", app.GetAppSpec().Name.c_str()); // Draw title
		ImGui::SetCursorPos(currentCursorPos);
	}

	// Window buttons
	const ImU32 buttonColN = Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 0.9f);
	const ImU32 buttonColH = Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 1.2f);
	const ImU32 buttonColP = Walnut::UI::Colors::Theme::textDarker;
	const float buttonWidth = 14.0f;
	const float buttonHeight = 14.0f;

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

	// Minimize Button
	ImGui::Spring();
	Walnut::UI::ShiftCursorY(8.0f);
	{
		const int iconWidth = app.GetApplicationIconClose()->GetWidth();
		const int iconHeight = app.GetApplicationIconMinimize()->GetHeight();
		const float padY = (buttonHeight - (float)iconHeight) / 2.0f;

		if (ImGui::Button("##Minimize", ImVec2(buttonWidth * 2, buttonHeight * 2)))
		{
			// TODO: move this stuff to a better place, like Window class
			if (app.GetWindowHandle())
			{
				Walnut::Application::Get().QueueEvent([windowHandle = app.GetWindowHandle()]() { glfwIconifyWindow(windowHandle); });
			}
		}

		ImRect minRect = Walnut::UI::GetItemRect();
		minRect.Min.x += buttonWidth * 0.5;
		minRect.Min.y += buttonHeight * 0.5;
		minRect.Max.x -= buttonWidth * 0.5;
		minRect.Max.y -= buttonHeight * 0.5;

		Walnut::UI::DrawButtonImage(app.GetApplicationIconMinimize(), buttonColN, buttonColH, buttonColP, Walnut::UI::RectExpanded(minRect, 0.0f, -padY));
	}


	// Maximize Button
	ImGui::Spring();
	Walnut::UI::ShiftCursorY(8.0f);
	{
		const int iconWidth = app.GetApplicationIconMaximize()->GetWidth();
		const int iconHeight = app.GetApplicationIconMaximize()->GetHeight();
		const float padY = (buttonHeight - (float)iconHeight) / 2.0f;

		const bool isMaximized = app.IsMaximized();

		if (ImGui::Button("##Maximize", ImVec2(buttonWidth * 2, buttonHeight * 2)))
		{
			Walnut::Application::Get().QueueEvent([isMaximized, windowHandle = app.GetWindowHandle()]()
				{
					if (isMaximized)
						glfwRestoreWindow(windowHandle);
					else
						glfwMaximizeWindow(windowHandle);
				});
		}

		ImRect maxRect = Walnut::UI::GetItemRect();
		maxRect.Min.x += buttonWidth * 0.5;
		maxRect.Min.y += buttonHeight * 0.5;
		maxRect.Max.x -= buttonWidth * 0.5;
		maxRect.Max.y -= buttonHeight * 0.5;

		Walnut::UI::DrawButtonImage(isMaximized ? app.GetApplicationIconRestore() : app.GetApplicationIconMaximize(), buttonColN, buttonColH, buttonColP, Walnut::UI::RectExpanded(maxRect, 0.0f, -padY));
	}

	// Close Button
	ImGui::Spring();
	Walnut::UI::ShiftCursorY(8.0f);
	{
		const int iconWidth = app.GetApplicationIconClose()->GetWidth();
		const int iconHeight = app.GetApplicationIconClose()->GetHeight();
		const float padY = (buttonHeight - (float)iconHeight) / 2.0f;

		if (ImGui::Button("##Close", ImVec2(buttonWidth * 2, buttonHeight * 2)))
			Walnut::Application::Get().Close();

		ImRect closeRect = Walnut::UI::GetItemRect();
		closeRect.Min.x += buttonWidth * 0.5;
		closeRect.Min.y += buttonHeight * 0.5;
		closeRect.Max.x -= buttonWidth * 0.5;
		closeRect.Max.y -= buttonHeight * 0.5;

		Walnut::UI::DrawButtonImage(app.GetApplicationIconClose(), Walnut::UI::Colors::Theme::text, Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 1.4f), buttonColP, Walnut::UI::RectExpanded(closeRect, 0.0f, -padY));
	}

	ImGui::PopStyleColor();

	ImGui::Spring(-1.0f, 18.0f);
	ImGui::EndHorizontal();

	outTitlebarHeight = titlebarHeight;
}

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";
	spec.IconPath = "Icon.png";
	spec.AppHeaderIconPath = "Icon.png";
	spec.CustomTitlebar = true;
	spec.CenterWindow = true;
	spec.CustomTitlebarFunc = std::function<void(float&)>(UI_DrawTitlebar);

	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<ExampleLayer> exampleLayer = std::make_shared<ExampleLayer>();
	app->PushLayer(exampleLayer);
	app->SetMenubarCallback([app, exampleLayer]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				exampleLayer->ShowAboutModal();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}