export module Menu;

import std;
import SDL3;
import ResourceManager;
import General;
import UI;
import WorldState;

namespace OpenKaiser {
	export enum class ResultAction {
		None,
		Back
	};

	export struct MenuItem {
		std::string name;
		std::string text;
		sdl::Keycode hotkey;
		std::vector<std::shared_ptr<MenuItem>> childItems;
		MenuItem* parent;
		std::function<ResultAction(const std::string)> callback;
	};

	export class MenuManager : public UIElement {
	private:
		std::shared_ptr<MenuItem> root_item_;
		std::shared_ptr<MenuItem> current_item_;

		float padding_ = 15;
		float item_width_ = 0;
		float item_height_ = 0;

		int num_columns_ = 3;
		int num_rows_ = 4;

		sdl::Color button_color_ = { 45, 52, 64, 255 };
		sdl::Color text_color_ = { 245, 245, 245, 255 };
	public:
		MenuManager() {
			root_item_ = std::make_shared<MenuItem>();
			current_item_ = root_item_;
		}

		std::shared_ptr<MenuItem> get_item_by_name(const std::string& name) {
				std::stack<std::shared_ptr<MenuItem>> search_list;
				search_list.push(root_item_);

				while (!search_list.empty()) {
					std::shared_ptr<MenuItem>& item = search_list.top();
					search_list.pop();

					if (item->name == name) {
						return item;
					}

					for (auto& child_item : item->childItems) {
						search_list.push(child_item);
					}
				}
				return nullptr;
			}

		std::shared_ptr<MenuItem>& get_root_item() {
				return root_item_;
			}

		void set_screen_area(sdl::FRect& screen_area) override {
				UIElement::set_screen_area(screen_area);

				item_height_ = (screen_area.h - (num_rows_ + 1) * padding_) / num_rows_;
				item_width_ = (screen_area.w - (num_columns_ + 1) * padding_) / num_columns_;
			}


		void add_item(std::shared_ptr<MenuItem>& parent, std::shared_ptr<MenuItem>& child) {
				parent->childItems.push_back(child);
				child->parent = parent.get();
			}

		void Draw(sdl::Point& offset) override {
			sdl::set_render_draw_color(renderer_, button_color_);

			int item_count = 0;
			for (int col = 0; col < num_columns_; col++) {
				for (int row = 0; row < num_rows_; row++) {
					if (item_count < current_item_->childItems.size()) {
						auto item = current_item_->childItems[item_count];
						sdl::FRect item_area{
							offset.x + screen_area_.x + padding_ + (padding_ + item_width_) * col,
							offset.y + screen_area_.y + padding_ + (padding_ + item_height_) * row,
							item_width_,
							item_height_
						};
						sdl::render_fill_rect(renderer_, item_area);

						auto texture = resource_manager_->get_text(item->text, 16, text_color_.r, text_color_.g, text_color_.b);
						sdl::FPoint mid = { item_area.x + item_area.w / 2, item_area.y + item_area.h / 2 };
						sdl::render_texture_centered(renderer_, texture, mid);
						item_count++;
					}
				}
			}
		}
		void HandleEvent(sdl::Event& event) override {
			if (event.type == sdl::EventType::KeyDown) {
				for (auto child : current_item_->childItems) {
					if (event.key.key == child->hotkey) {
						auto result = child->callback(child->name);
						if (result == ResultAction::Back) {
							current_item_ = get_item_by_name(current_item_->parent->name);
						}
					}
				}
			}
		}

	};
}