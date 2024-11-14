﻿#include "stdafx.h"
#include "UIMinimapEditorForm.h"
UIMinimapEditorForm*    UIMinimapEditorForm::Form = nullptr;

UIMinimapEditorForm::UIMinimapEditorForm()
{
	m_TextureRemove = nullptr;
	m_BackgroundTexture = nullptr;

	isEdited = false;
	ActiveFile.clear();
}

UIMinimapEditorForm::~UIMinimapEditorForm()
{
	for (auto& element : elements)
		if (element.texture) element.texture->Release();

	if (m_BackgroundTexture)m_BackgroundTexture->Release();
	selectedElement = nullptr;

	elements.clear();
}
void UIMinimapEditorForm::UnselectAllElements()
{
	for (auto& other_element : elements) {
		other_element.selected = false;
	}
	selectedElement = nullptr;
}
void UIMinimapEditorForm::SelectElement(Element& el)
{
	UnselectAllElements();

	el.selected = true;
	selectedElement = &el;
}

void UIMinimapEditorForm::ShowElementList() {

	ImGui::Text(" Elements");
	ImGui::Separator();

	for (int i = 0; i < elements.size(); i++) 
	{
		ImGui::PushID(i);
		bool is_selected = elements[i].selected;
		if (ImGui::Selectable(elements[i].name.c_str(), is_selected)) 
		{
			SelectElement(elements[i]);
		}
		ImGui::PopID();
	}
}

void UIMinimapEditorForm::RenderCanvas() 
{
	ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
	ImVec2 canvas_size = ImGui::GetContentRegionAvail();

	if (canvas_size.x < 64 && canvas_size.y < 64)
		return;

	ImGui::InvisibleButton("canvas", canvas_size);
	bool is_hovered = ImGui::IsItemHovered();
	bool is_mapItemHovered = false;

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
		if (!isDragging) {
			isDragging = true;
			initial_mouse_pos = ImGui::GetMousePos();
			initial_bg_position = m_BackgroundPosition;
		}
		ImVec2 mouse_delta = ImGui::GetMousePos() - initial_mouse_pos;
		m_BackgroundPosition = initial_bg_position + mouse_delta;
	}
	else {
		isDragging = false;
	}

	if (is_hovered) {
		float scroll = ImGui::GetIO().MouseWheel;
		if (scroll != 0.0f) {
			float old_zoom = m_Zoom;
			m_Zoom = Clamp(m_Zoom + scroll * 0.1f, 0.1f, 5.0f);

			ImVec2 mouse_pos = ImGui::GetIO().MousePos;
			m_BackgroundPosition.x += (mouse_pos.x - canvas_p0.x - m_BackgroundPosition.x) * (1 - m_Zoom / old_zoom);
			m_BackgroundPosition.y += (mouse_pos.y - canvas_p0.y - m_BackgroundPosition.y) * (1 - m_Zoom / old_zoom);
		}
	}

	ImVec2 bg_display_size(m_ImageW * m_Zoom, m_ImageH * m_Zoom);
	ImGui::GetWindowDrawList()->AddImage(m_BackgroundTexture, canvas_p0 + m_BackgroundPosition, canvas_p0 + m_BackgroundPosition + bg_display_size);

	
	for (int i = 0; i < elements.size(); i++) {
		auto& element = elements[i];

		ImVec2 element_screen_pos = canvas_p0 + m_BackgroundPosition + element.position * m_Zoom;
		ImVec2 element_screen_size = element.RenderSize;
		element_screen_size *= m_Zoom;
		float handle_size = 5.0f * m_Zoom;

		ImGui::GetWindowDrawList()->AddImage(element.texture, element_screen_pos, element_screen_pos + element_screen_size, ImVec2(0,0), ImVec2(1, 1), IM_COL32(255, 255, 255, m_ItemsOpacity));

		if (element.selected || m_AlwaysDrawBorder)
		{
			ImU32 col = (element.selected) ? IM_COL32(200, 255, 200, 255) : IM_COL32(255, 255, 255, 100);

			ImVec2 top_left = element_screen_pos - ImVec2(handle_size / 2, handle_size / 2);
			ImVec2 bottom_right = element_screen_pos + ImVec2(handle_size / 2, handle_size / 2) + element_screen_size;
			ImGui::GetWindowDrawList()->AddRect(top_left, bottom_right, col, 0.0f, ImDrawFlags_RoundCornersNone, 2.0f);
		}

		ImVec2 image_min = element_screen_pos;
		ImVec2 image_max = element_screen_pos + element_screen_size;
		bool inside = (ImGui::GetIO().MousePos.x >= image_min.x && ImGui::GetIO().MousePos.x <= image_max.x &&
			ImGui::GetIO().MousePos.y >= image_min.y && ImGui::GetIO().MousePos.y <= image_max.y);

		if (inside)
			is_mapItemHovered = inside;

		if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
		{
			if (inside) 
			{
				SelectElement(element);
			}
		}

		if (element.selected) 
		{
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) 
			{
				element.position.y--;
				isEdited = true;
			}
			else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
			{
				element.position.y++;
				isEdited = true;
			}

			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) 
			{
				element.position.x--;
				isEdited = true;
			}
			else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) 
			{
				element.position.x++;
				isEdited = true;
			}

			if (is_hovered && inside && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
				ImVec2 mouse_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

				element.position.x += mouse_delta.x / m_Zoom;
				element.position.y += mouse_delta.y / m_Zoom;

				ImGui::ResetMouseDragDelta();

				if(!isEdited)
					isEdited = true;
			}
		}
	}

	if (is_hovered && !is_mapItemHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		UnselectAllElements();
	}
}

