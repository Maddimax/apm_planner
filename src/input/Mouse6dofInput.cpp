/**
 * @file
 *   @brief 3dConnexion 3dMouse interface for QGroundControl
 *
 *   @author Matthias Krebs <makrebs@student.ethz.ch>
 *
 */

#include "Mouse6dofInput.h"
#include "UAS.h"
#include "UASManager.h"
#include "QMessageBox"
#ifdef MOUSE_ENABLED_LINUX
#include <QX11Info>
#include <X11/Xlib.h>
#undef Success              // Eigen library doesn't work if Success is defined
extern "C"
{
#include "xdrvlib.h"
}
#endif // MOUSE_ENABLED_LINUX

#ifdef MOUSE_ENABLED_WIN
Mouse6dofInput::Mouse6dofInput(Mouse3DInput* mouseInput) :
    mouse3DMax(0.075),   // TODO: check maximum value fot plugged device
    uas(NULL),
    done(false),
    mouseActive(false),
    xValue(0.0),
    yValue(0.0),
    zValue(0.0),
    aValue(0.0),
    bValue(0.0),
    cValue(0.0)
{
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));
    // Connect 3DxWare SDK MotionEvent
    connect(mouseInput, SIGNAL(Move3d(std::vector<float>&)), this, SLOT(motion3DMouse(std::vector<float>&)));
    // TODO: Connect button mapping
    //connect(mouseInput, SIGNAL(On3dmouseKeyDown(int)), this, SLOT);
    //connect(mouseInput, SIGNAL(On3dmouseKeyUp(int)), this, SLOT);

}
#endif //MOUSE_ENABLED_WIN

#ifdef MOUSE_ENABLED_LINUX
Mouse6dofInput::Mouse6dofInput(QWidget *parent) :
    mouse3DMax(0.075),   // TODO: check maximum value fot plugged device
    uas(NULL),
    done(false),
    mouseActive(false),
    xValue(0.0),
    yValue(0.0),
    zValue(0.0),
    aValue(0.0),
    bValue(0.0),
    cValue(0.0)
{

}
#endif //MOUSE_ENABLED_LINUX

#ifdef MOUSE_ENABLED_LINUX

void Mouse6dofInput::init3dMouse(QWidget* parent)
{
    if (!mouseActive)
    {
//        // man visudo --> then you can omit giving password (success not guarantied..)
//        qDebug() << "Starting 3DxWare Daemon for 3dConnexion 3dMouse";
//        QString processProgramm = "gksudo";
//        QStringList processArguments;
//        processArguments << "/etc/3DxWare/daemon/3dxsrv -d usb";
//        process3dxDaemon = new QProcess();
//        process3dxDaemon->start(processProgramm, processArguments);
//    //    process3dxDaemon->waitForFinished();
//    //    {
//    //        qDebug() << "... continuing without 3DxWare. May not be initialized properly!";
//    //        qDebug() << "Try in terminal as user root:" << processArguments.last();
//    //    }

        Display *display = QX11Info::display();
        if(!display)
        {
            qDebug() << "Cannot open display!" << endl;
        }
        if ( !MagellanInit( display, winId() ) )
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("No 3DxWare driver is running."));
            msgBox.setInformativeText(tr("Enter in Terminal 'sudo /etc/3DxWare/daemon/3dxsrv -d usb'"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();

            qDebug() << "No 3DxWare driver is running!";
            return;
        }
        else
        {
            qDebug() << "Initialized 3dMouse";
            mouseActive = true;
        }
    }
    else
    {
        qDebug() << "3dMouse already initialized..";
    }
}
#endif //MOUSE_ENABLED_LINUX

Mouse6dofInput::~Mouse6dofInput()
{
    done = true;
}

void Mouse6dofInput::setActiveUAS(UASInterface* uas)
{
    // Only connect / disconnect is the UAS is of a controllable UAS class
    UAS* tmp = 0;
    if (this->uas)
    {
        tmp = dynamic_cast<UAS*>(this->uas);
        if(tmp)
        {
            disconnect(this, SIGNAL(mouse6dofChanged(double,double,double,double,double,double)), tmp, SLOT(setManual6DOFControlCommands(double,double,double,double,double,double)));
            // Todo: disconnect button mapping
        }
    }

    this->uas = uas;

    tmp = dynamic_cast<UAS*>(this->uas);
    if(tmp) {
                connect(this, SIGNAL(mouse6dofChanged(double,double,double,double,double,double)), tmp, SLOT(setManual6DOFControlCommands(double,double,double,double,double,double)));
                // Todo: disconnect button mapping
    }
    if (!isRunning())
    {
        start();
    }
}

void Mouse6dofInput::init()
{
    // Make sure active UAS is set
    setActiveUAS(UASManager::instance()->getActiveUAS());
}

void Mouse6dofInput::run()
{
    init();

    forever
    {
        if (done)
        {
           done = false;
           exit();
        }

        if (mouseActive)
        {
            // Bound x value
            if (xValue > 1.0) xValue = 1.0;
            if (xValue < -1.0) xValue = -1.0;
            // Bound x value
            if (yValue > 1.0) yValue = 1.0;
            if (yValue < -1.0) yValue = -1.0;
            // Bound x value
            if (zValue > 1.0) zValue = 1.0;
            if (zValue < -1.0) zValue = -1.0;
            // Bound x value
            if (aValue > 1.0) aValue = 1.0;
            if (aValue < -1.0) aValue = -1.0;
            // Bound x value
            if (bValue > 1.0) bValue = 1.0;
            if (bValue < -1.0) bValue = -1.0;
            // Bound x value
            if (cValue > 1.0) cValue = 1.0;
            if (cValue < -1.0) cValue = -1.0;

            emit mouse6dofChanged(xValue, yValue, zValue, aValue, bValue, cValue);
        }

        // Sleep, update rate of 3d mouse is approx. 50 Hz (1000 ms / 50 = 20 ms)
        QGC::SLEEP::msleep(20);
    }
}

void Mouse6dofInput::motion3DMouse(std::vector<float> &motionData)
{
    if (motionData.size() < 6) return;
    mouseActive = true;

    xValue = (double)1.0e2f*motionData[ 1 ] / mouse3DMax;
    yValue = (double)1.0e2f*motionData[ 0 ] / mouse3DMax;
    zValue = (double)1.0e2f*motionData[ 2 ] / mouse3DMax;
    aValue = (double)1.0e2f*motionData[ 4 ] / mouse3DMax;
    bValue = (double)1.0e2f*motionData[ 3 ] / mouse3DMax;
    cValue = (double)1.0e2f*motionData[ 5 ] / mouse3DMax;

    //qDebug() << "NEW 3D MOUSE VALUES -- X" << xValue << " -- Y" << yValue << " -- Z" << zValue << " -- A" << aValue << " -- B" << bValue << " -- C" << cValue;
}
