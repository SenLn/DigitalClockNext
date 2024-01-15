/*
    Digital Clock - beautiful customizable clock with plugins
    Copyright (C) 2023  Nick Korotysh <nick.korotysh@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "core/settings.hpp"

#include <QSettings>

class BackendQSettings final : public ConfigBackend<QString, QString, QVariant> {
public:
  BackendQSettings()
    : _settings(std::make_unique<QSettings>())
  {}

  explicit BackendQSettings(const QString& filename)
    : _settings(std::make_unique<QSettings>(filename, QSettings::IniFormat))
  {}

  BackendQSettings(const QString& organization, const QString& application)
    : _settings(std::make_unique<QSettings>(organization, application))
  {}

  void load(const QString&) override {}
  void save(const QString&) override {}

  SettingsData allSettings() const override
  {
    SettingsData all_settings;
    // this assumes some config implementation specific:
    // key can't have '/', but tag can, so consider the
    // last part of "path" as "key" and the rest as tag
    const auto all_keys = _settings->allKeys();
    for (const auto& skey : all_keys) {
      auto sidx = skey.lastIndexOf('/');
      if (sidx == -1) continue;
      auto key = skey.mid(sidx + 1);
      auto tag = skey.mid(0, sidx);
      all_settings[tag][key] = _settings->value(skey);
    }
    return all_settings;
  }

  void setValue(const QString& tag, const QString& k, const QVariant& v) override
  {
    GroupGuard _(*_settings, tag);
    _settings->setValue(k, v);
  }

  std::optional<QVariant> value(const QString& tag, const QString& k) const override
  {
    GroupGuard _(*_settings, tag);
    QVariant v = _settings->value(k);
    return v.isValid() ? std::optional(v) : std::nullopt;
  }

private:
  class GroupGuard final {
  public:
    GroupGuard(QSettings& s, QAnyStringView g)
      : _settings(s) { _settings.beginGroup(g); }
    ~GroupGuard() { _settings.endGroup(); }
  private:
    QSettings& _settings;
  };

private:
  std::unique_ptr<QSettings> _settings;
};
