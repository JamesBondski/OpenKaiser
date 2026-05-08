export module UI;

import std;
import SDL3;
import WorldState;
import ResourceManager;
import General;
import GameController;

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
		}
	};

	export class Image : public UIElement {
	protected:
		sdl::TexturePtr texture_;
	public:
		sdl::TexturePtr& texture() {
			return texture_;
		}

		void set_texture(sdl::TexturePtr& texture) {
			texture_ = texture;
		}

		void Draw() override {
			sdl::render_texture(renderer_, texture_, screen_area_);
		}

		void UpdateLayout() override {
			UIElement::UpdateLayout();
			if (layout_.width_mode == LayoutMode::Auto) {
				layout_.width = static_cast<float>(texture_->w);
			}
			if (layout_.height_mode == LayoutMode::Auto) {
				layout_.height = static_cast<float>(texture_->h);
			}
		}
	};

	export class Padding : public Container {
	private:
		float pad_amount_ = 0;

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

			sdl::FRect child_area{ screen_area_.x + pad_amount_, screen_area_.y + pad_amount_, screen_area_.w - 2 * pad_amount_, screen_area_.h - 2 * pad_amount_ };
			child->set_screen_area(child_area);
		}

		float pad_amount() const {
			return pad_amount_;
		}

		void set_pad_amount(float new_amount) {
			pad_amount_ = new_amount;
		}
	};

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

		void Draw() override {
			auto texture = resource_manager_->get_text(text_, size_, color_.r, color_.g, color_.b);
			if (centered_) {
				sdl::FPoint middle{ screen_area_.x + screen_area_.w / 2,screen_area_.y + screen_area_.h / 2 };
				sdl::render_texture_centered(renderer_, texture, middle);
			}
			else {
				sdl::FRect output_area{ screen_area_.x, screen_area_.y, static_cast<float>(texture->w), static_cast<float>(texture->h) };
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

		void UpdateLayout() override {
			auto texture = resource_manager_->get_text(text_, size_, color_.r, color_.g, color_.b);
			if (layout_.width_mode == LayoutMode::Auto) {
				layout_.width = texture->w;
			}

			if (layout_.height_mode == LayoutMode::Auto) {
				layout_.height = texture->h;
			}
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

		void Draw() override {
			set_text(text_getter_());
			TextElement::Draw();
		}
	};

	export class GameState : public Padding {
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
			Padding::Init(renderer, resources, state, controller);

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
			Padding::HandleEvent(event);
		}
	};
}
