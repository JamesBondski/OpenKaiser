export module UI;

import std;
import SDL3;
import WorldState;
import ResourceManager;
import General;
import GameController;

namespace OpenKaiser {
	export class UIElement {
	protected:
		sdl::FRect screen_area_;
		sdl::RendererPtr renderer_;
		std::shared_ptr<ResourceManager> resource_manager_;
		std::vector<std::shared_ptr<UIElement>> children_;
		UIElement* parent_;
		std::shared_ptr<WorldState> state_;
		std::string id_;
		std::shared_ptr<GameController> controller_;
		bool fill_ = false;

	public:
		virtual void set_screen_area(sdl::FRect& screen_area) {
			screen_area_ = screen_area;

			if (children_.size() == 1 && children_[0]->fill()) {
				sdl::FRect child_area{ 0, 0, screen_area_.w, screen_area_.h };
				children_[0]->set_screen_area(child_area);
			}
		}

		sdl::FRect& screen_area() {
			return screen_area_;
		}

		sdl::FRect GetOffsetArea(sdl::Point& offset) {
			return sdl::FRect{ screen_area_.x + offset.x, screen_area_.y + offset.y, screen_area_.w, screen_area_.h };
		}

		void set_fill(bool value) {
			fill_ = value;
		}

		bool fill() const {
			return fill_;
		}

		void set_parent(UIElement* parent) {
			parent_ = parent;
		}

		UIElement* parent() {
			return parent_;
		}

		void set_world_state(std::shared_ptr<WorldState>& state) {
			state_ = state;
			for (auto element : children_) {
				element->set_world_state(state);
			}
		}

		void set_id(const std::string& id) {
			id_ = id;
		}

		std::string& id() {
			return id_;
		}

		template <std::derived_from<UIElement> T> void AddChild(std::shared_ptr<T>& element) {
			element->Init(renderer_, resource_manager_, state_, controller_);
			element->set_parent(this);
			children_.push_back(element);
		}

		virtual void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) {
			renderer_ = renderer;
			resource_manager_ = resources;
			state_ = state;
			controller_ = controller;
		}

		virtual void Update(float passed_time) {
			for (auto element : children_) {
				element->Update(passed_time);
			}
		}

		virtual void Draw(sdl::Point& offset) {
				sdl::Point new_offset{ offset.x + static_cast<int>(screen_area_.x), offset.y + static_cast<int>(screen_area_.y) };
				for (auto element : children_) {
					sdl::Rect clip_rect{ new_offset.x + static_cast<int>(element->screen_area().x), new_offset.y + static_cast<int>(element->screen_area().y), static_cast<int>(element->screen_area().w), static_cast<int>(element->screen_area().h) };
				sdl::set_render_clip_rect(renderer_, &clip_rect);
				element->Draw(new_offset);
				sdl::set_render_clip_rect(renderer_, nullptr);
			}
		}
		virtual void HandleEvent(sdl::Event& event) {
			for (auto element : children_) {
				element->HandleEvent(event);
			}
		};
	};

	export class Padding : public UIElement {
	private:
		float pad_amount_;

	public:
		void set_screen_area(sdl::FRect& screen_area) override {
			UIElement::set_screen_area(screen_area);

			if (children_.size() > 1) {
				throw OpenKaiserError("Padding can only hold 1 element.");
			}

			if (children_.size() == 0) {
				return;
			}

			std::shared_ptr<UIElement>& child = children_[0];
			if (child->fill()) {
				throw OpenKaiserError("Elements within padding should not be set to fill.");
			}

			sdl::FRect child_area{ pad_amount_, pad_amount_, screen_area_.w - 2 * pad_amount_, screen_area_.h - 2 * pad_amount_ };
			child->set_screen_area(child_area);
		}

		float pad_amount() const {
			return pad_amount_;
		}

		void set_pad_amount(float new_amount) {
			pad_amount_ = new_amount;
		}
	};

	export class VerticalStack : public UIElement {
	private:
	public:
		void set_screen_area(sdl::FRect& screen_area) override {
			UIElement::set_screen_area(screen_area);

			sdl::FRect child_area = { 0, 0, screen_area_.w, 0 };
			for (std::shared_ptr<UIElement>& child : children_) {
				child_area.h = child->screen_area().h;
				child->set_screen_area(child_area);

				child_area.y += child_area.h;
			}
		}
	};

	export class TextElement : public UIElement {
	private:
		sdl::Color color_;
		std::string text_;
		float size_;
		bool centered_;

	public:
		TextElement(const std::string& text, float size, const sdl::Color& color, bool centered = false)
			: text_(text), size_(size), color_(color), centered_(centered) {
		}

		void Draw(sdl::Point& offset) override {
			auto texture = resource_manager_->get_text(text_, size_, color_.r, color_.g, color_.b);
			if (centered_) {
				sdl::FPoint middle{ offset.x + screen_area_.x + screen_area_.w / 2, offset.y + screen_area_.y + screen_area_.h / 2 };
				sdl::render_texture_centered(renderer_, texture, middle);
			}
			else {
				sdl::FRect output_area{ offset.x + screen_area_.x, offset.y + screen_area_.y, static_cast<float>(texture->w), static_cast<float>(texture->h) };
				sdl::render_texture(renderer_, texture, output_area);
			}
		}

		void set_color(const sdl::Color& color) {
			color_ = color;
		}

		sdl::Color& color() {
			return color_;
		}

		void set_text(const std::string& text) {
			text_ = text;
		}

		std::string& text() {
			return text_;
		}

		void set_size(float size) {
			size_ = size;
		}

		float size() const {
			return size_;
		}

		void set_centered(bool centered) {
			centered_ = centered;
		}

		bool IsCentered() const {
			return centered_;
		}

	};

	export class DynamicTextElement : public TextElement {
	private:
		std::function<std::string()> text_getter_;
	public:
		DynamicTextElement(std::function<std::string()>& text_getter, float size, const sdl::Color& color, bool centered = false)
			: TextElement("", size, color, centered) {
			text_getter_ = text_getter;
		}

		void Draw(sdl::Point& offset) override {
			set_text(text_getter_());
			TextElement::Draw(offset);
		}
	};

	export class GameState : public UIElement {
	protected:
		std::string next_state_;

	public:

		std::string& next_state() {
			return next_state_;
		}

		void set_next_state(const std::string& state_name) {
			next_state_ = state_name;
		}

		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::Init(renderer, resources, state, controller);

			int width, height;
			sdl::get_current_render_output_size(renderer, &width, &height);
			sdl::FRect own_area{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) };
			set_screen_area(own_area);
		}

		void HandleEvent(sdl::Event& event) override {
			if (event.type == sdl::EventType::WindowResized) {
				sdl::FRect own_area{ 0.0f, 0.0f, static_cast<float>(event.window.data1), static_cast<float>(event.window.data2) };
				set_screen_area(own_area);
			}
			UIElement::HandleEvent(event);
		}
	};
}
