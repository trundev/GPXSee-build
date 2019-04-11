#include <QLocale>
#include "tooltip.h"
#include "common/evdata.h"
#include "evgraphitem.h"


EVGraphItem::EVGraphItem(const Graph &graph, GraphType type,
  QGraphicsItem *parent) : GraphItem(graph, type, parent), _scalarId(-1)
{
}

void EVGraphItem::setScalarId(int id)
{
	_scalarId = id;
	_paramName = tr(EVData::getUserName((EVData::scalar_t)id));

	switch ((EVData::scalar_t)id)
	{
	// Main parameters (speed, electrical)
	case EVData::t_speed:
		_unitsSuffix = tr("km/h");
		break;

	case EVData::t_voltage:
		_unitsSuffix = tr("V");
		break;

	case EVData::t_current:
		_unitsSuffix = tr("A");
		break;

	case EVData::t_power:
		_unitsSuffix = tr("W");
		break;

	// Battery
	case EVData::t_battery_level:
		_unitsSuffix = tr("%");
		break;

	// Odometers
	case EVData::t_distance:
	case EVData::t_totaldistance:
		_unitsSuffix = tr("m");
		break;

	// Temperatures
	case EVData::t_system_temp:
	case EVData::t_cpu_temp:
		_unitsSuffix = QChar(0x00B0) + tr("C");
		break;

	// Self balancing parameters
	case EVData::t_tilt:
	case EVData::t_roll:
		_unitsSuffix = QChar(0x00B0);
		break;

	default:
		_unitsSuffix = tr("?");
		break;
	}

	setToolTip(toolTip());
}

QString EVGraphItem::toolTip() const
{
	ToolTip tt;
	QLocale l(QLocale::system());

	tt.insert(_paramName, QString::Null());
	tt.insert(tr("Average"), l.toString(avg(), 'f', 1) + UNIT_SPACE + _unitsSuffix);
	tt.insert(tr("Maximum"), l.toString(max(), 'f', 1) + UNIT_SPACE + _unitsSuffix);
	tt.insert(tr("Minimum"), l.toString(min(), 'f', 1) + UNIT_SPACE + _unitsSuffix);

	return tt.toString();
}
