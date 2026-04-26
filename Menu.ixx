export module Menu;

import std;
import SDL3;
import ResourceManager;
import General;
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
		std::shared_ptr<MenuItem> rootItem;
		std::shared_ptr<MenuItem> currentItem;

		float padding = 15;
		float itemWidth = 0;
		float itemHeight = 0;

		int numColumns = 3;
		int numRows = 4;

		sdl::Color buttonColor = { 45, 52, 64, 255 };
		sdl::Color textColor = { 245, 245, 245, 255 };
	public:
		MenuManager() {
			this->rootItem = std::make_shared<MenuItem>();
			this->currentItem = this->rootItem;
		}

		std::shared_ptr<MenuItem> getItemByName(const std::string& name) {
			std::stack<std::shared_ptr<MenuItem>> searchList;
			searchList.push(this->rootItem);

			while (!searchList.empty()) {
				std::shared_ptr<MenuItem>& item = searchList.top();
				searchList.pop();

				if (item->name == name) {
					return item;
				}
				
				for (auto& childItem : item->childItems) {
					searchList.push(childItem);
				}
			}
			return nullptr;
		}

		std::shared_ptr<MenuItem>& getRootItem() {
			return this->rootItem;
		}

		void set_screen_area(sdl::FRect& screenArea) override {
			UIElement::set_screen_area(screenArea);

			this->itemHeight = (screenArea.h - (numRows + 1) * padding) / numRows;
			this->itemWidth = (screenArea.w - (numColumns + 1) * padding) / this->numColumns;
		}


		void add_item(std::shared_ptr<MenuItem>& parent, std::shared_ptr<MenuItem>& child) {
			parent->childItems.push_back(child);
			child->parent = parent.get();
		}

		void draw() override {
			sdl::set_render_draw_color(this->renderer, this->buttonColor);

			int itemCount = 0;
			for (int col = 0; col < this->numColumns; col++) {
				for (int row = 0; row < this->numRows; row++) {
					if (itemCount < this->currentItem->childItems.size()) {
						auto item = this->currentItem->childItems[itemCount];
						sdl::FRect itemArea{
							this->screenArea.x + this->padding + (this->padding + this->itemWidth) * col,
							this->screenArea.y + this->padding + (this->padding + this->itemHeight) * row,
							this->itemWidth,
							this->itemHeight
						};
						sdl::render_fill_rect(this->renderer, itemArea);

						auto texture = this->resourceManager->get_text(item->text, 16, this->textColor.r, this->textColor.g, this->textColor.b);
						sdl::FPoint mid = { itemArea.x + itemArea.w / 2, itemArea.y + itemArea.h / 2 };
						sdl::render_texture_centered(this->renderer, texture, mid);
						itemCount++;
					}
				}
			}
		}
		void handle_event(sdl::Event& event) override {
			if (event.type == sdl::EventType::KeyDown) {
				for (auto child : this->currentItem->childItems) {
					if (event.key.key == child->hotkey) {
						auto result = child->callback(child->name);
						if (result == ResultAction::Back) {
							this->currentItem = this->getItemByName(this->currentItem->parent->name);
						}
					}
				}
			}
		}

	};
}