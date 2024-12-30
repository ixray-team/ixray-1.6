#pragma once
#include "../../xrEProps/NodeBuilder/MacroNodes.h"
#include "../../xrEProps/NodeBuilder/Builder.h"

struct MacroNodeTypes
{
	xr_string RegName;
	size_t CommandID;
	MacroType Type;
};

class CUIMacroView:
	public CNodeViewport
{
	bool IsOpen = false;
	xr_vector<MacroNodeTypes> CommandsList;

public:
	CUIMacroView();
	~CUIMacroView();

	virtual void Draw() override;
	void Show(bool State);
	void Exec();

private:
	void ExecMacro(CNodeMacro* Node);
};