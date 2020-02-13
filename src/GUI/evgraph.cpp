#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QContextMenuEvent>
#include "data/data.h"
#include "settings.h"
#include "common/evdata.h"
#include "evgraphitem.h"
#include "evgraph.h"


// Find action by ID, intended to lookup in the EVGraph::_evShowActions
static QAction* findAction(QList<QAction*> actions, int data) {
	for (int i = 0; i < actions.size(); i++) {
		if (actions[i]->data() == data) {
			return actions[i];
		}
	}
	return NULL;
}

EVGraph::EVGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	setYLabel(tr("Various"));

	setSliderPrecision(1);

#ifndef QT_NO_CONTEXTMENU
	QSettings settings(qApp->applicationName(), qApp->applicationName());
	settings.beginGroup(EVDATA_SETTINGS_GROUP);
	for (int id = 0; id < EVData::t_scalar_num; id++) {
		EVData::scalar_t scalarId = (EVData::scalar_t)id;

		QAction *a = new QAction(tr("Show ") + tr(EVData::getUserName(scalarId)), this);
		a->setMenuRole(QAction::NoRole);
		a->setCheckable(true);
		a->setData(id);

		bool checked = true;
		// Initially remove some of the parameters that are too big (fix incorrect scalling)....
		switch (scalarId)
		{
		case EVData::t_distance: case EVData::t_totaldistance:
			checked = false;
			break;
		}

		checked = settings.value(QString(EVDATA_SHOW_PREFIX_SETTINGS) + EVData::getInternalName(scalarId), checked).toBool();
		a->setChecked(checked);

		connect(a, SIGNAL(triggered(bool)), this,
		  SLOT(showEVData(bool)));

		_evShowActions.append(a);
		_contextMenu.addAction(a);
	}
	settings.endGroup();
#endif // QT_NO_CONTEXTMENU
}

EVGraph::~EVGraph()
{
#ifndef QT_NO_CONTEXTMENU
	QSettings settings(qApp->applicationName(), qApp->applicationName());
	settings.beginGroup(EVDATA_SETTINGS_GROUP);
	for (int id = 0; id < EVData::t_scalar_num; id++) {
		EVData::scalar_t scalarId = (EVData::scalar_t)id;

		QAction *a = findAction(_evShowActions, id);
		if (a) {
			bool checked = a->isChecked();
			settings.setValue(QString(EVDATA_SHOW_PREFIX_SETTINGS) + EVData::getInternalName(scalarId), checked);
		}
	}
	settings.endGroup();
	_evShowActions.clear();
#endif // QT_NO_CONTEXTMENU

	qDeleteAll(_tracks);
}

void EVGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());

		GraphTab::addInfo(tr("Graph number"), l.toString(_tracks.size()));
	} else
		clearInfo();
}

QList<GraphItem*> EVGraph::loadData(const Data &data)
{
	// Remove all graphs from previous file.
	// Do not visualize multiple files, to avoid confusion with the multiple
	// graphs from the same file.
	clear();

#ifndef QT_NO_CONTEXTMENU
	for (int i = 0; i < _evShowActions.size(); i++) {
		_evShowActions[i]->setDisabled(true);
	}
#endif // QT_NO_CONTEXTMENU

	QList<GraphItem*> graphs;
	for (int i = 0; i < data.tracks().count(); i++) {
		const Track &track = data.tracks().at(i);

		bool hasGraph = false;
		for (int id = 0; id < EVData::t_scalar_num; id++) {
			EVData::scalar_t scalarId = (EVData::scalar_t)id;

			const Graph &graph = track.evScalar(scalarId);
			if (graph.isValid()) {
				EVGraphItem *gi = new EVGraphItem(graph, _graphType,
				  _width, _palette.nextColor());
				gi->setScalarId(scalarId);
				gi->setUnits(_units);

				_tracks.append(gi);
				if (_showTracks)
					addGraph(gi);

				graphs.append(gi);
				hasGraph = true;

#ifndef QT_NO_CONTEXTMENU
				QAction *a = findAction(_evShowActions, id);
				if (a) {
					a->setDisabled(false);
					if (!a->isChecked())
						removeGraph(gi);
				}
#endif // QT_NO_CONTEXTMENU
			}
		}

		if (!hasGraph) {
			_palette.nextColor();
			graphs.append(0);
		}
	}

	for (int i = 0; i < data.routes().count(); i++) {
		_palette.nextColor();
		graphs.append(0);
	}

	for (int i = 0; i < data.areas().count(); i++)
		_palette.nextColor();

	setInfo();
	redraw();

	return graphs;
}

void EVGraph::clear()
{
	qDeleteAll(_tracks);
	_tracks.clear();

	GraphTab::clear();
}

void EVGraph::setUnits(Units units)
{
	GraphTab::setUnits(units);
}

void EVGraph::showTracks(bool show)
{
	_showTracks = show;

	for (int i = 0; i < _tracks.size(); i++) {
		if (show)
			addGraph(_tracks.at(i));
		else
			removeGraph(_tracks.at(i));
	}

	setInfo();

	redraw();
}

#ifndef QT_NO_CONTEXTMENU
void EVGraph::contextMenuEvent(QContextMenuEvent *event)
{
	const QList<QGraphicsItem *> &view_items = items(event->pos());

	// Find the the uppermost GraphItem --
	// the smallest index returned by QGraphicsView::items()
	const EVGraphItem *gi = NULL;
	for (int idx = 0; idx < view_items.size(); idx++) {
		QGraphicsObject *go = view_items[idx]->toGraphicsObject();
		if (go) {
			int i = _tracks.indexOf((EVGraphItem*)go);
			if (i >= 0) {
				gi = _tracks[i];
				break;
			}
		}
	}

	if (gi) {
		// Clicked on an EV graph, show modified menu
		EVData::scalar_t scalarId = gi->getScalarId();

		QAction *a = new QAction(tr("Show only ") + tr(EVData::getUserName(scalarId)), this);
		a->setMenuRole(QAction::NoRole);
		a->setData(scalarId);
		connect(a, SIGNAL(triggered()), this,
		  SLOT(showSingleEVData()));

		QMenu menu;
		menu.addAction(a);
		menu.addSeparator();
		for (int idx = 0; idx < _evShowActions.size(); idx++)
			menu.addAction(_evShowActions[idx]);

		menu.exec(event->globalPos());
		delete a;
	}
	else
		_contextMenu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU

void EVGraph::showEVData(bool show)
{
	QAction* act = qobject_cast<QAction *>(sender());
	if (act == NULL)
		return;

	EVGraphItem *gi = _tracks.at(act->data().toInt());
	if (show)
		addGraph(gi);
	else
		removeGraph(gi);
	redraw();
}

void EVGraph::showSingleEVData()
{
	QAction* act = qobject_cast<QAction *>(sender());
	if (act == NULL)
		return;

	int id = act->data().toInt();
	for (int idx = 0; idx < EVData::t_scalar_num; idx++)
		if (idx != id) {
			QAction *a = findAction(_evShowActions, idx);
			if (a) {
				a->setChecked(false);
				EVGraphItem *gi = _tracks.at(idx);
				removeGraph(gi);
			}
		}

	redraw();
}
