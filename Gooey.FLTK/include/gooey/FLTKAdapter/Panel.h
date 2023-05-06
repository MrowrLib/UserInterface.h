#pragma once

#include <cstdint>
//

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_draw.H>
#include <gooey.h>

#include <map>
#include <memory>

#include "Colors.h"
#include "Grid.h"
#include "Pack.h"
#include "WidgetContainer.h"

namespace gooey::FLTKAdapter {

    namespace Impl {

        // class PackWhichIncreasesSizeOfItsParent : public Fl_Pack {
        // public:
        //     PackWhichIncreasesSizeOfItsParent(
        //         int x, int y, int w, int h, const char* label = nullptr
        //     )
        //         : Fl_Pack(x, y, w, h, label) {}

        //     void add(Fl_Widget* widget) {
        //         Fl_Pack::add(widget);
        //         update_parent_size();
        //     }

        //     void update_parent_size() {
        //         if (!parent()) return;

        //         int max_x = 0;
        //         int max_y = 0;

        //         for (int i = 0; i < children(); ++i) {
        //             Fl_Widget* widget       = child(i);
        //             int        widget_max_x = widget->x() + widget->w();
        //             int        widget_max_y = widget->y() + widget->h();

        //             if (widget_max_x > max_x) max_x = widget_max_x;
        //             if (widget_max_y > max_y) max_y = widget_max_y;
        //         }

        //         parent()->size(max_x, max_y);
        //     }
        // };

        class BackgroundImage {
            std::string               _path;
            std::unique_ptr<Fl_Image> _image;
            Fl_Shared_Image*          _sharedImage;
            UIBackgroundImageStyle    _style;
            UIHorizontalAlignment     _horizontalAlignment;
            UIVerticalAlignment       _verticalAlignment;

        public:
            BackgroundImage(
                const char* path, UIBackgroundImageStyle style = UIBackgroundImageStyle::Default,
                UIHorizontalAlignment horizontalAlignment = UIHorizontalAlignment::Default,
                UIVerticalAlignment   verticalAlignment   = UIVerticalAlignment::Default,
                unsigned int width = 0, unsigned int height = 0
            )
                : _path(path),
                  _style(style),
                  _horizontalAlignment(horizontalAlignment),
                  _verticalAlignment(verticalAlignment) {
                _sharedImage = Fl_Shared_Image::get(path);
                if (_sharedImage) {
                    if (width == 0 || height == 0) _image.reset(_sharedImage->copy());
                    else _image.reset(_sharedImage->copy(width, height));
                }
            }

            ~BackgroundImage() {
                if (_sharedImage) _sharedImage->release();
            }

