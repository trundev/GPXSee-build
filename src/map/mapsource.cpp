#include <QFile>
#include <QXmlStreamReader>
#include "onlinemap.h"
#include "wmtsmap.h"
#include "mapsource.h"

#define ZOOM_MAX       19
#define ZOOM_MIN       0
#define BOUNDS_LEFT    -180
#define BOUNDS_TOP     85.0511
#define BOUNDS_RIGHT   180
#define BOUNDS_BOTTOM  -85.0511

MapSource::TMSConfig::TMSConfig()
  : zooms(ZOOM_MIN, ZOOM_MAX), bounds(Coordinates(BOUNDS_LEFT, BOUNDS_TOP),
  Coordinates(BOUNDS_RIGHT, BOUNDS_BOTTOM)) {}


Range MapSource::zooms(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	int min, max;
	bool res;

	if (attr.hasAttribute("min")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		min = attr.value("min").toString().toInt(&res);
#else // QT_VERSION < 5
		min = attr.value("min").toInt(&res);
#endif // QT_VERSION < 5
		if (!res || (min < ZOOM_MIN || min > ZOOM_MAX)) {
			reader.raiseError("Invalid minimal zoom level");
			return Range();
		}
	} else
		min = ZOOM_MIN;

	if (attr.hasAttribute("max")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		max = attr.value("max").toString().toInt(&res);
#else // QT_VERSION < 5
		max = attr.value("max").toInt(&res);
#endif // QT_VERSION < 5
		if (!res || (max < ZOOM_MIN || max > ZOOM_MAX)) {
			reader.raiseError("Invalid maximal zoom level");
			return Range();
		}
	} else
		max = ZOOM_MAX;

	if (min > max) {
		reader.raiseError("Invalid maximal/minimal zoom level combination");
		return Range();
	}

	return Range(min, max);
}

RectC MapSource::bounds(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	double top, left, bottom, right;
	bool res;

	if (attr.hasAttribute("top")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		top = attr.value("top").toString().toDouble(&res);
#else // QT_VERSION < 5
		top = attr.value("top").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (top < BOUNDS_BOTTOM || top > BOUNDS_TOP)) {
			reader.raiseError("Invalid bounds top value");
			return RectC();
		}
	} else
		top = BOUNDS_TOP;

	if (attr.hasAttribute("bottom")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		bottom = attr.value("bottom").toString().toDouble(&res);
#else // QT_VERSION < 5
		bottom = attr.value("bottom").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (bottom < BOUNDS_BOTTOM || bottom > BOUNDS_TOP)) {
			reader.raiseError("Invalid bounds bottom value");
			return RectC();
		}
	} else
		bottom = BOUNDS_BOTTOM;

	if (attr.hasAttribute("left")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		left = attr.value("left").toString().toDouble(&res);
#else // QT_VERSION < 5
		left = attr.value("left").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (left < BOUNDS_LEFT || left > BOUNDS_RIGHT)) {
			reader.raiseError("Invalid bounds left value");
			return RectC();
		}
	} else
		left = BOUNDS_LEFT;

	if (attr.hasAttribute("right")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		right = attr.value("right").toString().toDouble(&res);
#else // QT_VERSION < 5
		right = attr.value("right").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (right < BOUNDS_LEFT || right > BOUNDS_RIGHT)) {
			reader.raiseError("Invalid bounds right value");
			return RectC();
		}
	} else
		right = BOUNDS_RIGHT;

	if (bottom >= top) {
		reader.raiseError("Invalid bottom/top bounds combination");
		return RectC();
	}
	if (left >= right) {
		reader.raiseError("Invalid left/right bounds combination");
		return RectC();
	}

	return RectC(Coordinates(left, top), Coordinates(right, bottom));
}

void MapSource::map(QXmlStreamReader &reader, Config &config)
{
	config.type = (reader.attributes().value("type") == "WMTS") ? WMTS : TMS;

	while (reader.readNextStartElement()) {
		if (reader.name() == "name")
			config.name = reader.readElementText();
		else if (reader.name() == "url") {
			config.wmts.rest = (reader.attributes().value("type") == "REST")
			  ? true : false;
			config.url = reader.readElementText();
		} else if (reader.name() == "zoom") {
			config.tms.zooms = zooms(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "bounds") {
			config.tms.bounds = bounds(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "format") {
			config.wmts.format = reader.readElementText();
		} else if (reader.name() == "layer")
			config.wmts.layer = reader.readElementText();
		else if (reader.name() == "style")
			config.wmts.style = reader.readElementText();
		else if (reader.name() == "set") {
			config.wmts.yx = (reader.attributes().value("axis") == "yx")
			  ? true : false;
			config.wmts.set = reader.readElementText();
		} else
			reader.skipCurrentElement();
	}
}

Map *MapSource::loadFile(const QString &path)
{
	QFile file(path);
	QXmlStreamReader reader;
	Config config;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return 0;
	}

	reader.setDevice(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == "map")
			map(reader, config);
		else
			reader.raiseError("Not an online map source file");
	}
	if (reader.error()) {
		_errorString = QString("%1: %2").arg(reader.lineNumber())
		  .arg(reader.errorString());
		return 0;
	}

	if (config.name.isEmpty()) {
		_errorString = "Missing name definition";
		return 0;
	}
	if (config.url.isEmpty()) {
		_errorString = "Missing URL definition";
		return 0;
	}
	if (config.type == WMTS) {
		if (config.wmts.layer.isEmpty()) {
			_errorString = "Missing layer definition";
			return 0;
		}
		if (config.wmts.style.isEmpty()) {
			_errorString = "Missing style definiton";
			return 0;
		}
		if (config.wmts.set.isEmpty()) {
			_errorString = "Missing set definiton";
			return 0;
		}
		if (config.wmts.format.isEmpty()) {
			_errorString = "Missing format definition";
			return 0;
		}
	}

	Map *m;
	if (config.type == WMTS)
		m = new WMTSMap(config.name, WMTS::Setup(config.url, config.wmts.layer,
		  config.wmts.set, config.wmts.style, config.wmts.format,
		  config.wmts.rest, config.wmts.yx));
	else
		m = new OnlineMap(config.name, config.url, config.tms.zooms,
		  config.tms.bounds);
	if (!m->isValid()) {
		_errorString = m->errorString();
		delete m;
		return 0;
	}

	return m;
}
