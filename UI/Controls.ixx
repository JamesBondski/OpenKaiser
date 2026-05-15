export module UI:Controls;

import std;
import SDL3;
import ResourceManager;
import Events;
import :Core;

namespace OpenKaiser {
	export class Button : public UIElement {
	private:
		std::string text_;
		sdl::Keycode hotkey_;
		sdl::Color background_color_;
		sdl::Color text_color_;
		float text_size_ = 16.0f;
		Event<UIElement*> on_action_;

		sdl::TexturePtr text_texture_;
		sdl::FPoint mid_point_;

		void UpdateTexture() {
			if (text_.empty()) {
				text_texture_.reset();
			}
			else {
				if (!renderer_) {
					return;
				}

				text_texture_ = resource_manager_->get_text(text_, text_size_, text_color_);
			}
		}
	public:
		Event<UIElement*>& on_action() {
			return on_action_;
		}

		const std::string& text() const {
			return text_;
		}

		void set_text(const std::string& text) {
			text_ = text;
			UpdateTexture();
		}

		const sdl::Keycode hotkey() const {
			return hotkey_;
		}

		void set_hotkey(sdl::Keycode hotkey) {
			hotkey_ = hotkey;
		}

		const sdl::Color& background_color() const {
			return background_color_;
		}

		void set_background_color(const sdl::Color& color) {
			background_color_ = color;
		}

		const sdl::Color& text_color() const {
			return text_color_;
		}

		void set_text_color(const sdl::Color& color) {
			text_color_ = color;
		}

		float text_size() const {
			return text_size_;
		}

		void set_text_size(float size) {
			text_size_ = size;
			UpdateTexture();
		}

		void set_screen_area(sdl::FRect& rect) override {
			UIElement::set_screen_area(rect);
			mid_point_ = {screen_area_.x + screen_area_.w / 2, screen_area_.y + screen_area_.h / 2};
		}

		void Init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::Init(renderer, resources, state, controller);
			UpdateTexture();
		}

		void Draw() override {
			sdl::set_render_draw_color(renderer_, background_color_);
			sdl::render_fill_rect(renderer_, screen_area_);

			sdl::render_texture_centered(renderer_, text_texture_, mid_point_);
		}

		void HandleEvent(sdl::Event& event) override {
			if (event.type == sdl::EventType::KeyDown) {
				if (event.key.key == hotkey_) {
					on_action_.emit(this);
				}
			}
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

	export class TextElement : public UIElement {
	private:
		sdl::Color color_;
		std::string text_;
		float size_;
		bool centered_;
		sdl::TexturePtr texture_;

		void UpdateTexture() {
			texture_.reset();
			texture_ = resource_manager_->get_text(text_, size_, color_.r, color_.g, color_.b);
		}

	public:
		TextElement(const std::string& text, float size, const sdl::Color& color, bool centered = false)
			: text_(text), size_(size), color_(color), centered_(centered) {
		}

		void Draw() override {
			if (centered_) {
				sdl::FPoint middle{ screen_area_.x + screen_area_.w / 2,screen_area_.y + screen_area_.h / 2 };
				sdl::render_texture_centered(renderer_, texture_, middle);
			}
			else {
				sdl::FRect output_area{ screen_area_.x, screen_area_.y, static_cast<float>(texture_->w), static_cast<float>(texture_->h) };
				sdl::render_texture(renderer_, texture_, output_area);
			}
		}

		void set_color(const sdl::Color& color) {
			color_ = color;
		}

		sdl::Color& color() {
			return color_;
			UpdateTexture();
		}

		void set_text(const std::string& text) {
			text_ = text;
			UpdateTexture();
		}

		std::string& text() {
			return text_;
		}

		void set_size(float size) {
			size_ = size;
			UpdateTexture();
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
			if (!texture_) {
				return;
			}

			if (layout_.width_mode == LayoutMode::Auto) {
				layout_.width = static_cast<float>(texture_->w);
			}

			if (layout_.height_mode == LayoutMode::Auto) {
				layout_.height = static_cast<float>(texture_->h);
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
			std::string new_text = text_getter_();
			if (new_text != text()) {
				set_text(new_text);
			}
			TextElement::Draw();
		}
	};
}
