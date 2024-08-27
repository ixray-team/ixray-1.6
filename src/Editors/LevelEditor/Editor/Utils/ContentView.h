#pragma once

class CContentView:
	public XrUI
{
	struct DragDropData 
	{
		xr_string FileName;
	};

	struct IconData {
		ref_texture Icon;
		bool UseButtonColor = false;
	};

	DragDropData Data;
public:
	CContentView();
	virtual void Draw() override;
	virtual void Init();
	virtual void Destroy();
	virtual void ResetBegin();
	virtual void ResetEnd();

private:
	bool DrawItem(const xr_string& FilePath, size_t& HorBtnIter, const size_t IterCount);
	bool DrawContext(const std::filesystem::path& Path) const;
	IconData& GetTexture(const xr_string& IconPath);

	struct HintItem
	{
		xr_string Name;
		ImVec2 Pos;
		bool Active = false;
	};

	HintItem CurrentItemHint;

private:
	xr_string CurrentDir;
	xr_string RootDir;
	xr_string LogsDir;
	ImVec2 BtnSize = { 64, 64 };

	xr_hash_map<xr_string, IconData> Icons;
};