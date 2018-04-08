#ifndef SIMPLEXY_H
#define SIMPLEXY_H

#include <QObject>
#include <QWidget>
#include <QtCharts>
#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QAbstractBarSeries>
#include <QtCharts/QPercentBarSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCore/QRunnable>
#include <thread>
#include "t1net/metrics_send.h"

class SimpleXY : public QLineSeries
{
    Q_OBJECT
public:
    explicit SimpleXY(QObject* parent = nullptr);
    virtual ~SimpleXY();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
signals:

public slots:

private:
    std::unique_ptr<QRunnable> m_dummy_writer, m_reader;
    QVector<QPointF> plot_vector;
    std::list<QVector<QPointF> > list;
};

#endif // SIMPLEXY_H
