export module Menu;

import std;
import SDL3;
import ResourceManager;
import General;
import UI;
import WorldState;
import Events;

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
		Event<const std::string&> on_action;
	};

	export class MenuManager : public HorizontalStack {
	private:
		std::shared_ptr<MenuItem> root_item_;
		std::shared_ptr<MenuItem> current_item_;
		std::shared_ptr<MenuItem> next_item_;

		float padding_ = 15;
		float item_width_ = 0;
		float item_height_ = 0;

		int num_columns_ = 3;
		int num_rows_ = 4;

		sdl::Color button_color_ = { 45, 52, 64, 255 };
		sdl::Color text_color_ = { 245, 245, 245, 255 };

		std::vector<std::shared_ptr<VerticalStack>> columns_;
		std::vector<Connection> button_connections_;

		void HandleButtonClick(UIElement* button) {
			if (button->id() == "back") {
				next_item_ = get_item_by_name(current_item_->parent->name);
				return;
			}

			auto item = get_item_by_name(button->id());
			// Check if it is an action item
			if (item->childItems.size() == 0) {
				item->on_action.emit(button->id());
			}
			else {
				next_item_ = item;
			}
		}

		std::shared_ptr<Padded<Button>> CreateButton(const std::string& name, const std::string& text, sdl::Keycode hotkey) {
			std::shared_ptr<Padded<Button>> button = std::make_shared<Padded<Button>>();
			button->set_pad_amount(5);
			button->set_text(text);
			button->set_id(name);
			button->set_background_color(button_color_);
			button->set_text_color(text_color_);
			button->set_hotkey(hotkey);
			button_connections_.push_back(button->on_action().subscribe(this, &MenuManager::HandleButtonClick));
			button->set_layout({ 0, LayoutMode::Fill, 0, LayoutMode::Fill });
			return button;
		}

		void UpdateButtons() {
			button_connections_.clear();
			for (auto column : columns_) {
				column->RemoveChildren();
			}

			int column_ = 0;
			int count = 0;
			for (auto item : current_item_->childItems) {
				auto button = CreateButton(item->name, item->text, item->hotkey);
				columns_[column_]->AddChild(button);
				count++;
				if (count == num_rows_) {
					column_++;
					count = 0;
				}
			}

			// If it's not the root item, add a back button
			if (current_item_->parent != nullptr) {
				auto back_button = CreateButton("back", "(B)ack", sdl::SDLK::B);
				columns_[column_]->AddChild(back_button);
			}
		}
	public:
		MenuManager() {
			root_item_ = std::make_shared<MenuItem>();
			root_item_->name = "root";
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

		const std::shared_ptr<MenuItem>& get_root_item() const {
			return root_item_;
		}

		void add_item(std::shared_ptr<MenuItem>& parent, std::shared_ptr<MenuItem>& child) {
			parent->childItems.push_back(child);
			child->parent = parent.get();
			UpdateButtons();
		}

		void Update(float passedTime) override {
			if (next_item_) {
				current_item_ = next_item_;
				UpdateButtons();
				next_item_.reset();
			}
		}

		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::Init(renderer, resources, state, controller);

			for (int i = 0; i < num_columns_; i++) {
				std::shared_ptr<VerticalStack> stack = std::make_shared<VerticalStack>();
				stack->set_layout({ 0, LayoutMode::Fill, 0, LayoutMode::Fill });
				AddChild(stack);
				columns_.push_back(stack);
			}

			UpdateButtons();
		}
	};
}