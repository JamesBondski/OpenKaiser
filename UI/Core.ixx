export module UI:Core;

import std;
import SDL3;
import WorldState;
import ResourceManager;
import General;
import GameController;
import Events;
import :Layout;

namespace OpenKaiser {
	export class UIElement {
	protected:
		sdl::FRect screen_area_;
		sdl::RendererPtr renderer_;
		std::shared_ptr<ResourceManager> resource_manager_;
		UIElement* parent_;
		std::shared_ptr<WorldState> state_;
		std::string id_;
		std::shared_ptr<GameController> controller_;
		Layout layout_;
		ScopedConnections handlers_;

	public:
		Layout& layout() {
			return layout_;
		}

		void set_layout(Layout layout) {
			layout_ = layout;
		}

		// This method receives the actual screen area that it occupies
		virtual void set_screen_area(sdl::FRect& screen_area) {
			screen_area_ = screen_area;
		}

		sdl::FRect& screen_area() {
			return screen_area_;
		}

		void set_parent(UIElement* parent) {
			parent_ = parent;
		}

		UIElement* parent() {
			return parent_;
		}

		virtual void set_world_state(std::shared_ptr<WorldState>& state) {
			state_ = state;
		}

		void set_id(const std::string& id) {
			id_ = id;
		}

		std::string& id() {
			return id_;
		}

		virtual void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) {
			renderer_ = renderer;
			resource_manager_ = resources;
			state_ = state;
			controller_ = controller;
		}

		virtual void Update(float passed_time) {
		}

		virtual void Draw() {
		}

		virtual void HandleEvent(sdl::Event& event) {
		}

		// Called whenever height or weight are set to LayoutMode::Auto to set its width/height
		virtual void UpdateLayout() {
		}
	};

	export class Container : public UIElement {
	protected:
		std::vector<std::shared_ptr<UIElement>> children_;

		float GetChildHeight(std::shared_ptr<UIElement>& child) const {
			switch (child->layout().height_mode) {
			case LayoutMode::Auto:
			case LayoutMode::Fixed:
				return child->layout().height;
			case LayoutMode::Percent:
				return screen_area_.h * child->layout().height / 100;
			case LayoutMode::Ratio:
				if (child->layout().width_mode != LayoutMode::Fixed) {
					throw OpenKaiserError("Width needs to be fixed to use LayoutMode::Ratio.");
				}
				return child->layout().width * child->layout().height;
			case LayoutMode::Fill:
				return 0;
			}
			return 0;
		}

		float GetChildWidth(std::shared_ptr<UIElement>& child) const {
			switch (child->layout().width_mode) {
			case LayoutMode::Fill:
				return 0;
			case LayoutMode::Auto:
			case LayoutMode::Fixed:
				return child->layout().width;
			case LayoutMode::Percent:
				return screen_area_.w * child->layout().width / 100;
			case LayoutMode::Ratio:
				if (child->layout().height_mode != LayoutMode::Fixed) {
					throw OpenKaiserError("Height needs to be fixed to use LayoutMode::Ratio.");
				}
				return child->layout().width * child->layout().height;
			}
			return 0;
		}
	public:
		virtual void Update(float passed_time) {
			for (auto element : children_) {
				element->Update(passed_time);
			}
		}

		virtual void Draw() {;
			for (auto element : children_) {
				sdl::FRect& area = element->screen_area();
				sdl::Rect clip_rect{ static_cast<int>(area.x), static_cast<int>(area.y), static_cast<int>(area.w), static_cast<int>(area.h) };
				sdl::set_render_clip_rect(renderer_, &clip_rect);
				element->Draw();
				sdl::set_render_clip_rect(renderer_, nullptr);
			}
		}

		virtual void HandleEvent(sdl::Event& event) {
			for (auto element : children_) {
				element->HandleEvent(event);
			}
		}

		void set_world_state(std::shared_ptr<WorldState>& state) {
			UIElement::set_world_state(state);
			for (auto element : children_) {
				element->set_world_state(state);
			}
		}

		template <std::derived_from<UIElement> T> void AddChild(std::shared_ptr<T>& element) {
			element->Init(renderer_, resource_manager_, state_, controller_);
			element->set_parent(this);
			children_.push_back(element);

			// Update children positions
			sdl::FRect own_pos = screen_area_;
			set_screen_area(own_pos);
		}

		void RemoveChildren() {
			children_.clear();
		}
	};

	export template <class T> requires std::derived_from<T, UIElement>
	class Padded : public T {
	private:
		float pad_amount_ = 0;
	public:
		float pad_amount() const {
			return pad_amount_;
		}

		void set_pad_amount(float new_amount) {
			pad_amount_ = new_amount;
		}

		void set_screen_area(sdl::FRect& screen_area) override {
			sdl::FRect padded_area{screen_area.x + pad_amount_, screen_area.y + pad_amount_, screen_area.w - 2 * pad_amount_, screen_area.h - 2 * pad_amount_};
			T::set_screen_area(padded_area);
		}
	};
}
