#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTimer>
#include <pgb/CPU.hpp>
#include <pgb/GPU.hpp>
#include <QGraphicsPixmapItem>

class GBPixMap : public QGraphicsPixmapItem {
public:
  void advance(int phase) override;

  std::shared_ptr<CPU> cpu;
  std::shared_ptr<GPU> gpu;
  std::shared_ptr<MMU> mmu;
  std::shared_ptr<ROM> rom;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_romLoadButton_clicked();

private:
  Ui::MainWindow *ui;
  void loadRom();

  QGraphicsScene *scene;
  GBPixMap *pixMap;
  QTimer timer;
};

#endif // MAINWINDOW_HPP
