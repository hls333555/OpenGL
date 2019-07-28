#pragma once

#include <unordered_map>
#include <string>
#include <functional>

namespace test
{
	class Test
	{
	public:
		Test() {}
		virtual ~Test() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}

	};

	class TestMenu : public Test
	{
	public:
		TestMenu(Test*& currentTest);
		~TestMenu() {}

		virtual void OnImGuiRender() override;

		template<typename T>
		void RegisterTest(const std::string& name)
		{
			std::cout << "Registering test: " << name << std::endl;
			m_Tests.insert(std::make_pair(name, []() { return new T(); }));
		}

	private:
		// Set it to be a reference so that it can modify the outside one internally
		Test*& m_CurrentTest;
		std::unordered_map<std::string, std::function<Test* ()>> m_Tests;
	};

}
