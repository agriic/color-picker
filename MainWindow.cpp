//
// Created by agriic on 22.10.5.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <numeric>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

static std::tuple<double, double, double> rgb_to_cvhsl(double pr, double pg, double pb);

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pixs.push_back(ui->w1);
    pixs.push_back(ui->w2);
    pixs.push_back(ui->w3);
    pixs.push_back(ui->w4);
    pixs.push_back(ui->w5);
    pixs.push_back(ui->w6);
    pixs.push_back(ui->w7);
    pixs.push_back(ui->w8);
    pixs.push_back(ui->w9);

    convert = [](double pr, double pg, double pb) { return rgb_to_cvhsl(pr, pg, pb); };

    for (auto w : pixs) {
        w->setAutoFillBackground(true);
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::handle_timer);
    timer->start(100);

    connect(ui->rbHSL, &QRadioButton::clicked, this, &MainWindow::handle_colorspace);
    connect(ui->rbHSV, &QRadioButton::clicked, this, &MainWindow::handle_colorspace);
    connect(ui->rbRGB, &QRadioButton::clicked, this, &MainWindow::handle_colorspace);
}

MainWindow::~MainWindow()
{
    if (display) {
        XCloseDisplay((Display*)display);
    }
    delete ui;
}

static std::tuple<int, int> get_mouse(Display* display)
{
    auto number_of_screens = XScreenCount(display);

    Bool result = False;
    int root_x;
    int root_y;
    for (int i = 0; i < number_of_screens; i++) {
        Window window_returned;
        int win_x;
        int win_y;
        unsigned int mask_return;

        result = XQueryPointer(display, XRootWindow(display, i), &window_returned,
                               &window_returned, &root_x, &root_y, &win_x, &win_y,
                               &mask_return);
        if (result == True) {
            break;
        }
    }
    if (result != True) {
        fprintf(stderr, "No mouse found.\n");
        return {0, 0};
    }

    return {root_x, root_y};
}