void UIMinimapEditorForm::ShowPreview()
{
	if (!PreviewMode || !ImGui::Begin("Preview", &PreviewMode, ImGuiWindowFlags_NoCollapse))
		return;

	ImVec2 WSize = ImGui::GetContentRegionAvail();

	xr_string data;
	string256 buff;

	data += "[level_maps_single]\n";
	for (auto element : elements)
	{
		sprintf(buff, "%s    =\n", element.name.c_str());
		data += buff;
	}
	data += "\n\n";

	for (auto element : elements)
	{
		sprintf(buff, "[%s]\n        global_rect    = %.2f, %.2f, %.2f, %.2f\n", element.name.c_str(),element.position.x, 
			element.position.y, element.position.x + element.RenderSize.x, element.position.y + element.RenderSize.y);
		data += buff;
	}

	ImGui::InputTextMultiline(" ", data.data(), 2048*2, WSize, ImGuiInputTextFlags_ReadOnly);

	ImGui::End();
}



void UIMinimapEditorForm::ShowMenu() 
{
	ImGui::Text((isEdited) ? "[Edited]" :" ");
	ImGui::SameLine();

	ImGui::Separator();
	if (ImGui::BeginMenu("File"))
	{
		string_path out_activefile;

		sprintf(out_activefile, "Active File: %s", (ActiveFile.empty()) ? "None" : ActiveFileShort.c_str());

		ImGui::BeginDisabled(ActiveFile.empty());
		ImGui::Text(out_activefile);
		ImGui::Separator();
		if (ImGui::MenuItem("Save"))
		{
			SaveFile(true);
		}
		ImGui::EndDisabled();

		if (ImGui::MenuItem("Save As"))
		{
			SaveFile(false);
		}

		if (ImGui::MenuItem("Open"))
		{
			OpenFile();
		}
		ImGui::EndMenu();
	}
	
	if (ImGui::MenuItem("Set Map Background"))
	{
		LoadBGClick();
	}
	if (ImGui::MenuItem("Add Location"))
	{
		Element test;
		//test.name = "Test";
		test.texture = nullptr;
		test.position = ImVec2(0, 0);
		test.FileSize = ImVec2(100, 100);

		int res = LoadTexture(test);

		if (res == 2)
		{
			u32 mem = 0;
			test.texture = RImplementation.texture_load("ed\\ed_nodata", mem);
		}

		if (res != 1)
		{
			test.RenderSize = test.FileSize;
			elements.push_back(test);
			isEdited = true;
			SelectElement(elements[elements.size() - 1]);
		}
	}
	if (ImGui::MenuItem("Reset zoom&pos")) 
	{
		m_Zoom = 1.f;
		m_BackgroundPosition.x = 0;
		m_BackgroundPosition.y = 0;
	}
	ImGui::Checkbox("Preview", &PreviewMode);
	ImGui::Checkbox("Always Draw Border", &m_AlwaysDrawBorder);

	ImGui::Text("Items Opacity:");
	ImGui::DragInt("##op", &m_ItemsOpacity, 1.0f, 1, 255.0f);
	ImGui::Separator();

	if (selectedElement)
	{
		ImGui::Text("Selected item:");
		/*ImGui::Text("Name:");
		if (ImGui::InputText("##", (*selectedElement).name.data(), 255))
		{
			isEdited = true;
		}*/

		ImGui::Text("Texture:");
		ImGui::SameLine();
		if (ImGui::Button("Change"))
		{
			Element copy = (*selectedElement);
			if (LoadTexture(*selectedElement) == 0)
			{
				(*selectedElement).RenderSize = copy.RenderSize;
				//(*selectedElement).name = copy.name;
				isEdited = true;
			}
			
		}

		ImGui::Text("Position:");
		if (ImGui::DragFloat2("##position", (float*)&(*selectedElement).position, 1.0f))
		{
			if (!isEdited) isEdited = true;
		}
		ImGui::Text("Size:");
		if (ImGui::DragFloat2("##size", (float*)&(*selectedElement).RenderSize, 1.0f, 1.0f, 1024))
		{
			if (!isEdited) isEdited = true;
		}
		
		if (ImGui::Button("Delete"))
		{
			auto it = elements.begin();
			for (; it != elements.end(); ++it) {
				if (&(*it) == selectedElement) {
					if (it->texture)
						it->texture->Release();
					it->texture = nullptr;

					elements.erase(it);
					selectedElement = nullptr;
					break;
				}
			}
			isEdited = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset size"))
		{
			(*selectedElement).RenderSize = (*selectedElement).FileSize;
			isEdited = true;
		}
	}
}

void UIMinimapEditorForm::OpenFile()
{
	xr_string fn;
	if (!EFS.GetOpenName("$game_config$", fn, false, NULL, 0, "*.ltx"))
	{
		return;
	}
	elements.clear();

	ActiveFile = fn.c_str();
	ActiveFileShort = FS.fix_path(fn.c_str());

	FS.TryLoad(fn.c_str());

	auto ltxFile = new CInifile(fn.c_str(), TRUE);

	xr_vector<shared_str> levels;

	
	if ((*ltxFile).section_exist("global_map"))
	{
		if ((*ltxFile).line_exist("global_map", "texture"))
		{
			auto tx = (*ltxFile).r_string("global_map", "texture");

			string_path texturePath;
			sprintf(texturePath, "%s%s.dds", FS.get_path("$game_textures$")->m_Path, tx);
			LoadBGClick(texturePath);
		}

	}

	{
		//GetLevels

		if (!(*ltxFile).section_exist("level_maps_single"))
		{
			Msg("! !!!");
			return;
		}
		CInifile::Sect& S = (*ltxFile).r_section("level_maps_single");
		CInifile::SectCIt it = S.Data.begin();
		CInifile::SectCIt it_e = S.Data.end();

		for (; it != it_e; ++it)
		{
			levels.push_back(it->first);
		}
	}

	for (auto& levelName : levels)
	{
		CInifile::Sect& S = (*ltxFile).r_section(levelName);
		CInifile::SectCIt it = S.Data.begin();
		CInifile::SectCIt it_e = S.Data.end();

		if (!(*ltxFile).line_exist(levelName, "global_rect"))
			continue;
		
		Fvector4 tmp = (ltxFile)->r_fvector4(levelName, "global_rect");

		Element el;
		el.name = levelName.c_str();
		el.texture = nullptr;
		el.selected = false;
		el.position.x = tmp.x;
		el.position.y = tmp.y;

		el.RenderSize.x = tmp.z - tmp.x;
		el.RenderSize.y = tmp.w - tmp.y;

		string_path texturePath;
		sprintf(texturePath, "%smap\\map_%s.dds", FS.get_path("$game_textures$")->m_Path, levelName.c_str());

		LoadTexture(el, texturePath);
		elements.push_back(el);
	}

	xr_delete(ltxFile);
	isEdited = false;
}

void UIMinimapEditorForm::SaveFile(bool saveCurrent)
{

	xr_string fn;
	if (saveCurrent)
	{
		fn = ActiveFile;
	}
	else if (!EFS.GetSaveName("$game_data$", fn, NULL, 0, "*.ltx"))
	{
		return;
	}

	if (!saveCurrent) 
	{
		ActiveFile = fn.c_str();
		ActiveFileShort = FS.fix_path(fn.c_str());
	}

	FS.TryLoad(fn.c_str());
	auto ltxFile = new CInifile(fn.c_str(), FALSE, TRUE, 1);

	{
		(*ltxFile).remove_line("global_map", "texture");
		(*ltxFile).remove_line("global_map", "bound_rect");

		(*ltxFile).w_string("global_map", "texture", m_BackgroundTexturePath.c_str());
		(*ltxFile).w_fvector4("global_map", "bound_rect", Fvector4(0.f, 0.f, m_ImageW, m_ImageH));
	}

	for (auto& element : elements)
	{
		(*ltxFile).remove_line("level_maps_single", element.name.c_str());
		(*ltxFile).w_string("level_maps_single", element.name.c_str(), "");
		//
		(*ltxFile).remove_line(element.name.c_str(), "global_rect");
		auto dat = Fvector4(element.position.x, element.position.y, element.RenderSize.x+ element.position.x, element.RenderSize.y + element.position.y);
		(*ltxFile).w_fvector4(element.name.c_str(), "global_rect", dat);
	}

	xr_delete(ltxFile);
	isEdited = false;
}

void UIMinimapEditorForm::Draw()
{
	if (m_TextureRemove)
	{
		m_TextureRemove->Release();
		m_TextureRemove = nullptr;
	}
	if (m_BackgroundTexture == nullptr)
	{
		u32 mem = 0;
		m_BackgroundTexture = RImplementation.texture_load("ui\\ui_nomap2", mem);
		m_BackgroundTexturePath = "ui\\ui_nomap2";
	}

	{
		ImGui::Columns(2, "mapEditor", false);

		float leftColumnWidth = 300.0f;
		ImGui::SetColumnWidth(0, leftColumnWidth);

		ImVec2 windowSize = ImGui::GetContentRegionAvail();

		float menuSizeY = 440.f;
		ImVec2 elementsListSize = ImVec2(windowSize.x, windowSize.y - menuSizeY);
		ImVec2 menuSize = ImVec2(windowSize.x, menuSizeY - 10.f);

		ImGui::BeginChild("Elements List", elementsListSize, true);
		ShowElementList();
		ImGui::EndChild();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY());
		ImGui::BeginChild("Menu", menuSize, true);
		ShowMenu();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("Canvas");
		RenderCanvas();
		ImGui::EndChild();
	}
	
	
	ShowPreview();
}

