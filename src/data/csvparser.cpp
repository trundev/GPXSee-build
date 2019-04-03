#include <QImageReader>
#include "csvparser.h"
#include "GUI/icons.h"

// Wheel log columns
#define ENUM_WHEELLOG_COLUMNS(F)	\
	/* Android's GPS data */ \
	F(date) F(time) F(latitude) F(longitude) F(gps_speed) F(gps_alt) F(gps_heading) F(gps_distance) \
	/* Electric Vehicle data */ \
	F(speed) F(voltage) F(current) F(power) F(battery_level) F(distance) F(totaldistance) \
	F(system_temp) F(cpu_temp) F(tilt) F(roll) \
	F(mode) F(alert) \

bool CSVParser::parse_waypoints(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	bool res;

	_errorLine = 1;
	_errorString.clear();

	while (!file->atEnd()) {
		QByteArray line = file->readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() < 3) {
			_errorString = "Parse error";
			return false;
		}

		qreal lon = list[0].trimmed().toDouble(&res);
		if (!res || (lon < -180.0 || lon > 180.0)) {
			_errorString = "Invalid longitude";
			return false;
		}
		qreal lat = list[1].trimmed().toDouble(&res);
		if (!res || (lat < -90.0 || lat > 90.0)) {
			_errorString = "Invalid latitude";
			return false;
		}
		Waypoint wp(Coordinates(lon, lat));

		QByteArray ba = list[2].trimmed();
		QString name = QString::fromUtf8(ba.data(), ba.size());
		wp.setName(name);

		if (list.size() > 3) {
			ba = list[3].trimmed();
			wp.setDescription(QString::fromUtf8(ba.data(), ba.size()));
		}

		waypoints.append(wp);
		_errorLine++;
	}

	return true;
}

// Parse logs generated by Wheellog Android app (Electric Unicycle logger)
// (https://github.com/palachzzz/WheelLogAndroid.git)

// Log file column header (first line)
static struct {
	const char *name;
	int column;
} WlColumns[] = {
#define F_STRUCT(n)	{#n, -1},
ENUM_WHEELLOG_COLUMNS(F_STRUCT)
#undef F_STRUCT
};

typedef enum {
#define F_ENUM(n)	wl_##n##_idx,
ENUM_WHEELLOG_COLUMNS(F_ENUM)
#undef F_ENUM
	wl_idx_total
} WlColumn_t;

static QString get_column_str(const QList<QByteArray> &list, WlColumn_t column)
{
	int col = WlColumns[column].column;
	if (col == -1) {
		return QString::Null();
	}
	QByteArray ba = list[col].trimmed();
	return QString::fromUtf8(ba.data(), ba.size());
}

