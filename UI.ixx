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
		sdl::FRect screenArea;
		sdl::RendererPtr renderer;
		std::shared_ptr<ResourceManager> resourceManager;
		std::vector<std::shared_ptr<UIElement>> children;
		std::shared_ptr<WorldState> state;
		std::string id;
		std::shared_ptr<GameController> controller;

	public:
		virtual void set_screen_area(sdl::FRect& screenArea) {
			this->screenArea = screenArea;
		}

		sdl::FRect& get_screen_area() {
			return this->screenArea;
		}

		sdl::FRect get_offset_area(sdl::Point& offset) {
			return sdl::FRect{this->screenArea.x + offset.x, this->screenArea.y + offset.y, this->screenArea.w, this->screenArea.h};
		}

		void set_world_state(std::shared_ptr<WorldState>& state) {
			this->state = state;
			for (auto element : children) {
				element->set_world_state(state);
			}
		}

		void set_id(const std::string& id) {
			this->id = id;
		}

		std::string& get_id() {
			return this->id;
		}

		template <typename T> void add_child(std::shared_ptr<T>& element) {
			element->init(this->renderer, this->resourceManager, this->state, this->controller);
			this->children.push_back(element);
		}

		virtual void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller) {
			this->renderer = renderer;
			this->resourceManager = resources;
			this->state = state;
			this->controller = controller;
		}

		virtual void update(float passedTime) {
			for (auto element : children) {
				element->update(passedTime);
			}
		}

		virtual void draw(sdl::Point& offset) {
			sdl::Point newOffset{ offset.x + this->screenArea.x, offset.y + this->screenArea.y };
			for (auto element : children) {
				sdl::Rect clipRect{ newOffset.x + element->get_screen_area().x, newOffset.y + element->get_screen_area().y, element->get_screen_area().w, element->get_screen_area().h };
				sdl::set_render_clip_rect(this->renderer, &clipRect);
				element->draw(newOffset);
				sdl::set_render_clip_rect(this->renderer, nullptr);
			}
		}
		virtual void handle_event(sdl::Event& event) {
			for (auto element : children) {
				element->handle_event(event);
			}
		};
	};

	export class VerticalStack : public UIElement {
	private:
	public:
		void set_screen_area(sdl::FRect& screenArea) override {
			UIElement::set_screen_area(screenArea);

			// Start with our top left and keep the width. Height will be taken from the child elements.
			sdl::FRect childArea = {0, 0, this->screenArea.w, 0};
			for (std::shared_ptr<UIElement>& child : this->children) {
				// Keep width of
				childArea.h = child->get_screen_area().h;
				child->set_screen_area(childArea);

				childArea.y += childArea.h;
			}
		}
	};

	export class TextElement : public UIElement {
	private:
		sdl::Color color;
		std::string text;
		float size;
		bool centered;

	public:
		TextElement(const std::string& text, float size, const sdl::Color& color, bool centered = false)
			: text(text), size(size), color(color), centered(centered) {
		}

		void draw(sdl::Point& offset) override {
			auto texture = this->resourceManager->get_text(this->text, this->size, this->color);
			if (this->centered) {
				sdl::FPoint middle{ offset.x + this->screenArea.x + this->screenArea.w / 2, offset.y + this->screenArea.y + this->screenArea.h / 2 };
				sdl::render_texture_centered(this->renderer, texture, middle);
			}
			else {
				sdl::FRect outputArea{ offset.x + this->screenArea.x, offset.y + this->screenArea.y, this->screenArea.w, this->screenArea.h };
				sdl::render_texture(this->renderer, texture, outputArea);
			}
		}

		void set_color(const sdl::Color& color) {
			this->color = color;
		}

		sdl::Color& get_color() {
			return this->color;
		}

		void set_text(const std::string& text) {
			this->text = text;
		}

		std::string& get_text() {
			return this->text;
		}

		void set_size(float size) {
			this->size = size;
		}

		float get_size() {
			return this->size;
		}

		void set_centered(bool centered) {
			this->centered = centered;
		}

		bool is_centered() {
			return this->centered;
		}

	};

	export class DynamicTextElement : public TextElement {
	private:
		std::function<std::string()> textGetter;
	public:
		DynamicTextElement(std::function<std::string()>& textGetter, float size, const sdl::Color& color, bool centered = false)
			: TextElement("", size, color, centered) {
			this->textGetter = textGetter;
		}

		void draw(sdl::Point& offset) override {
			this->set_text(textGetter());
			TextElement::draw(offset);
		}
	};

	export class GameState : public UIElement {
	protected:
		std::string nextState;

		void fill_screen() {

		}
	public:

		std::string& get_next_state() {
			return nextState;
		}

		void set_next_state(const std::string& stateName) {
			nextState = stateName;
		}

		void init(sdl::RendererPtr& renderer, std::shared_ptr<ResourceManager>& resources, std::shared_ptr<WorldState>& state, std::shared_ptr<GameController>& controller)  override {
			UIElement::init(renderer, resources, state, controller);

			// Set screen area to whole screen
			int width, height;
			sdl::get_current_render_output_size(renderer, &width, &height);
			sdl::FRect ownArea{ 0, 0, width, height };
			this->set_screen_area(ownArea);
		}

		void handle_event(sdl::Event& event) override {
			if (event.type == sdl::EventType::WindowResized) {
				sdl::FRect ownArea{ 0, 0, event.window.data1, event.window.data2 };
				this->set_screen_area(ownArea);
			}
			UIElement::handle_event(event);
		}
	};
}
