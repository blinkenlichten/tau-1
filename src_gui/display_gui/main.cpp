#include <QApplication>
#include <QDir>
#include <cassert>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  {
    QDir dir(QApplication::applicationDirPath());
    QStringList lib_paths = QApplication::libraryPaths();
    lib_paths << QApplication::applicationDirPath();
#ifdef __MACH__
    lib_paths << dir.absolutePath() << dir.absolutePath() + "/plugins";
    dir.cdUp();
    lib_paths << dir.absolutePath();
    dir.cd("PlugIns");
    lib_paths << dir.absolutePath();
#endif
    dir.cd("plugins");
    lib_paths << dir.absolutePath();
    dir.cd("platforms");
    lib_paths << dir.absolutePath();
    dir.cd("imageformats");
    lib_paths << dir.absolutePath();
    QApplication::setLibraryPaths(lib_paths);
  }
  //Widget w;
  //w.show();

  return a.exec();
}
