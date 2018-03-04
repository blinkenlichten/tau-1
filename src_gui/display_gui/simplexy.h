#ifndef SIMPLEXY_H
#define SIMPLEXY_H

#include <QObject>
#include <QWidget>
#include <QtCharts/QLineSeries>
#include <thread>
#include "t1net/metrics_send.h"

class SimpleXY : public QLineSeries
{
    Q_WIDGET
public:
    explicit SimpleXY();
    virtual ~SimpleXY();

protected:
    virtual void paintEvent(QEvent*) override;
    virtual void timerEvent(QTimerEvent *event) override;
signals:

public slots:

private:
    QVector<QPointF> plot_vector;
    std::list<QVector<QPointF> > list;
};

#endif // SIMPLEXY_H