void UIMinimapEditorForm::Update()
{
	if (Form)
	{
		if (!Form->IsClosed())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(500, 500));
			if (ImGui::BeginPopupModal("MinimapEditor", &Form->bOpen, 0, true))
			{
				Form->Draw();
				ImGui::EndPopup();
			}
			else
			{
				Form->bOpen = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleVar();
		}
		else
		{
			xr_delete(Form);
		}

	}
}

void UIMinimapEditorForm::Show()
{
	VERIFY(!Form);
	Form = new UIMinimapEditorForm();
	Form->bOpen = true;
}

extern bool Stbi_Load(LPCSTR full_name, U32Vec& data, u32& w, u32& h, u32& a);

int UIMinimapEditorForm::LoadTexture(Element& el, const xr_string texture)
{
	xr_string fn;
	m_ImageData.clear();

	if (texture != "")
	{
		fn = texture;
	}
	else if (!EFS.GetOpenName("$game_textures$", fn, false, NULL, 0, "*.dds; *.tga"))
	{
		return 1;
	}
	
	u32 W, H, A;
	const xr_string prefix = "map_";

	if (!Stbi_Load(fn.c_str(), m_ImageData, W, H, A))
	{
		return 2;
	}

	if (el.texture)
		el.texture->Release();
	el.texture = nullptr;

	el.path = fn;
	el.FileSize.x = W;
	el.FileSize.y = H;

	// map_somename -> somename
	auto ck_map = xr_path(fn).stem().string();
	el.name = (ck_map.find(prefix) == 0) ? ck_map.substr(prefix.size()).c_str() : xr_path(fn).stem().string().c_str();

	ID3DTexture2D* pTexture = nullptr;
	{
		R_CHK(REDevice->CreateTexture(W, H, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture, 0));
		el.texture = pTexture;
		{
			D3DLOCKED_RECT rect;
			R_CHK(pTexture->LockRect(0, &rect, 0, D3DLOCK_DISCARD));
			for (int i = 0; i < H; i++)
			{

				unsigned char* dest = static_cast<unsigned char*>(rect.pBits) + (rect.Pitch * i);
				memcpy(dest, m_ImageData.data() + (W * i), sizeof(unsigned char) * W * 4);
			}
			R_CHK(pTexture->UnlockRect(0));
		}
	}

	return 0;
}

