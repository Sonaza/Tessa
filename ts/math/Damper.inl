
template <class T>
Damper<T>::Damper()
{
}

template <class T>
Damper<T>::Damper(const T &value, float smoothing)
	: m_currentValue(value)
	, m_targetValue(value)
	, m_smoothingFactor(smoothing)
{
}

template <class T>
void Damper<T>::update(TimeSpan deltaTime)
{
	m_currentValue = lerp(m_currentValue, m_targetValue, 1.f - std::exp(-m_smoothingFactor * deltaTime.getSecondsAsFloat()));
}

template <class T>
void Damper<T>::reset(const T &value, float smoothing)
{
	m_currentValue = value;
	m_targetValue = value;
	m_smoothingFactor = smoothing;
}

template <class T>
void Damper<T>::setSmoothingFactor(float smoothing)
{
	TS_ASSERT(smoothing >= 0.f && "Smoothing factor value must be between zero and infinity.");
	m_smoothingFactor = smoothing;
}

template <class T>
float Damper<T>::getSmoothingFactor() const
{
	return m_smoothingFactor;
}

template <class T>
void Damper<T>::setTarget(const T &target)
{
	m_targetValue = target;
}

template <class T>
const T &Damper<T>::getTarget() const
{
	return m_targetValue;
}

template <class T>
void Damper<T>::setValue(const T &value)
{
	m_currentValue = value;
}

template <class T>
const T &Damper<T>::getValue() const
{
	return m_currentValue;
}

template <class T>
void Damper<T>::cutToTarget()
{
	m_currentValue = m_targetValue;
}
