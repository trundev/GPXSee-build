#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graphtab.h"

class ElevationGraphItem;

class ElevationGraph : public GraphTab
{
	Q_OBJECT

public:
	ElevationGraph(QWidget *parent = 0);
	~ElevationGraph();

	QString label() const {return tr("Elevation");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void setUnits(enum Units units);
	void showTracks(bool show);
	void showRoutes(bool show);

private:
	enum PathType {TrackPath, RoutePath};

	qreal max() const;
	qreal min() const;
	qreal ascent() const;
	qreal descent() const;

	void setYUnits(Units units);
	void setInfo();

	GraphItem *loadGraph(const Graph &graph, PathType type, const QColor &color,
	  bool primary);
	void showItems(const QList<ElevationGraphItem *> &list, bool show);

	qreal _trackAscent, _trackDescent;
	qreal _routeAscent, _routeDescent;
	qreal _trackMax, _routeMax;
	qreal _trackMin, _routeMin;

	bool _showTracks, _showRoutes;
	QList<ElevationGraphItem *> _tracks, _routes;
};

#endif // ELEVATIONGRAPH_H
