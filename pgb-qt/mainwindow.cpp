#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include "mainwindow.hpp"
#include "./ui_mainwindow.h"

size_t frame;

QMessageLogger logger;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  scene = new QGraphicsScene(this);
  ui->emuGraphics->setScene(scene);
  pixMap = new GBPixMap();
  scene->addItem(pixMap);
}

MainWindow::~MainWindow() {
//  delete scene;
//  delete pixMap;
  delete ui;
}

void MainWindow::loadRom() {
  QString fileName = QFileDialog::getOpenFileName(
    this, "Select ROM",
    ".",
    "Game Boy Roms (*.gb)"
    );

  if (fileName.isEmpty()) {
    return;
  }

  QFile file(fileName);
  file.open(QFile::ReadOnly);
  QByteArray array = file.readAll();
  std::vector<uint8_t> bytes;
  bytes.resize(array.size());
  memcpy(bytes.data(), array.data(), bytes.size());

  rom = ROM::readRom(bytes);

}

void MainWindow::on_romLoadButton_clicked() {
  loadRom();
}

void GBPixMap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  QGraphicsPixmapItem::paint(painter, option, widget);
  frame++;
  logger.debug("Frame %zu", frame);
}

void GBPixMap::advance(int phase) {
  logger.debug("Frame %zu phase %d", frame, phase);
}