            void Draw(int x, int y, int w, int h) {
                auto style = _style;
                if (style == UIBackgroundImageStyle::Default)
                    style = Defaults::BackgroundImageStyle;

                auto horizontalAlignment = _horizontalAlignment;
                if (horizontalAlignment == UIHorizontalAlignment::Default)
                    horizontalAlignment = Defaults::DefaultHorizontalAlignment;

                auto verticalAlignment = _verticalAlignment;
                if (verticalAlignment == UIVerticalAlignment::Default)
                    verticalAlignment = Defaults::DefaultVerticalAlignment;

                switch (style) {
                    case UIBackgroundImageStyle::Original:
                        _image->draw(x, y, w, h);
                        break;
                    case UIBackgroundImageStyle::Fill: {
                        fl_push_clip(x, y, w, h);
                        std::unique_ptr<Fl_Image> scaledImage(_image->copy(w, h));
                        scaledImage->draw(x, y);
                        fl_pop_clip();
                        break;
                    }
                    case UIBackgroundImageStyle::Repeat: {
                        int imgW = _image->w();
                        int imgH = _image->h();
                        for (int X = x; X < x + w; X += imgW) {
                            for (int Y = y; Y < y + h; Y += imgH) {
                                _image->draw(X, Y, imgW, imgH, 0, 0);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        };

        class BackgroundImageBox : public Fl_Box {
            std::map<std::string, std::unique_ptr<BackgroundImage>> _backgroundImages;

        public:
            BackgroundImageBox(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {}

            void AddBackgroundImage(
                const char* path, UIBackgroundImageStyle style,
                UIHorizontalAlignment horizontalAlignment, UIVerticalAlignment verticalAlignment,
                unsigned int width, unsigned int height
            ) {
                _backgroundImages.insert(std::make_pair(
                    path, std::move(std::make_unique<BackgroundImage>(
                              path, style, horizontalAlignment, verticalAlignment, width, height
                          ))
                ));
            }

            void RemoveBackgroundImage(const char* path) { _backgroundImages.erase(path); }

            void draw() override {
                // Fl_Box::draw();
                for (auto& [path, image] : _backgroundImages) image->Draw(x(), y(), w(), h());
            }
        };
    }

    class Panel : public UIPanel, WidgetContainer {
        Fl_Group*                                _implGroup;
        Impl::PackWhichIncreasesSizeOfItsParent* _implPack;
        // Impl::BackgroundImageBox*                       _implBackgroundImageBox;
        std::map<Impl::FLTK_Grid*, std::pair<int, int>> _initialGridDimensions;

        void storeInitialGridDimensions(Impl::FLTK_Grid* grid) {
            _initialGridDimensions[grid] = std::make_pair(grid->w(), grid->h());
        }

    public:
        Panel(Fl_Pack* pack, bool horizontal)
            : _implGroup(new Fl_Group(0, 0, Defaults::PanelWidth, Defaults::PanelHeight)) {
            _implGroup->resizable(_implGroup);
            _implGroup->box(FL_UP_BOX);
            // _implBackgroundImageBox =
            //     new Impl::BackgroundImageBox(0, 0, Defaults::PanelWidth, Defaults::PanelHeight);
            // _implGroup->add(_implBackgroundImageBox);
            _implPack = new Impl::PackWhichIncreasesSizeOfItsParent(
                0, 0, Defaults::PanelWidth, Defaults::PanelHeight
            );
            _implPack->spacing(10);
            _implPack->type(horizontal ? Fl_Pack::HORIZONTAL : Fl_Pack::VERTICAL);
            _implGroup->add(_implPack);  // Add the inner Fl_Pack to the outer Fl_Group.
            // _implGroup->end();
            pack->add(_implGroup);  // Add the outer Fl_Group to the passed Fl_Pack.
            SetImplFlPack(_implPack);
        }

        GOOEY_FLTK_COLOR_SETTERS(_implGroup)

        UILabel*  AddLabel(const char* text) override { return WidgetContainer::AddLabel(text); }
        UIButton* AddButton(const char* text) override { return WidgetContainer::AddButton(text); }

        UIPanel* AddHorizontalPanel() override {
            auto panel = std::make_unique<Panel>(_implPack, true);
            return static_cast<UIPanel*>(AddWidget(std::move(panel)));
        }
        UIPanel* AddVerticalPanel() override {
            auto panel = std::make_unique<Panel>(_implPack, false);
            return static_cast<UIPanel*>(AddWidget(std::move(panel)));
        }
        UIGrid* AddGrid(unsigned int cols, unsigned int rows) override {
            auto grid = std::make_unique<Grid>(_implPack, cols, rows);
            storeInitialGridDimensions(grid.get()->GetImplGrid());
            return static_cast<UIGrid*>(AddWidget(std::move(grid)));
        }

        bool AddBackgroundImage(
            const char* path, UIBackgroundImageStyle style = UIBackgroundImageStyle::Default,
            UIHorizontalAlignment horizontalAlignment = UIHorizontalAlignment::Left,
            UIVerticalAlignment   verticalAlignment   = UIVerticalAlignment::Top,
            unsigned int width = 0, unsigned int height = 0
        ) override {
            // _implBackgroundImageBox->AddBackgroundImage(
            //     path, style, horizontalAlignment, verticalAlignment, width, height
            // );
            return true;
        }

        bool RemoveBackgroundImage(const char* path) override {
            // _implBackgroundImageBox->RemoveBackgroundImage(path);
            return true;
        }
    };
}
//
// void SetBackgroundImage(const char* path) {
//     // _backgroundImage = std::make_unique<Fl_PNG_Image>(path);
// }

// void DrawBackgroundImage(
//     int x, int y, int w, int h,
//     UIBackgroundImageStyle mode = UIBackgroundImageStyle::Default
// ) {
//     _backgroundImage->draw(w - _backgroundImage->w(), y, w, h);
//     // fl_push_clip(x, y, w, h);
//     // std::unique_ptr<Fl_Image> scaledImage(_backgroundImage->copy(w, h));
//     // scaledImage->draw(x, y);
//     // fl_pop_clip();
// }
//
// void draw() override {
//     // ONCE
//     // if (_image) _image->draw(x(), y(), w(), h());

//     // Repeating
//     // if (_image) {
//     //     fl_push_clip(x(), y(), w(), h());
//     //     int imgW = _image->w();
//     //     int imgH = _image->h();
//     //     for (int X = x(); X < x() + w(); X += imgW) {
//     //         for (int Y = y(); Y < y() + h(); Y += imgH) {
//     //             _image->draw(X, Y, imgW, imgH, 0, 0);
//     //         }
//     //     }
//     //     fl_pop_clip();
//     // }

//     // Stretch (maintain aspect ratio)
//     // if (_image) {
//     //     fl_push_clip(x(), y(), w(), h());

//     //     // Calculate the aspect ratio of the image
//     //     float imgAspectRatio =
//     //         static_cast<float>(_image->w()) / static_cast<float>(_image->h());
//     //     int drawW = w();
//     //     int drawH = static_cast<int>(drawW / imgAspectRatio);

//     //     // If the calculated height is less than the available height, adjust the
//     //     width
//     //     // accordingly
//     //     if (drawH < h()) {
//     //         drawH = h();
//     //         drawW = static_cast<int>(drawH * imgAspectRatio);
//     //     }

//     //     _image->draw(x(), y(), drawW, drawH, 0, 0);

//     //     fl_pop_clip();
//     // }

//     // Stetch (ignore aspect ratio)
//     // if (_image) {
//     //     fl_push_clip(x(), y(), w(), h());
//     //     std::unique_ptr<Fl_Image> scaledImage(_image->copy(w(), h()));
//     //     scaledImage->draw(x(), y());
//     //     fl_pop_clip();
//     // }

//     if (!_backgroundImage)
//         _backgroundImage.reset(Fl_Shared_Image::get("Cleric.png")->copy());

//     if (_backgroundImage) {
//         DrawBackgroundImage(x(), y(), w(), h());
//         // _backgroundImage->draw(0, 0, w(), h());
//         // fl_push_clip(x(), y(), w(), h());
//         // std::unique_ptr<Fl_Image> scaledImage(_backgroundImage->copy(w(), h()));
//         // scaledImage->draw(x(), y());
//         // fl_pop_clip();
//     }

//     Fl_Box::draw();
// }