#ifndef EVDATA_H
#define EVDATA_H

#include <QDebug>

// Note: Must match the Wheellog CSV column headers
#define ENUM_EVDATA_SCALARS(F) \
	F(speed) F(voltage) F(current) F(power) F(battery_level) \
	F(distance) F(totaldistance) F(system_temp) F(cpu_temp) \
	F(tilt) F(roll)

// Electric Vehicle data, esp. Electric Unicycle
class EVData
{
public:
	typedef enum {
#define F_ENUM(n)	t_##n,
		ENUM_EVDATA_SCALARS(F_ENUM)
#undef F_ENUM
		t_scalar_num
	} scalar_t;

	EVData() {
		for (int i = 0; i < t_scalar_num; i++)
			_scalars[i] = NAN;
	}

	void setScalar(scalar_t id, qreal val)  { if (id < t_scalar_num) _scalars[id] = val;}
	qreal scalar(scalar_t id) const         { return id < t_scalar_num ? _scalars[id] : NAN;}

	void setMode(const QString &mode)       {_mode = mode;}
	void setAlert(const QString &alert)     {_alert = alert;}
	const QString &alert() const {return _alert;}
	const QString &mode() const {return _mode;}

	bool hasAlert() const {return !_alert.isEmpty();}

	// Return the plain CSV column name (internal name)
	static const char *getInternalName(scalar_t id)
	{
		if ((unsigned)id < t_scalar_num) {
			static const char *scalarNames[] = {
				#define F_ENUM(n)	#n,
				ENUM_EVDATA_SCALARS(F_ENUM)
				#undef F_ENUM
			};
			return scalarNames[id];
		}
		return NULL;
	}

	// Return a user readable name
	static const char *getUserName(scalar_t id)
	{
		switch (id)
		{
		case t_speed:         return "Speed";
		case t_voltage:       return "Voltage";
		case t_current:       return "Current";
		case t_power:         return "Motor Power";
		case t_battery_level: return "Battery";
		case t_distance:      return "Trip distance";
		case t_totaldistance: return "Total distance";
		case t_system_temp:   return "System temperature";
		case t_cpu_temp:      return "CPU temperature";
		case t_tilt:          return "Tilt angle";
		case t_roll:          return "Roll angle";
		default:              return NULL;
		}
	}

private:
	// Electric drive parameters
	qreal _scalars[t_scalar_num];

	// String data
	QString _mode;
	QString _alert;
};

Q_DECLARE_TYPEINFO(EVData, Q_PRIMITIVE_TYPE);

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const EVData &c);
#endif // QT_NO_DEBUG

#endif // EVDATA_H
