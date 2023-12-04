#include "about_dialog.hpp"
#include "ui_about_dialog.h"

#include <QDate>
#include <QLocale>
#include <QSysInfo>

#include "app/updater/build_date.hpp"

static QString buildDateString()
{
  using digital_clock::core::build_date;
  return QLocale::system().toString(build_date(), QLocale::ShortFormat);
}

// based on Qt Creator sources
static QString compilerString()
{
#if defined(Q_CC_CLANG) // must be before GNU, because clang claims to be GNU too
  QString platformSpecific;
#if defined(__apple_build_version__) // Apple clang has other version numbers
  platformSpecific = QLatin1String(" (Apple)");
#elif defined(Q_CC_MSVC)
  platformSpecific = QLatin1String(" (clang-cl)");
#endif
  return QString("Clang %1.%2").arg(__clang_major__).arg(__clang_minor__) + platformSpecific;
#elif defined(Q_CC_GNU)
  return QString("GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#elif defined(Q_CC_MSVC)
  if (_MSC_VER > 1999)
    return QLatin1String("MSVC <unknown>");
  if (_MSC_VER >= 1930)
    return QLatin1String("MSVC 2022");
  if (_MSC_VER >= 1920)
    return QLatin1String("MSVC 2019");
  if (_MSC_VER >= 1910)
    return QLatin1String("MSVC 2017");
  if (_MSC_VER >= 1900)
    return QLatin1String("MSVC 2015");
#endif
  return QLatin1String("<unknown compiler>");
}

static QString buildCompatibilityString()
{
  return QString("Qt %1 (%2, %3)").arg(QLatin1String(qVersion()),
                                       compilerString(),
                                       QSysInfo::buildCpuArchitecture());
}

AboutDialog::AboutDialog(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::AboutDialog)
{
  ui->setupUi(this);

  // no changelog info for now
  ui->changelog_lbl->hide();
  // no info links for now
  ui->info_group->hide();

  ui->logo_lbl->setPixmap(QApplication::windowIcon().pixmap(192));
  ui->logo_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  ui->info_tab_layout->setAlignment(ui->logo_lbl, Qt::AlignCenter);

  QFont fnt = font();
  fnt.setBold(true);
  fnt.setPointSizeF(fnt.pointSizeF() * 4/3);
  ui->app_ver_lbl->setFont(fnt);
  fnt.setPointSizeF(fnt.pointSizeF() * 4/3);
  ui->app_name_lbl->setFont(fnt);

  ui->app_name_lbl->setText(QApplication::applicationDisplayName());
  ui->app_ver_lbl->setText(tr("version: %1").arg(QApplication::applicationVersion()));
  ui->build_qt_lbl->setText(buildCompatibilityString());
  ui->build_date_lbl->setText(tr("build date: %1").arg(buildDateString()));
}

AboutDialog::~AboutDialog()
{
  delete ui;
}
