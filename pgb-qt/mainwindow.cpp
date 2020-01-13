#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QTimer>
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

  QObject::connect(&timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
  timer.start(1000 / 60);
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

  pixMap->rom = ROM::readRom(bytes);
  pixMap->mmu = std::make_shared<MMU>(pixMap->rom);
  pixMap->cpu = std::make_shared<CPU>(pixMap->mmu);
  pixMap->gpu = std::make_shared<GPU>(pixMap->mmu);
}

void MainWindow::on_romLoadButton_clicked() {
  loadRom();
}

void GBPixMap::advance(int phase) {
  if (phase == 1) {
    if (cpu) {
      uint64_t frame = cpu->clock() / CLOCK_FRAME;
      while (true) {
        uint64_t startClock = cpu->clock();
        cpu->emulateInstruction();
        uint64_t endClock = cpu->clock();
        gpu->update(endClock - startClock);
        uint64_t newFrame = endClock / CLOCK_FRAME;
        if (newFrame != frame) {
//          cpu->printState();
          QImage image(
            (uchar *) (gpu->framebuffer.data()),
            LINE_WIDTH, LINES,
            QImage::Format::Format_RGB555
          );
          setPixmap(QPixmap::fromImage(image));
          break;
        }
      }
    }
  }
}