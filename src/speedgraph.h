#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graphview.h"
#include "gpx.h"
#include "units.h"

class SpeedGraph : public GraphView
{
	Q_OBJECT

public:
	SpeedGraph(QWidget *parent = 0);

	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

	qreal avg() const;
	qreal max() const {return _max;}

private:
	void addInfo();

	qreal _max;
	QList<QPointF> _avg;
};

#endif // SPEEDGRAPH_H
