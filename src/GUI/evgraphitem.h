#ifndef EVGRAPHITEM_H
#define EVGRAPHITEM_H

#include "graphitem.h"
#include "common/evdata.h"


class EVGraphItem : public GraphItem
{
	Q_OBJECT

public:
	EVGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, QGraphicsItem *parent = 0);

	QString info() const;

	void setScalarId(EVData::scalar_t id);
	EVData::scalar_t getScalarId() const {return _scalarId;}

private:
	EVData::scalar_t _scalarId;
	QString _paramName;
	QString _unitsSuffix;
};

#endif // EVGRAPHITEM_H
