#include "legacy_skin_loader.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <ranges>

#include <QFileInfo>
#include <QSettings>

#include "render/legacy_image_renderable.hpp"

namespace {

using namespace Qt::Literals::StringLiterals;

// no file extension, just filenames
// also required keys under [files] section
constexpr std::array<QLatin1StringView, 11> required_files = {
  "0"_L1, "1"_L1, "2"_L1, "3"_L1, "4"_L1, "5"_L1, "6"_L1, "7"_L1, "8"_L1, "9"_L1, "s1"_L1
};

constexpr std::array<QLatin1StringView, 3> optional_files = {
  "s2"_L1, "am"_L1, "pm"_L1
};

constexpr QLatin1StringView skin_ini = "skin.ini"_L1;

using FileList = QHash<QLatin1StringView, QString>;

template<typename ReqFiller, typename OptFiller>
FileList get_skin_files_impl(const QDir& skin_dir, ReqFiller req_filler, OptFiller opt_filler)
{
  FileList files;
  std::invoke(req_filler, files, skin_dir, required_files);

  if (files.size() != required_files.size())
    return {};  // something required doesn't exist - invalid skin

  std::invoke(opt_filler, files, skin_dir, optional_files);
  return files;
}

template<std::ranges::input_range R>
void files_from_config_filler(FileList& files, const QDir& skin_dir, const R& files_list)
{
  QSettings s(skin_dir.filePath(skin_ini), QSettings::IniFormat);
  s.beginGroup("files");
  for (const auto& k : files_list)
    if (auto v = s.value(k).toString(); !v.isEmpty() && skin_dir.exists(v))
      files[k] = skin_dir.filePath(v);
  s.endGroup();
}

FileList get_files_from_config(const QDir& skin_dir)
{
  return get_skin_files_impl(skin_dir,
                             files_from_config_filler<decltype(required_files)>,
                             files_from_config_filler<decltype(optional_files)>);
}

QString find_skin_file(QLatin1StringView name, const QDir& skin_dir)
{
  QString target_file;
  for (const auto& ext : {"svg", "png"}) {
    auto file_name = QString("%1.%2").arg(name).arg(ext);
    if (skin_dir.exists(file_name)) {
      target_file = skin_dir.filePath(file_name);
      break;
    }
  }
  return target_file;
}

template<std::ranges::input_range R>
void files_from_skin_dir_filler(FileList& files, const QDir& skin_dir, const R& files_list)
{
  for (const auto& name : files_list)
    if (auto f = find_skin_file(name, skin_dir); !f.isEmpty())
      files[name] = f;
}

FileList get_files_from_dir(const QDir& skin_dir)
{
  return get_skin_files_impl(skin_dir,
                             files_from_skin_dir_filler<decltype(required_files)>,
                             files_from_skin_dir_filler<decltype(optional_files)>);
}

FileList get_skin_files(const QDir& skin_dir)
{
  FileList files = get_files_from_config(skin_dir);
  if (files.isEmpty())
    files = get_files_from_dir(skin_dir);
  return files;
}

} // namespace

LegacyRenderableFactory::LegacyRenderableFactory(const QHash<QChar, QString>& files)
{
  for (auto iter = files.begin(); iter != files.end(); ++iter)
    _resources[iter.key()] = createRenderable(iter.value());
  _has_2_seps = _resources.contains(':') && _resources.contains(' ');
}

std::shared_ptr<SkinResource> LegacyRenderableFactory::item(QChar ch) const
{
  if (ch.isSpace())
    ch = ' ';
  if (RenderableFactory::isSeparator(ch) && ch != ' ')
    ch = ':';
  if (!_has_2_seps && ch == ' ')
    ch = ':';
  auto iter = _resources.find(ch.toLower());
  return iter != _resources.end() ? iter.value() : nullptr;
}

std::shared_ptr<SkinResource> LegacyRenderableFactory::createRenderable(const QString& path)
{
  QFileInfo fi(path);
  QString ext = fi.suffix().toLower();

  if (ext == "svg")
    return std::make_shared<SvgImageRenderable>(path);

  if (ext == "png")
    return std::make_shared<RasterImageRenderable>(path);

  return nullptr;
}


void LegacySkinLoader::init(const QString& skin_root)
{
  QFileInfo finfo(skin_root);
  if (!finfo.isDir())
    return;

  QDir skin_dir(finfo.absoluteFilePath());

  updateMeta(u"name"_s, finfo.fileName());
  loadMeta(skin_dir);
  loadFiles(skin_dir);

  setValid(meta().contains("name") && _files.size() >= 11);
}

void LegacySkinLoader::loadMeta(const QDir& skin_dir)
{
  if (!skin_dir.exists(skin_ini))
    return;

  QSettings s(skin_dir.filePath(skin_ini), QSettings::IniFormat);
  s.beginGroup("info");
  const auto meta_keys = s.childKeys();
  std::ranges::for_each(meta_keys, [&](const auto& k) { updateMeta(k, s.value(k)); });
  s.endGroup();
}

void LegacySkinLoader::loadFiles(const QDir& skin_dir)
{
  auto skin_files = get_skin_files(skin_dir);
  if (skin_files.isEmpty())
    return;

  auto add_file = [&](QChar ch, QLatin1StringView key) {
    if (auto file = skin_files.value(key); !file.isEmpty())
      _files[ch] = file;
  };

  for (char c = '0'; c <= '9'; c++) {
    const std::array<char, 2> str_buf = {c, '\0'};
    add_file(c, QLatin1StringView(str_buf.data()));
  }

  add_file(':'_L1, "s1"_L1);
  add_file(' '_L1, "s2"_L1);
  add_file('a'_L1, "am"_L1);
  add_file('p'_L1, "pm"_L1);
}
