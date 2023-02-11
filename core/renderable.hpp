#pragma once

#include <QRectF>

class QPainter;

class Renderable {
public:
  virtual ~Renderable() = default;

  // TODO: maybe rename to geometry
  // means "original bounding rect", i.e. as was returned from the source
  // this value will be used by layout as base value for transormation
  virtual QRectF rect() const = 0;

  virtual qreal advanceX() const = 0;
  virtual qreal advanceY() const = 0;

  // no target rect here!
  // it is renderer responsibility to setup coordinate system
  virtual void render(QPainter* p) const = 0;

  // TODO: consider `noexcept` just because the only flag is assumed
  virtual bool isVisible() const = 0;
  virtual void setVisible(bool visible) = 0;
};


class RenderableBase : public Renderable {
public:
  void render(QPainter* p) const final
  {
    if (!_visible || !p)
      return;

    renderImpl(p);
  }

  bool isVisible() const final { return _visible; }
  void setVisible(bool visible) final { _visible = visible; }

protected:
  virtual void renderImpl(QPainter* p) const = 0;

private:
  bool _visible = true;
};
