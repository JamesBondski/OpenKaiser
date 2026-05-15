export module UI:Stacks;

import std;
import SDL3;
import :Core;
import :Layout;

namespace OpenKaiser {
	export class HorizontalStack : public Container {
	public:
		void set_screen_area(sdl::FRect& screen_area) override {
			UIElement::set_screen_area(screen_area);

			// Update layout for LayoutMode::Auto elements and calculate total width
			// Also count the number of fill elements
			float total_width = 0;
			float fill_count = 0;
			for (std::shared_ptr<UIElement>& child : children_) {
				if (child->layout().height_mode == LayoutMode::Auto || child->layout().width_mode == LayoutMode::Auto) {
					child->UpdateLayout();
				}

				float child_width = GetChildWidth(child);
				fill_count += child_width == 0 ? 1 : 0;
				total_width += child_width;
			}

			float fill_width = (fill_count > 0)
				? std::max(0.0f, (screen_area_.w - total_width) / fill_count)
				: 0.0f;

			sdl::FRect child_area = { screen_area_.x, screen_area_.y, 0, screen_area_.h };
			for (std::shared_ptr<UIElement>& child : children_) {
				Layout& child_layout = child->layout();
				child_area.w = child_layout.width_mode == LayoutMode::Fill ? fill_width : GetChildWidth(child);
				child_area.h = child_layout.height_mode == LayoutMode::Fill ? screen_area_.h : GetChildHeight(child);

				child->set_screen_area(child_area);
				child_area.x += child_area.w;
			}
		}
	};

	export class VerticalStack : public Container {
	private:
	public:
		void set_screen_area(sdl::FRect& screen_area) override {
			UIElement::set_screen_area(screen_area);

			// Update layout for LayoutMode::Auto elements and calculate total height
			// Also count the number of fill elements
			float total_height = 0;
			float fill_count = 0;
			for (std::shared_ptr<UIElement>& child : children_) {
				if (child->layout().height_mode == LayoutMode::Auto || child->layout().width_mode == LayoutMode::Auto) {
					child->UpdateLayout();
				}

				float child_height = GetChildHeight(child);
				fill_count += child_height == 0 ? 1 : 0;
				total_height += child_height;
			}

			float fill_height = (fill_count > 0)
				? std::max(0.0f, (screen_area_.h - total_height) / fill_count)
				: 0.0f;

			sdl::FRect child_area = { screen_area_.x, screen_area_.y, screen_area_.w, 0 };
			for (std::shared_ptr<UIElement>& child : children_) {
				Layout& child_layout = child->layout();
				child_area.h = child_layout.height_mode == LayoutMode::Fill ? fill_height : GetChildHeight(child);
				child_area.w = child_layout.width_mode == LayoutMode::Fill ? screen_area_.w : GetChildWidth(child);

				child->set_screen_area(child_area);
				child_area.y += child_area.h;
			}
		}
	};
}
