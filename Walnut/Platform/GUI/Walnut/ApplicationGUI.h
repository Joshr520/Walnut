#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Image.h"

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <filesystem>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "vulkan/vulkan.h"

#if defined(WL_PLATFORM_WINDOWS) && !defined(WL_HEADLESS)

#include <Windows.h>

#endif


void check_vk_result(VkResult err);

struct GLFWwindow;

namespace Walnut {

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;

		std::filesystem::path IconPath;

		std::vector<uint8_t> IconData;

		bool WindowResizeable = true;

		// Uses custom Walnut titlebar instead
		// of Windows default
		bool CustomTitlebar = false;

		// Window will be created in the center
		// of primary monitor
		bool CenterWindow = false;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();

		static Application& Get();

		void SetAppName(std::string_view name);

		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback) { m_MenubarCallback = menubarCallback; }

		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }

		void Close();

		bool IsMaximized() const;
		std::shared_ptr<Image> GetApplicationIcon() const { return m_AppHeaderIcon; }

		float GetTime();
		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
		bool IsTitleBarHovered() const { return m_TitleBarHovered; }

		static VkInstance GetInstance();
		static VkPhysicalDevice GetPhysicalDevice();
		static VkDevice GetDevice();

		static VkCommandBuffer GetCommandBuffer(bool begin);
		static void FlushCommandBuffer(VkCommandBuffer commandBuffer);

		static void SubmitResourceFree(std::function<void()>&& func);

		static ImFont* GetFont(const std::string& name);

		template<typename Func>
		void QueueEvent(Func&& func)
		{
			m_EventQueue.push(func);
		}

		void SetKeyCallback(std::function<void(int, int, int, int)> func) { m_KeyCallback = func; };
		void ResetKeyCallback() { m_KeyCallback = nullptr; };
		void SetMouseButtonCallback(std::function<void(int, int, int)> func) { m_MouseButtonCallback = func; };
		void ResetMouseButtonCallback() { m_MouseButtonCallback = nullptr; };

		std::function<void(int, int, int, int)> GetKeyCallback() { return m_KeyCallback; };
		std::function<void(int, int, int)> GetMouseButtonCallback() { return m_MouseButtonCallback; };

#if defined(WL_PLATFORM_WINDOWS) && !defined(WL_HEADLESS)

		void SetRawInputCallback(std::function<void(RAWINPUT*)> func) { m_RawInputCallback = func; };
		void ResetRawInputCallback() { m_RawInputCallback = nullptr; };

		std::function<void(RAWINPUT*)> GetRawInputCallback() { return m_RawInputCallback; };

#endif
	private:
		void Init();
		void Shutdown();

		// For custom titlebars
		void UI_DrawTitlebar(float& outTitlebarHeight);
		void UI_DrawMenubar();
	private:
		ApplicationSpecification m_Specification;
		GLFWwindow* m_WindowHandle = nullptr;
		bool m_Running = false;

		float m_TimeStep = 0.0f;
		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		bool m_TitleBarHovered = false;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenubarCallback;

		std::mutex m_EventQueueMutex;
		std::queue<std::function<void()>> m_EventQueue;

		// Resources
		// TODO(Yan): move out of application class since this can't be tied
		//            to application lifetime
		std::shared_ptr<Walnut::Image> m_AppHeaderIcon;
		std::shared_ptr<Walnut::Image> m_IconClose;
		std::shared_ptr<Walnut::Image> m_IconMinimize;
		std::shared_ptr<Walnut::Image> m_IconMaximize;
		std::shared_ptr<Walnut::Image> m_IconRestore;

		std::function<void(int, int, int, int)> m_KeyCallback;
		std::function<void(int, int, int)> m_MouseButtonCallback;

#if defined(WL_PLATFORM_WINDOWS) && !defined(WL_HEADLESS)

		std::function<void(RAWINPUT*)> m_RawInputCallback;

#endif
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}