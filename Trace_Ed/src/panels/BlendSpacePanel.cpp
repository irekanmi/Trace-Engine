
#include "BlendSpacePanel.h"

namespace trace {



	void BlendSpacePanel::AddSample(const std::string& name, float x, float y)
	{
		samples.push_back({ name, x, y });
		//RebuildTriangulation();
	}

	BlendSpacePanel::BlendSpacePanel(const std::string& title, const std::string& xLabel, const std::string& yLabel)
		: title(title), xAxisLabel(xLabel), yAxisLabel(yLabel)
	{


	}

}