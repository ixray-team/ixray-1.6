#include "stdafx.h"
#include "MacroNodes.h"

CNodeMacro::CNodeMacro(size_t Macro, const xr_string Name) :
	INodeUnknown(Name.data()), MacroCommandID(Macro)
{
	AddContactLink("Out", true);
	AddContactLink("In");
};

void CNodeMacro::Draw()
{
	INodeUnknown::Draw();

	DrawHeader();

	if (Type == MacroType::eVoid)
	{

		ImGui::PushItemWidth(60);
		ImGui::Text("Empty args...");
		ImGui::PopItemWidth();
	}
	else
	{
		ImGui::Text("First Value:");
		ImGui::SameLine();
		ImGui::PushItemWidth(120);
		string256 Value1 = {};
		if (ImGui::InputText("##frstval", (char*)&Value1, sizeof(Value1)))
		{
			FirstValue = Value1;
		}
		ImGui::PopItemWidth();
	}

	if (Type == MacroType::eTwoArg)
	{
		ImGui::Text("Second Value:");
		ImGui::SameLine();
		ImGui::PushItemWidth(60);
		string256 Value2 = {};
		if (ImGui::InputText("##scndval", (char*)&Value2, sizeof(Value2)))
		{
			FirstValue = Value2;
		}
		ImGui::PopItemWidth();
	}

	DrawEnd();
}

void CNodeMacro::SetType(MacroType NewType)
{
	Type = NewType;

	int Red	  = (int)(NewType == MacroType::eOneArg) * 200 + MacroCommandID;
	int Blue  = (int)(NewType == MacroType::eVoid)   * 200 + MacroCommandID;
	int Green = (int)(NewType == MacroType::eTwoArg) * 200 + (int)MacroCommandID / 2;

	Red		= std::min(Red, 255);
	Blue	= std::min(Blue, 255);
	Green	= std::min(Green, 255);

	Header = ImColor(Red, Green, Blue);
}