bool CSVParser::parse_wheellog(QFile *file, QList<TrackData> &tracks,
	QList<RouteData> &routes, QList<Area> &polygons,
	QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);

	_errorLine = 1;
	_errorString.clear();

	// First line is the header
	file->seek(0);
	QByteArray line = file->readLine();
	QList<QByteArray> header_list = line.split(',');
	if (header_list.size() < 4) {
		_errorString = "Parse error";
		return false;
	}

	// Obtain the column indicies to be extracted
	for (size_t idx = 0; idx < sizeof(WlColumns) / sizeof(*WlColumns); idx++)
		WlColumns[idx].column = -1;
	for (int col = 0; col < header_list.size(); col++) {
		QByteArray ba = header_list[col].trimmed();
		QString name = QString::fromUtf8(ba.data(), ba.size());		

		for (size_t idx = 0; idx < sizeof(WlColumns) / sizeof(*WlColumns); idx++) {
			if (name == WlColumns[idx].name) {
				if (WlColumns[idx].column == -1) {
					WlColumns[idx].column = col;
					break;
				}
				else {
					_errorString = "Duplicate column header: " + name;
					return false;
				}
			}
		}
	}

	// Check for mandatory columns
	if (WlColumns[wl_latitude_idx].column == -1 || WlColumns[wl_longitude_idx].column == -1) {
		_errorString = "Missing latitude and/or longitude columns";
		return false;
	}

	_errorLine++;

	tracks.append(TrackData());
	TrackData &track = tracks.back();
	track.append(SegmentData());

	QString last_mode;
	QDateTime last_time_stamp;

	while (!file->atEnd()) {
		line = file->readLine().trimmed();
		// Ignore empty lines
		if (line.isEmpty()) {
			_errorLine++;
			continue;
		}

		QList<QByteArray> list = line.split(',');
		if (list.size() < header_list.size()) {
			_errorString = "Insufficient parameter number";
			return false;
		}

		// Sometimes, the last column (alert) contain commas,
		// merge back items entries after the last header column
		while (list.size() > header_list.size()) {
			int last = list.size() - 1;
			list[last - 1] += "," + list[last];
			list.removeLast();
		}

		QString mode, alert;
		Coordinates coords;
		QDateTime time_stamp;
		Trackpoint trackpoint;

		for (int idx = 0; idx < sizeof(WlColumns) / sizeof(*WlColumns); idx++) {
			QString str_val = get_column_str(list, static_cast<WlColumn_t>(idx));
			bool res = !str_val.isNull();
			if (!res)
				continue;

			// Convert to float if necessary
			double float_val = (double)NAN;
			switch (idx)
			{
			// These columns contain strings or date-time
			case wl_date_idx: case wl_time_idx: case wl_mode_idx: case wl_alert_idx:
				//qDebug("%d(%s): %s\n", idx, WlColumns[idx].name, qUtf8Printable(str_val));
				break;
			// Other columns contain float numbers
			default:
				float_val = str_val.toDouble(&res);
				//qDebug("%d(%s): %f\n", idx, WlColumns[idx].name, float_val);
			}
			if (!res)
				continue;

			switch (idx)
			{
			/* Android's GPS data */
			case wl_date_idx:			time_stamp.setDate(QDate::fromString(str_val, Qt::ISODate)); break;
			case wl_time_idx:			time_stamp.setTime(QTime::fromString(str_val)); break;
			case wl_latitude_idx:		coords.setLat(float_val); break;
			case wl_longitude_idx:		coords.setLon(float_val); break;
			case wl_gps_speed_idx:		/*trackpoint.setSpeed(float_val / 3.6);*/ break;	// km/h -> m/s
			case wl_gps_alt_idx:		trackpoint.setElevation(float_val); break;
			case wl_gps_heading_idx:	break;
			case wl_gps_distance_idx:	break;
			/* Electric Vehicle data */
			case wl_speed_idx:			trackpoint.setSpeed(float_val / 3.6); break;	// km/h -> m/s
			case wl_voltage_idx:		break;
			case wl_current_idx:		break;
			case wl_power_idx:			trackpoint.setPower(float_val); break;
			case wl_battery_level_idx:	break;
			case wl_distance_idx:		break;
			case wl_totaldistance_idx:	break;
			case wl_system_temp_idx:	trackpoint.setTemperature(float_val); break;
			case wl_cpu_temp_idx:		break;
			case wl_tilt_idx:			break;
			case wl_roll_idx:			break;
			case wl_mode_idx:			mode = str_val; break;
			case wl_alert_idx:			alert = str_val; break;
			}
		}

		if (coords.isValid()) {
			// Avoid problems with unordered time-stamps by ignoring them
			// TODO: Must sort the log
			if (last_time_stamp < time_stamp) {
				last_time_stamp = time_stamp;
				trackpoint.setTimestamp(time_stamp);
				trackpoint.setCoordinates(coords);
				track.last().append(trackpoint);
			}
			else {
				qDebug() << "Warning: Ignored unordered time-stamp" << time_stamp.toString() << ", at line" << _errorLine;
			}

			// Add waypoint on alert or mode change
			if (!alert.isEmpty() || last_mode != mode) {
				Waypoint waypoint(coords);
				waypoint.setTimestamp(time_stamp);
				waypoint.setElevation(trackpoint.elevation());
				waypoint.setName(alert.isEmpty() ? mode : "ALERT");
				QString descr;
				QTextStream(&descr) << "Line " << _errorLine << ": " << alert;
				waypoint.setDescription(descr);
#if 1	//Experimental: Add icons
				static ImageInfo poiIcon(SHOW_POI_ICON, QImageReader(SHOW_POI_ICON).size());
				static ImageInfo alertIcon(CLOSE_FILE_ICON, QImageReader(CLOSE_FILE_ICON).size());
				waypoint.setImage(alert.isEmpty() ?  poiIcon : alertIcon);
#endif
				waypoints.append(waypoint);
			}

			// Keep the last mode
			last_mode = mode;
		}

		_errorLine++;
	}

	return true;
}

bool CSVParser::parse(QFile *file, QList<TrackData> &tracks,
	QList<RouteData> &routes, QList<Area> &polygons,
	QVector<Waypoint> &waypoints)
{
	// Try Garmin waypoints format
	if (!parse_waypoints(file, tracks, routes, polygons, waypoints))
		// Try Wheellog format
		return parse_wheellog(file, tracks, routes, polygons, waypoints);

	return true;
}
