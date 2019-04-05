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
