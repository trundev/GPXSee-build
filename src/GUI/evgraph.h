#ifndef EVGRAPH_H
#define EVGRAPH_H

#include <QMenu>
#include "graphtab.h"

class EVGraph : public GraphTab
{
	Q_OBJECT

public:
	EVGraph(QWidget *parent = 0);
	~EVGraph();

	QString label() const {return tr("Electric Vehicle");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void setUnits(enum Units units);
	void showTracks(bool show);

public slots:
	void showEVData(bool show);

private:
#ifndef QT_NO_CONTEXTMENU
	QMenu _contextMenu;
	QList<QAction*> _evShowActions;
	void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

	void setInfo();

	bool _showTracks;
};

#endif // EVGRAPH_H
