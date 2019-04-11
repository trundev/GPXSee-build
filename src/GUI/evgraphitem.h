#ifndef EVGRAPHITEM_H
#define EVGRAPHITEM_H

#include "graphitem.h"


class EVGraphItem : public GraphItem
{
	Q_OBJECT

public:
	EVGraphItem(const Graph &graph, GraphType type,
	  QGraphicsItem *parent = 0);

	void setScalarId(int id);

private:
	int _scalarId;
	QString _paramName;
	QString _unitsSuffix;

	QString toolTip() const;
};

#endif // EVGRAPHITEM_H
