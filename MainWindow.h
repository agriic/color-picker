//
// Created by agriic on 22.10.5.
//

#ifndef PICKER2_MAINWINDOW_H
#define PICKER2_MAINWINDOW_H

#include <QWidget>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    [[maybe_unused]] void on_clearButton_clicked();
    void handle_timer();
    void handle_colorspace();

private:
    Ui::MainWindow *ui;

    void* display = nullptr;
    QTimer* timer = nullptr;
    std::vector<QWidget*> pixs;

    std::function<std::tuple<double, double, double>(double, double, double)> convert;
    std::string cspace = "HSL";

protected:
    void keyPressEvent(QKeyEvent *event) override;


};


#endif //PICKER2_MAINWINDOW_H
