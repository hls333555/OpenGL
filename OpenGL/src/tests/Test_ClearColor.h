#pragma once

#include "Test.h"

namespace test
{
	class Test_ClearColor : public Test
	{
	public:
		Test_ClearColor();
		~Test_ClearColor() {}

		virtual void OnRender() override;
		virtual void OnImGuiRender() override;

	private:
		float m_ClearColor[4];
	};

}