void UIMinimapEditorForm::LoadBGClick(const xr_string texture)
{
	xr_string fn;
	m_ImageData.clear();

	if (texture != "")
	{
		fn = texture;
	}
	else if (!EFS.GetOpenName("$game_textures$", fn, false, NULL, 0, "*.dds"))
	{
		return;
	}

	if (texture == "")
		fn = FS.fix_path(fn.c_str());

	if (Stbi_Load(fn.c_str(), m_ImageData, m_ImageW, m_ImageH, m_ImageA))
	{
		isEdited = true;

		xr_string game_texture_dir = FS.get_path("$game_textures$")->m_Path;

		m_BackgroundTexturePath = (fn.find(game_texture_dir) == 0) ? fn.substr(game_texture_dir.size()) : fn;
		
		if (auto p = xr_path(m_BackgroundTexturePath); p.has_extension())
		{
			p.replace_extension("");
			m_BackgroundTexturePath = p;
		}

		m_TextureRemove = m_BackgroundTexture;
		ID3DTexture2D* pTexture = nullptr;
		{
			R_CHK(REDevice->CreateTexture(m_ImageW, m_ImageH, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pTexture, 0));
			m_BackgroundTexture = pTexture;

			{
				D3DLOCKED_RECT rect;
				R_CHK(pTexture->LockRect(0, &rect, 0, D3DLOCK_DISCARD));
				for (int i = 0; i < m_ImageH; i++)
				{

					unsigned char* dest = static_cast<unsigned char*>(rect.pBits) + (rect.Pitch * i);
					memcpy(dest, m_ImageData.data() + (m_ImageW * i), sizeof(unsigned char) * m_ImageW * 4);
				}
				R_CHK(pTexture->UnlockRect(0));
			}
		}
	}
	
}
