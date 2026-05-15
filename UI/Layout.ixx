export module UI:Layout;

import std;

namespace OpenKaiser {
	export enum class LayoutMode {
		Auto,
		Fill,
		Fixed,
		Percent,
		Ratio
	};

	export struct Layout {
		float width = 0;
		LayoutMode width_mode = LayoutMode::Fill;
		float height = 0;
		LayoutMode height_mode = LayoutMode::Fill;
	};
}