static std::tuple<double, double, double> rgb_to_cvhsl(double pr, double pg, double pb)
{
    double r = pr / (256 * 256.0);
    double g = pg / (256 * 256.0);
    double b = pb / (256 * 256.0);

    double mx = std::max(std::max(r, g), b);
    double mn = std::min(std::min(r, g), b);

    double d = mx - mn;

    double H = 0;
    if (d != 0) {
        if (mx == r) {
            H = (g - b) / d;
        } else if (mx == g) {
            H = 2.0 + (b - r) / d;
        } else if (mx == b) {
            H = 4.0 + (r - g) / d;
        }
    }
    H *= 60;
    if (H < 0) H += 360;

    double L = std::midpoint(mx, mn);

    double S = 0;
    if (d != 0) {
        S = d / (1 - fabs(2 * L - 1));
    }

    return {H / 2, S * 255, L * 255};
}
static std::tuple<double, double, double> rgb_to_cvhsv(double pr, double pg, double pb)
{
    double r = pr / (256 * 256.0);
    double g = pg / (256 * 256.0);
    double b = pb / (256 * 256.0);

    double mx = std::max(std::max(r, g), b);
    double mn = std::min(std::min(r, g), b);

    double d = mx - mn;

    double H = 0;
    if (d == 0) {
        if (mx == r) {
            H = (g - b) / d;
        } else if (mx == g) {
            H = 2.0 + (b - r) / d;
        } else if (mx == b) {
            H = 4.0 + (r - g) / d;
        }
    }
    H *= 60;
    if (H < 0) H += 360;

    double V = mx;

    double S = 0;
    if (mx != 0) {
        S = d / mx;
    }

    return {H / 2, S * 255, V * 255};
}
static std::tuple<double, double, double> rgb_to_cvrgb(double pr, double pg, double pb)
{
    double r = pr / (256 * 256.0);
    double g = pg / (256 * 256.0);
    double b = pb / (256 * 256.0);

    return {r * 255, g * 255, b * 255};
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!display) {
        display = XOpenDisplay((char *) nullptr);
        if (!display) {
            qDebug() << "E1";
            return;
        }
    }

    auto* dp = (Display*)this->display;

    auto [x, y] = get_mouse(dp);
    if (x < 0) x++;
    if (y < 0) y++;

    XImage *image = XGetImage (dp, XRootWindow (dp, XDefaultScreen (dp)), x-1, y-1, 3, 3, AllPlanes, XYPixmap);
    if (!image) {
        // image not available on screen edges
        return;
    }

    double mh = 360, ms = 300, ml = 300;
    double xh = 0, xs = 0, xl = 0;

    std::string cpix;
    XColor cc;
    double cl;

    for (int iy = 0; iy < image->height; iy++) {
        for (int ix = 0; ix < image->width; ix++) {
            XColor c;
            c.pixel = XGetPixel (image, ix, iy);
            XQueryColor (dp, XDefaultColormap(dp, XDefaultScreen (dp)), &c);

            auto [h,s,l] = convert(c.red, c.green, c.blue);

            mh = std::min(h, mh);
            ms = std::min(s, ms);
            ml = std::min(l, ml);
            xh = std::max(h, xh);
            xs = std::max(s, xs);
            xl = std::max(l, xl);

            if (ix == 1 && iy == 1) {
                cpix = fmt::format("{:3.0f}, {:3.0f}, {:3.0f}", h,s,l);
                cc = c;
                cl = l;
            }
        }
    }
    XFree (image);


    auto str = fmt::format("M({:4d},{:4d}) | #{:6X}  {}({})   |   [{:3.0f} {:3.0f}] [{:3.0f} {:3.0f}] [{:3.0f} {:3.0f}]",
                           x, y, cc.pixel, cspace, cpix, mh, xh, ms, xs, ml, xl);

    ui->clist->addItem(QString::fromStdString(str));
    ui->clist->item(ui->clist->count()-1)->setBackground(QBrush(QColor(cc.red/256, cc.green/256, cc.blue/256)));
    if (cl < 125) {
        ui->clist->item(ui->clist->count()-1)->setForeground(QBrush(QColor(255, 255, 255)));
    }

    ui->clist->scrollToBottom();
}

void MainWindow::on_clearButton_clicked()
{
    ui->clist->clear();
}

void MainWindow::handle_timer()
{
    if (!display) {
        display = XOpenDisplay((char *) nullptr);
        if (!display) {
            qDebug() << "E1";
            return;
        }
    }

    auto* dp = (Display*)this->display;

    auto [x, y] = get_mouse(dp);
    if (x < 0) x++;
    if (y < 0) y++;

    XImage *image = XGetImage (dp, XRootWindow (dp, XDefaultScreen (dp)), x-1, y-1, 3, 3, AllPlanes, XYPixmap);
    if (!image) {
        return;
    }

    for (int iy = 0; iy < image->height; iy++) {
        for (int ix = 0; ix < image->width; ix++) {
            int id = iy * image->width + ix;
            XColor c;
            c.pixel = XGetPixel (image, ix, iy);
            XQueryColor (dp, XDefaultColormap(dp, XDefaultScreen (dp)), &c);

            pixs[id]->setPalette(QPalette(QColor(c.red/256, c.green/256, c.blue/256)));
        }
    }
    XFree (image);
}

void MainWindow::handle_colorspace()
{
    qDebug() << "Click";
    if (ui->rbHSL->isChecked()) {
        convert = [](double pr, double pg, double pb) { return rgb_to_cvhsl(pr, pg, pb); };
        cspace = "HSL";
    } else if (ui->rbHSV->isChecked()) {
        convert = [](double pr, double pg, double pb) { return rgb_to_cvhsv(pr, pg, pb); };
        cspace = "HSV";
    } else if (ui->rbRGB->isChecked()) {
        convert = [](double pr, double pg, double pb) { return rgb_to_cvrgb(pr, pg, pb); };
        cspace = "RGB";
    }
}
