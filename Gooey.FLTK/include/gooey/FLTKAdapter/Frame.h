#pragma once

#include <Windows.h>  // for icon support

#include <cstdint>  // <--- Require before FL/Fl.H
//

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <gooey.h>

#include <map>
#include <memory>

#include "Colors.h"
#include "Defaults.h"
#include "Grid.h"
#include "Pack.h"
#include "Panel.h"
#include "WidgetContainer.h"

namespace gooey::FLTKAdapter {

    namespace Impl {
        class FLTKWindow : public Fl_Window {
            Impl::PackWhichIncreasesSizeOfItsParent* _pack;
            std::unique_ptr<Fl_Image>                _backgroundImage;
            UIBackgroundImageStyle _BackgroundImageStyle = UIBackgroundImageStyle::Default;
            std::map<Fl_Widget*, std::pair<int, int>> _initialWidgetSizes;

        public:
            FLTKWindow(int w, int h)
                : Fl_Window(w, h), _pack(new Impl::PackWhichIncreasesSizeOfItsParent(0, 0, w, h)) {
                resizable(this);
                _pack->type(Fl_Pack::VERTICAL);
                _pack->init_sizes();
                resizable(_pack);
            }

            Impl::PackWhichIncreasesSizeOfItsParent* GetFlPack() { return _pack; }

            void SetBackgroundImage(const char* path, UIBackgroundImageStyle mode) {
                if (_backgroundImage) delete _backgroundImage.release();
                _backgroundImage      = std::make_unique<Fl_PNG_Image>(path);
                _BackgroundImageStyle = mode;
                if (_BackgroundImageStyle == UIBackgroundImageStyle::Default)
                    _BackgroundImageStyle = Defaults::BackgroundImageStyle;
            }

            void DrawBackgroundImage(
                int x, int y, int w, int h,
                UIBackgroundImageStyle mode = UIBackgroundImageStyle::Default
            ) {
                fl_push_clip(x, y, w, h);
                std::unique_ptr<Fl_Image> scaledImage(_backgroundImage->copy(w, h));
                scaledImage->draw(x, y);
                fl_pop_clip();
            }

            void draw() override {
                Fl_Window::draw();

                if (_backgroundImage) DrawBackgroundImage(x(), y(), w(), h());
            }

            void show() override {
                Fl_Window::show();
                this->init_sizes();
                this->redraw();
            }
        };
    }

    class Frame : public UIWindow, WidgetContainer {
        Impl::FLTKWindow* _implWindow;

    public:
        Frame() : _implWindow(new Impl::FLTKWindow(Defaults::WindowWidth, Defaults::WindowHeight)) {
            SetImplFlPack(_implWindow->GetFlPack());
        }

        GOOEY_FLTK_COLOR_SETTERS(_implWindow)

        UILabel*  AddLabel(const char* text) override { return WidgetContainer::AddLabel(text); }
        UIButton* AddButton(const char* text) override { return WidgetContainer::AddButton(text); }

        UIPanel* AddHorizontalPanel() override {
            auto panel = std::make_unique<Panel>(_implWindow->GetFlPack(), true);
            return static_cast<UIPanel*>(AddWidget(std::move(panel)));
        }
        UIPanel* AddVerticalPanel() override {
            auto panel = std::make_unique<Panel>(_implWindow->GetFlPack(), false);
            return static_cast<UIPanel*>(AddWidget(std::move(panel)));
        }
        UIGrid* AddGrid(unsigned int cols, unsigned int rows) override {
            auto grid = std::make_unique<Grid>(_implWindow->GetFlPack(), cols, rows);
            return static_cast<UIGrid*>(AddWidget(std::move(grid)));
        }

        bool SetTitle(const char* title) override {
            _implWindow->label(title);
            return true;
        }

        bool Show() override {
            _implWindow->show();
            // Fl::check();
            // _implWindow->resize(
            //     _implWindow->x(), _implWindow->y(), _implWindow->w(), _implWindow->h()
            // );
            // _implWindow->redraw();
            return true;
        }
    };
}
